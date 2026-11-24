#include "DraggableValueBox.h"
#include "Theme.h"
#include "SvarogLookAndFeel.h"

DraggableValueBox::DraggableValueBox (juce::AudioProcessorValueTreeState& apvtsToUse,
                                      const juce::String& parameterIDToUse,
                                      const juce::String& prefixLabel,
                                      const juce::String& valueSuffix,
                                      double resetValue)
    : apvts (apvtsToUse),
      parameterID (parameterIDToUse),
      prefix (prefixLabel),
      suffix (valueSuffix),
      resetToValue (resetValue)
{
    param = apvts.getParameter (parameterID);
    jassert (param != nullptr);
    if (param != nullptr)
        range = param->getNormalisableRange();

    setInterceptsMouseClicks (true, false);
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);

    editor.setVisible (false);
    editor.setMultiLine (false);
    editor.setReturnKeyStartsNewLine (false);
    editor.setJustification (juce::Justification::centred);
    editor.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold()).withHeight (13.5f)));
    editor.setColour (juce::TextEditor::backgroundColourId, Theme::surfaceRaised.darker (0.35f));
    editor.setColour (juce::TextEditor::textColourId,       Theme::textPrimary);
    editor.setColour (juce::TextEditor::outlineColourId,    juce::Colours::transparentBlack);
    editor.setColour (juce::TextEditor::focusedOutlineColourId, Theme::accentRedBright.withAlpha (0.4f));
    editor.setColour (juce::TextEditor::highlightColourId,  Theme::accentRedBright.withAlpha (0.3f));
    editor.addListener (this);
    addChildComponent (editor);

    apvts.addParameterListener (parameterID, this);
    setValueFromParameter();
}

DraggableValueBox::~DraggableValueBox()
{
    apvts.removeParameterListener (parameterID, this);
    if (auto* topLevel = getTopLevelComponent())
        topLevel->removeMouseListener (this);
}

void DraggableValueBox::resized()
{
    editor.setBounds (getLocalBounds());
}

void DraggableValueBox::setClampRange (double minVal, double maxVal)
{
    clampMin = minVal;
    clampMax = maxVal;
    hasClamp = (clampMin <= clampMax);
    if (hasClamp)
    {
        const double clamped = clampedValue (currentValue);
        if (std::abs (clamped - currentValue) > 1.0e-6)
            applyFromNormalized ((float) range.convertTo0to1 ((float) clamped));
    }
}

void DraggableValueBox::setShowPrefix (bool shouldShow)
{
    showPrefix = shouldShow;
    repaint();
}

double DraggableValueBox::clampedValue (double value) const
{
    return hasClamp ? juce::jlimit (clampMin, clampMax, value) : value;
}

void DraggableValueBox::setValueFromParameter()
{
    if (param == nullptr) return;
    currentValue = range.convertFrom0to1 (param->getValue());
    repaint();
}

void DraggableValueBox::parameterChanged (const juce::String& id, float)
{
    if (id != parameterID) return;
    juce::MessageManager::callAsync (
        [safe = juce::Component::SafePointer<DraggableValueBox> (this)]
        { if (safe != nullptr) safe->setValueFromParameter(); });
}

juce::String DraggableValueBox::formatDisplay() const
{
    juce::String valueStr;
    if (range.interval >= 1.0f)
        valueStr = juce::String (juce::roundToInt (currentValue));
    else
        valueStr = juce::String (currentValue, 1);

    return (showPrefix && prefix.isNotEmpty() ? prefix + ": " : juce::String())
         + valueStr
         + (suffix.isNotEmpty() ? " " + suffix : juce::String());
}

juce::String DraggableValueBox::formatEditValue() const
{
    if (range.interval >= 1.0f)
        return juce::String (juce::roundToInt (currentValue));
    return juce::String (currentValue, 1);
}

void DraggableValueBox::startEditing()
{
    if (editing) return;
    editing = true;

    editor.setText (formatEditValue(), juce::dontSendNotification);
    editor.setVisible (true);
    editor.grabKeyboardFocus();
    editor.selectAll();

    if (auto* topLevel = getTopLevelComponent())
        topLevel->addMouseListener (this, true);

    repaint();
}

void DraggableValueBox::finishEditing (bool commit)
{
    if (! editing) return;
    editing = false;

    if (auto* topLevel = getTopLevelComponent())
        topLevel->removeMouseListener (this);

    if (commit && param != nullptr)
    {
        juce::String text = editor.getText().trim();
        if (suffix.isNotEmpty() && text.endsWithIgnoreCase (suffix))
            text = text.dropLastCharacters (suffix.length()).trim();

        const float newValue = text.getFloatValue();
        const float clamped = (float) clampedValue (newValue);
        const float norm = range.convertTo0to1 (clamped);

        param->beginChangeGesture();
        param->setValueNotifyingHost (norm);
        param->endChangeGesture();

        currentValue = clamped;
    }

    editor.setVisible (false);
    repaint();
}

void DraggableValueBox::textEditorReturnKeyPressed (juce::TextEditor&)
{
    finishEditing (true);
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
}

