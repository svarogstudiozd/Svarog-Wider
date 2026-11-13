#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SvarogWiderAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SvarogWiderAudioProcessorEditor (SvarogWiderAudioProcessor&);
    ~SvarogWiderAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SvarogWiderAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SvarogWiderAudioProcessorEditor)
};