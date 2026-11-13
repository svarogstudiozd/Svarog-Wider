#include "PluginEditor.h"

SvarogWiderAudioProcessorEditor::SvarogWiderAudioProcessorEditor (SvarogWiderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
}

SvarogWiderAudioProcessorEditor::~SvarogWiderAudioProcessorEditor() = default;

void SvarogWiderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (25, 25, 30));
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawFittedText ("Svarog Wider",
                      getLocalBounds(),
                      juce::Justification::centred, 1);
}

void SvarogWiderAudioProcessorEditor::resized()
{
    // Nothing to lay out yet.
}