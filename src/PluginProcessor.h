#pragma once

#include <JuceHeader.h>
#include "LevelMeterSource.h"

class SvarogWiderAudioProcessor : public juce::AudioProcessor
{
public:
    SvarogWiderAudioProcessor();
    ~SvarogWiderAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override;
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    static constexpr int kNumCopies   = 4;
    static constexpr int kMaxChannels = 2;

    static juce::String originalVolumeId()                 { return "originalVolume"; }
    static juce::String originalOnId()                     { return "originalOn"; }
    static juce::String copyVolumeId   (int copy)          { return "copy" + juce::String (copy) + "Volume"; }
    static juce::String copyOnId       (int copy)          { return "copy" + juce::String (copy) + "On"; }
    static juce::String copyDelayId    (int copy)          { return "copy" + juce::String (copy) + "Delay"; }
    static juce::String copyPanId      (int copy)          { return "copy" + juce::String (copy) + "Pan"; }
    static juce::String copyDetuneId   (int copy)          { return "copy" + juce::String (copy) + "Detune"; }
    static juce::String copyModSpeedId (int copy)          { return "copy" + juce::String (copy) + "ModSpeed"; }
    static juce::String copyModDepthId (int copy)          { return "copy" + juce::String (copy) + "ModDepth"; }
    static juce::String lowCutId()                         { return "lowCut"; }
    static juce::String lowCutOnId()                       { return "lowCutOn"; }
    static juce::String outputGainId()                     { return "outputGain"; }

    float getCopyLfoValue (int copyIndex) const;

        // --- Stereo correlation (for the spread meter) ---
    float getStereoCorrelation() const { return stereoCorrelation.load(); }

    LevelMeterSource& getInputMeterSource()  { return inputMeterSource; }
    LevelMeterSource& getOutputMeterSource() { return outputMeterSource; }

        static constexpr int getGonioSize() { return kGonioSize; }
    int getGonioWritePos() const { return gonioWritePos.load(); }
    float getGonioL (int i) const { return gonioL[i]; }
    float getGonioR (int i) const { return gonioR[i]; }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

struct PitchShifter
    {
        float smoothDelaySamples = 0.0f;
        float prevLfo = 0.0f;
        bool  initialized = false;
    };

    struct CopyState
    {
        juce::AudioBuffer<float> delayBuffer;
        int writePos = 0;
        int bufferSize = 0;
        juce::dsp::IIR::Filter<float> lowCut[kMaxChannels];
        PitchShifter shifter[kMaxChannels];
        std::atomic<float> lfoPhase { 0.0f };
    };

    CopyState copies[kNumCopies];
    juce::AudioBuffer<float> scratchBuffer;

    LevelMeterSource inputMeterSource;
    LevelMeterSource outputMeterSource;

        // --- Goniometer ring buffer ---
    static constexpr int kGonioSize = 4096;
    std::atomic<int> gonioWritePos { 0 };
    float gonioL[kGonioSize] { 0.0f };
    float gonioR[kGonioSize] { 0.0f };

    double currentSampleRate = 44100.0;
    float  lastLowCutHz      = -1.0f;

    std::atomic<float> stereoCorrelation { 0.0f };




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SvarogWiderAudioProcessor)
};