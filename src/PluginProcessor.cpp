#include "PluginProcessor.h"
#include "PluginEditor.h"

SvarogWiderAudioProcessor::SvarogWiderAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

SvarogWiderAudioProcessor::~SvarogWiderAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout
SvarogWiderAudioProcessor::createParameterLayout()
{
    // No parameters yet — empty start.
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    return { params.begin(), params.end() };
}

const juce::String SvarogWiderAudioProcessor::getName() const { return JucePlugin_Name; }
bool SvarogWiderAudioProcessor::acceptsMidi()  const { return false; }
bool SvarogWiderAudioProcessor::producesMidi() const { return false; }
bool SvarogWiderAudioProcessor::isMidiEffect() const { return false; }
double SvarogWiderAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int SvarogWiderAudioProcessor::getNumPrograms() { return 1; }
int SvarogWiderAudioProcessor::getCurrentProgram() { return 0; }
void SvarogWiderAudioProcessor::setCurrentProgram (int) {}
const juce::String SvarogWiderAudioProcessor::getProgramName (int) { return {}; }
void SvarogWiderAudioProcessor::changeProgramName (int, const juce::String&) {}

void SvarogWiderAudioProcessor::prepareToPlay (double /*sampleRate*/, int /*samplesPerBlock*/)
{
    // Nothing to prepare yet.
}

void SvarogWiderAudioProcessor::releaseResources() {}

bool SvarogWiderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void SvarogWiderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Passthrough — audio goes through unchanged.
}

bool SvarogWiderAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* SvarogWiderAudioProcessor::createEditor()
{
    return new SvarogWiderAudioProcessorEditor (*this);
}

void SvarogWiderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SvarogWiderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SvarogWiderAudioProcessor();
}