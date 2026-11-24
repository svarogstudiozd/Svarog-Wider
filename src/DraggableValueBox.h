#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class DraggableValueBox : public juce::Component,
                          private juce::AudioProcessorValueTreeState::Listener,
                          private juce::TextEditor::Listener
{
public:
    DraggableValueBox (juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& parameterID,
                       const juce::String& prefixLabel,
                       const juce::String& valueSuffix,
                       double resetValue);

    ~DraggableValueBox() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void enablementChanged() override;

    void setValueFromParameter();
    void setClampRange (double minVal, double maxVal);
    void setShowPrefix (bool shouldShow);
    void setUiScale (float newScale);

    juce::String getParameterID() const { return parameterID; }
    double getCurrentValue() const noexcept { return currentValue; }

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::String parameterID;
    juce::RangedAudioParameter* param = nullptr;
    juce::NormalisableRange<float> range;

    juce::String prefix;
    juce::String suffix;
    double resetToValue = 0.0;

    double currentValue = 0.0;
    double clampMin = 0.0;
    double clampMax = 0.0;
    bool   hasClamp = false;
    bool   showPrefix = true;

    bool hovered = false;
    bool editing = false;
    float uiScale = 1.0f;

    juce::TextEditor editor;

    float dragStartValue = 0.0f;
    int   dragStartY = 0;
    bool  dragWasStarted = false;

    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void textEditorReturnKeyPressed (juce::TextEditor&) override;
    void textEditorFocusLost (juce::TextEditor&) override;
    void textEditorEscapeKeyPressed (juce::TextEditor&) override;

    juce::String formatDisplay() const;
    juce::String formatEditValue() const;
    void applyFromNormalized (float normalized);
    double clampedValue (double value) const;
    void startEditing();
    void finishEditing (bool commit);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DraggableValueBox)
};