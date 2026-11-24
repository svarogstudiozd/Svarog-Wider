#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Theme.h"
#include "SvarogLookAndFeel.h"
#include "DraggableValueBox.h"
#include "LevelMeterComponent.h"

// =========================================================
//  Hover slider (for the knobs) with smooth hover fade animation
// =========================================================
class HoverSlider : public juce::Slider, private juce::Timer
{
public:
    HoverSlider()
    {
        setPaintingIsUnclipped (true);
        startTimerHz (60);   // 60fps for smooth animation
    }

    ~HoverSlider() override
    {
        stopTimer();
    }

    void mouseEnter (const juce::MouseEvent&) override
    {
        targetHover = 1.0f;
        repaint();
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        targetHover = 0.0f;
        repaint();
    }

    void mouseMove (const juce::MouseEvent&) override { repaint(); }

    void mouseDown (const juce::MouseEvent& e) override
    {
        getProperties().set ("isDragging", true);
        juce::Slider::mouseDown (e);
        repaint();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        juce::Slider::mouseDrag (e);
        repaint();
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        getProperties().set ("isDragging", false);
        juce::Slider::mouseUp (e);
        repaint();
    }

    bool hitTest (int x, int y) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (2.0f);
        auto radius   = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto dx = (float) x - bounds.getCentreX();
        auto dy = (float) y - bounds.getCentreY();
        return (dx * dx + dy * dy) <= (radius * radius);
    }

private:
    void timerCallback() override
    {
        float current = (float) getProperties().getWithDefault ("hoverAlpha", 0.0f);
        if (std::abs (current - targetHover) < 0.001f)
            return;

        const float step = 0.12f;
        if (current < targetHover)
            current = juce::jmin (targetHover, current + step);
        else
            current = juce::jmax (targetHover, current - step);

        getProperties().set ("hoverAlpha", current);
        repaint();
    }

    float targetHover = 0.0f;
};

// =========================================================
//  Dragging slider (for pan/delay/low cut) with smooth hover fade
// =========================================================
class DraggingSlider : public juce::Slider, private juce::Timer
{
public:
    DraggingSlider()
    {
        setPaintingIsUnclipped (true);
        startTimerHz (60);
    }

    ~DraggingSlider() override
    {
        stopTimer();
    }

    void mouseEnter (const juce::MouseEvent&) override
    {
        targetHover = 1.0f;
        repaint();
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        targetHover = 0.0f;
        repaint();
    }

    void mouseMove (const juce::MouseEvent&) override { repaint(); }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        juce::Slider::mouseDrag (e);
        repaint();
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        juce::Slider::mouseUp (e);
        repaint();
    }

private:
    void timerCallback() override
    {
        float current = (float) getProperties().getWithDefault ("hoverAlpha", 0.0f);
        if (std::abs (current - targetHover) < 0.001f)
            return;

        const float step = 0.12f;
        if (current < targetHover)
            current = juce::jmin (targetHover, current + step);
        else
            current = juce::jmax (targetHover, current - step);

        getProperties().set ("hoverAlpha", current);
        repaint();
    }

    float targetHover = 0.0f;
};

// =========================================================
//  Toggle button that repaints when its state changes
// =========================================================
class RepaintOnToggleButton : public juce::TextButton
{
public:
    void buttonStateChanged() override
    {
        juce::TextButton::buttonStateChanged();
        repaint();

        if (auto* parent = getParentComponent())
        {
            auto margin = 20;
            auto r = getBounds().expanded (margin);
            parent->repaint (r);
        }
    }
};

// =========================================================
//  Stereo graph
// =========================================================
class StereoGraph : public juce::Component
{
public:
    StereoGraph (SvarogWiderAudioProcessor& p);
    void paint (juce::Graphics& g) override;
private:
    SvarogWiderAudioProcessor& audioProcessor;
};

// =========================================================
//  LFO graph
// =========================================================
class LfoGraph : public juce::Component
{
public:
    LfoGraph (SvarogWiderAudioProcessor& p, int copyIndex);
    void paint (juce::Graphics& g) override;
private:
    SvarogWiderAudioProcessor& audioProcessor;
    int copyIndex;
    juce::Image trailImage;
};

// =========================================================
//  Low cut graph
// =========================================================
class LowCutGraph : public juce::Component
{
public:
    LowCutGraph (SvarogWiderAudioProcessor& p);
    void paint (juce::Graphics& g) override;
private:
    SvarogWiderAudioProcessor& audioProcessor;
};

