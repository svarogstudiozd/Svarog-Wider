#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"
#include "BinaryData.h"

class SvarogLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SvarogLookAndFeel() = default;

    static juce::Typeface::Ptr getInterBold();
    static juce::Typeface::Ptr getInterRegular();

    juce::Font getLabelFont (juce::Label& label) override;
    juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override;

    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics& g,
                         juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    void drawLinearSlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           juce::Slider::SliderStyle style,
                           juce::Slider& slider) override;

    void drawPanel (juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour fill) const;
    void drawWell (juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour fill) const;
    void drawRaisedControl (juce::Graphics& g,
                            juce::Rectangle<float> bounds,
                            juce::Colour base,
                            bool isHighlighted = false,
                            bool isDown = false,
                            bool isGlowing = false,
                            float cornerRadius = Radius::control) const;
    void drawRecessedControl (juce::Graphics& g,
                              juce::Rectangle<float> bounds,
                              juce::Colour fill,
                              float cornerRadius = Radius::control) const;

    void drawControlShadow (juce::Graphics& g, juce::Rectangle<float> bounds) const;
    void drawActiveControlShadow (juce::Graphics& g, juce::Rectangle<float> bounds) const;
    void drawControlGlow (juce::Graphics& g, juce::Rectangle<float> bounds) const;
    void drawTileShadow (juce::Graphics& g, juce::Rectangle<float> bounds) const;
    void drawPanelShadow (juce::Graphics& g, juce::Rectangle<float> bounds) const;

    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override;
};