void DraggableValueBox::textEditorFocusLost (juce::TextEditor&)
{
    finishEditing (false);
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
}

void DraggableValueBox::textEditorEscapeKeyPressed (juce::TextEditor&)
{
    finishEditing (false);
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
}

void DraggableValueBox::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const bool enabled = isEnabled();

    auto* lf = dynamic_cast<SvarogLookAndFeel*> (&getLookAndFeel());

    juce::Colour fill = Theme::surfaceRaised.darker (0.35f);
    if (! enabled)         fill = fill.darker (0.35f);
    else if (hovered)      fill = fill.brighter (0.05f);

    if (lf != nullptr)
    {
        lf->drawRecessedControl (g, bounds, fill);

        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addRoundedRectangle (bounds, Radius::control);
        g.reduceClipRegion (clip, {});
        juce::ColourGradient dim (
            juce::Colours::black.withAlpha (0.50f), bounds.getX(), bounds.getBottom(),
            juce::Colours::black.withAlpha (0.0f), bounds.getX(), bounds.getBottom() - 3.0f, false);
        g.setGradientFill (dim);
        g.fillRect (juce::Rectangle<float> (bounds.getX(), bounds.getBottom() - 3.0f,
                                            bounds.getWidth(), 3.0f));
    }
    else
    {
        g.setColour (fill);
        g.fillRoundedRectangle (bounds, Radius::control);
    }

    if (editing) return;

    juce::Colour textColour = ! enabled ? Theme::textFaint
                             : hovered  ? Theme::textPrimary
                                        : Theme::textSecondary;

    g.setColour (textColour);
    g.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold())
                                .withHeight (13.5f * uiScale)));

    const int hPad = juce::roundToInt (6.0f * uiScale);

    if (enabled)
    {
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.drawText (formatDisplay(),
                    getLocalBounds().reduced (hPad, 0).translated (0, 1),
                    juce::Justification::centred);
    }

    g.setColour (textColour);
    g.drawText (formatDisplay(),
                getLocalBounds().reduced (hPad, 0),
                juce::Justification::centred);
}

void DraggableValueBox::mouseDown (const juce::MouseEvent& e)
{
    if (editing)
    {
        if (e.eventComponent != &editor && ! editor.isParentOf (e.eventComponent))
            finishEditing (false);
        return;
    }

    dragStartValue = (float) currentValue;
    dragStartY = e.getMouseDownY();
    dragWasStarted = false;
}

void DraggableValueBox::mouseDrag (const juce::MouseEvent& e)
{
    if (editing) return;
    if (param == nullptr) return;
    if (e.getDistanceFromDragStart() < 2) return;

    dragWasStarted = true;

    constexpr float pixelsForFullRange = 200.0f;
    const int deltaY = dragStartY - e.getPosition().y;
    const float normStart = param->convertTo0to1 (dragStartValue);
    const float normDelta = (float) deltaY / pixelsForFullRange;
    const float normNew   = juce::jlimit (0.0f, 1.0f, normStart + normDelta);

    const float value = range.convertFrom0to1 (normNew);
    const float clamped = (float) clampedValue (value);
    const float normClamped = range.convertTo0to1 (clamped);
    param->setValueNotifyingHost (normClamped);

    currentValue = clamped;
    repaint();
}

void DraggableValueBox::mouseUp (const juce::MouseEvent&)
{
    if (editing) return;

    // If the user dragged, it was a value change (no editing).
    // If not, it was a simple click — open the text editor.
    if (! dragWasStarted)
        startEditing();

    dragWasStarted = false;
}

void DraggableValueBox::mouseDoubleClick (const juce::MouseEvent&)
{
    // Double-click: cancel any editor that single-click just opened,
    // and reset to default.
    if (editing)
        finishEditing (false);

    if (param != nullptr)
    {
        const float norm = param->convertTo0to1 ((float) resetToValue);
        param->beginChangeGesture();
        param->setValueNotifyingHost (norm);
        param->endChangeGesture();
        currentValue = resetToValue;
        repaint();
    }
}

void DraggableValueBox::mouseEnter (const juce::MouseEvent&) { hovered = true;  repaint(); }
void DraggableValueBox::mouseExit  (const juce::MouseEvent&) { hovered = false; repaint(); }

void DraggableValueBox::applyFromNormalized (float normalized)
{
    if (param == nullptr) return;
    param->beginChangeGesture();
    param->setValueNotifyingHost (normalized);
    param->endChangeGesture();
    currentValue = range.convertFrom0to1 (normalized);
    repaint();
}

void DraggableValueBox::enablementChanged()
{
    setMouseCursor (isEnabled() ? juce::MouseCursor::UpDownResizeCursor
                                : juce::MouseCursor::NormalCursor);
    repaint();
}

void DraggableValueBox::setUiScale (float newScale)
{
    uiScale = juce::jmax (0.5f, newScale);
    editor.applyFontToAllText (
        juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold())
                          .withHeight (13.5f * uiScale)));
    repaint();
}