class VoiceColorDot : public juce::Component
{
public:
    explicit VoiceColorDot (juce::Colour c) : colour (c)
    {
        setInterceptsMouseClicks (false, false);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.35f;
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();

        g.setColour (colour);
        g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        g.setColour (juce::Colours::white.withAlpha (0.7f));
        g.fillEllipse (cx - 1.5f, cy - 1.5f, 3.0f, 3.0f);
    }

private:
    juce::Colour colour;
};

// =========================================================
//  Main editor
// =========================================================
class SvarogWiderAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    explicit SvarogWiderAudioProcessorEditor (SvarogWiderAudioProcessor&);
    ~SvarogWiderAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    SvarogWiderAudioProcessor& audioProcessor;

    SvarogLookAndFeel lookAndFeel;

    StereoGraph graph;

    // ---- Master output ----
    juce::Label  outputLabel;
    std::unique_ptr<DraggableValueBox> outputValueBox;

    // ---- Original ----
    juce::Label  originalHeaderLabel;
    RepaintOnToggleButton originalOnButton;
    HoverSlider  originalVolumeKnob;
    juce::Label  originalVolumeLabel;
    std::unique_ptr<ButtonAttachment> originalOnAttachment;
    std::unique_ptr<SliderAttachment> originalVolumeAttachment;
    std::unique_ptr<LevelMeterComponent> inputMeter;

    // ---- Low cut ----
    juce::Label  lowCutHeaderLabel;
    RepaintOnToggleButton lowCutOnButton;
    DraggingSlider lowCutSlider;
    std::unique_ptr<DraggableValueBox> lowCutValueBox;
    std::unique_ptr<ButtonAttachment>  lowCutOnAttachment;
    std::unique_ptr<SliderAttachment>  lowCutSliderAttachment;
    std::unique_ptr<LowCutGraph>       lowCutGraph;
    std::unique_ptr<LevelMeterComponent> outputMeter;

    // ---- Per-copy controls ----
    struct CopyControls
    {
        juce::Label headerLabel;
        std::unique_ptr<VoiceColorDot> voiceDot;
        RepaintOnToggleButton onButton;

        HoverSlider volumeKnob;
        juce::Label volumeLabel;

        DraggingSlider   panSlider;
        juce::Label      panLabel;
        std::unique_ptr<DraggableValueBox> panValueBox;

        DraggingSlider   delaySlider;
        juce::Label      delayLabel;
        std::unique_ptr<DraggableValueBox> delayValueBox;

        std::unique_ptr<LfoGraph> lfoGraph;

        juce::Label detuneLabel;
        juce::Label speedLabel;
        juce::Label depthLabel;

        std::unique_ptr<DraggableValueBox> detuneBox;
        std::unique_ptr<DraggableValueBox> speedBox;
        std::unique_ptr<DraggableValueBox> depthBox;

        std::unique_ptr<ButtonAttachment> onAttachment;
        std::unique_ptr<SliderAttachment> volumeAttachment;
        std::unique_ptr<SliderAttachment> panSliderAttachment;
        std::unique_ptr<SliderAttachment> delaySliderAttachment;
    };

float getUiScale() const
    {
        static constexpr float baseW = 800.0f;
        static constexpr float baseH = 600.0f;
        return juce::jmax (0.75f, juce::jmin ((float) getWidth() / baseW, (float) getHeight() / baseH));
    }

    int scaled (int basePx) const
    {
        return juce::roundToInt ((float) basePx * getUiScale());
    }

    int getVoiceColumnContentHeight() const
    {
        const int verticalPad = scaled (8);
        const int topPad      = scaled (8);
        const int headerH     = scaled (28);
        const int boxH        = scaled (22);
        const int trackH      = scaled (24);
        const int lfoH        = scaled (34);

        return verticalPad
             + topPad
             + headerH
             + scaled (28)      // ON button row
             + scaled (100)     // Volume knobArea
             + scaled (8)
             + boxH + trackH    // Pan
             + scaled (6)
             + boxH + trackH    // Delay
             + scaled (2)
             + lfoH             // LFO graph
             + scaled (6)
             + boxH             // Detune
             + scaled (4)
             + boxH             // Speed
             + scaled (4)
             + boxH             // Depth
             + verticalPad;
    }

    CopyControls copyControls[SvarogWiderAudioProcessor::kNumCopies];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SvarogWiderAudioProcessorEditor)
};