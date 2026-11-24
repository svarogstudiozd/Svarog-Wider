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
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ---- Original ----
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { originalVolumeId(), 1 }, "Original Volume",
        juce::NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f, "dB"));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { originalOnId(), 1 }, "Original On", true));

    // ---- Voices ----„„
    for (int i = 1; i <= kNumCopies; ++i)
    {
        const auto suffix = juce::String (i);

        // Per-voice defaults
        bool  defaultOn     = false;
        float defaultDelay  = 0.0f;
        float defaultDetune = 0.0f;
        float defaultSpeed  = 0.05f;   // minimum, effectively "off"
        float defaultDepth  = 0.0f;
        float defaultPan    = 0.0f;
        float defaultVolume = 0.0f;

        if (i == 1)
        {
            defaultOn     = true;
            defaultDelay  = 20.0f;
            defaultDetune = 9.0f;
            defaultSpeed  = 0.7f;
            defaultDepth  = 20.0f;
            defaultPan    = -100.0f;
        }
        else if (i == 2)
        {
            defaultOn     = true;
            defaultDelay  = 25.0f;
            defaultDetune = -9.0f;
            defaultSpeed  = 0.9f;
            defaultDepth  = -20.0f;
            defaultPan    = 100.0f;
        }
        // Voices 3 and 4 use the neutral defaults above (off, 0, etc.)

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { copyOnId (i), 1 }, "Voice " + suffix + " On", defaultOn));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { copyVolumeId (i), 1 }, "Voice " + suffix + " Volume",
            juce::NormalisableRange<float> (-24.0f, 12.0f, 0.1f), defaultVolume, "dB"));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { copyDelayId (i), 1 }, "Voice " + suffix + " Delay",
            juce::NormalisableRange<float> (0.0f, 50.0f, 0.1f), defaultDelay, "ms"));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { copyPanId (i), 1 }, "Voice " + suffix + " Pan",
            juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f), defaultPan));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { copyDetuneId (i), 1 }, "Voice " + suffix + " Detune",
            juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f), defaultDetune, "ct"));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { copyModSpeedId (i), 1 }, "Voice " + suffix + " Mod Speed",
            juce::NormalisableRange<float> (0.05f, 5.0f, 0.01f), defaultSpeed, "Hz"));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { copyModDepthId (i), 1 }, "Voice " + suffix + " Mod Depth",
            juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f), defaultDepth, "%"));
    }

    // ---- Global ----
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { lowCutId(), 1 }, "Low Cut",
        juce::NormalisableRange<float> (20.0f, 500.0f, 0.1f, 0.5f), 100.0f, "Hz"));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { lowCutOnId(), 1 }, "Low Cut On", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { outputGainId(), 1 }, "Output Gain",
        juce::NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f, "dB"));

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

float SvarogWiderAudioProcessor::getCopyLfoValue (int copyIndex) const
{
    if (copyIndex >= 0 && copyIndex < kNumCopies)
        return copies[copyIndex].lfoPhase.load();
    return 0.0f;
}

void SvarogWiderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    const int maxDelaySamples = static_cast<int> (sampleRate * 0.1) + 200;
    const int bufferSize      = maxDelaySamples + 4;

    const float lowCutHz = apvts.getRawParameterValue (lowCutId())->load();
    const auto  coeffs   = juce::dsp::IIR::Coefficients<float>::makeHighPass (
                               currentSampleRate, juce::jmax (20.0f, lowCutHz), 0.707f);

    for (auto& copy : copies)
    {
        copy.bufferSize = bufferSize;
        copy.writePos   = 0;
        copy.delayBuffer.setSize (kMaxChannels, bufferSize);
        copy.delayBuffer.clear();
        copy.lfoPhase.store (0.0f);

for (int ch = 0; ch < kMaxChannels; ++ch)
        {
            copy.lowCut[ch].reset();
            copy.lowCut[ch].coefficients = coeffs;
            copy.shifter[ch].smoothDelaySamples = 0.0f;
            copy.shifter[ch].initialized = false;
        }
    }

    lastLowCutHz = lowCutHz;
    scratchBuffer.setSize (kMaxChannels, samplesPerBlock);

    inputMeterSource.reset();
    outputMeterSource.reset();
}

void SvarogWiderAudioProcessor::releaseResources()
{
    for (auto& copy : copies)
        copy.delayBuffer.setSize (0, 0);
    scratchBuffer.setSize (0, 0);
}

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

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = juce::jmin (buffer.getNumChannels(), kMaxChannels);
    if (numSamples == 0 || numChannels == 0) return;

    if (scratchBuffer.getNumSamples() < numSamples)
        scratchBuffer.setSize (kMaxChannels, numSamples, false, false, true);

    inputMeterSource.pushSamples (buffer, 0, numSamples);

    const bool origOn = apvts.getRawParameterValue (originalOnId())->load() > 0.5f;
    const float origDb = apvts.getRawParameterValue (originalVolumeId())->load();
    const float originalGain = origOn ? juce::Decibels::decibelsToGain (origDb) : 0.0f;
    const float lowCutHz = apvts.getRawParameterValue (lowCutId())->load();
    const bool  lowCutOn = apvts.getRawParameterValue (lowCutOnId())->load() > 0.5f;

    if (std::abs (lowCutHz - lastLowCutHz) > 0.01f)
    {
        const auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (
                                currentSampleRate, juce::jmax (20.0f, lowCutHz), 0.707f);
        for (auto& copy : copies)
            for (auto& f : copy.lowCut)
                f.coefficients = coeffs;
        lastLowCutHz = lowCutHz;
    }

    float copyGain[kNumCopies], copyDelayMs[kNumCopies], copyPan[kNumCopies];
    float copyDetune[kNumCopies], copyModSpeed[kNumCopies], copyModDepth[kNumCopies];
    bool  copyOn[kNumCopies];

    for (int i = 0; i < kNumCopies; ++i)
    {
        const int id = i + 1;
        copyOn[i]       = apvts.getRawParameterValue (copyOnId (id))->load() > 0.5f;
        copyGain[i]     = juce::Decibels::decibelsToGain (
                            apvts.getRawParameterValue (copyVolumeId (id))->load());
        copyDelayMs[i]  = apvts.getRawParameterValue (copyDelayId  (id))->load();
        copyPan[i]      = apvts.getRawParameterValue (copyPanId    (id))->load();
        copyDetune[i]   = apvts.getRawParameterValue (copyDetuneId (id))->load();
        copyModSpeed[i] = apvts.getRawParameterValue (copyModSpeedId (id))->load();
        copyModDepth[i] = apvts.getRawParameterValue (copyModDepthId (id))->load() / 100.0f;
    }

    scratchBuffer.clear();
    for (int ch = 0; ch < numChannels; ++ch)
        scratchBuffer.addFrom (ch, 0, buffer, ch, 0, numSamples, originalGain);

    const float phaseInc = juce::MathConstants<float>::twoPi / static_cast<float> (currentSampleRate);

for (int c = 0; c < kNumCopies; ++c)
    {
        auto& copy = copies[c];

        if (!copyOn[c])
        {
            for (int i = 0; i < numSamples; ++i)
            {
                for (int ch = 0; ch < numChannels; ++ch)
                    copy.delayBuffer.setSample (ch, copy.writePos, buffer.getSample (ch, i));
                copy.writePos = (copy.writePos + 1) % copy.bufferSize;
            }
            continue;
        }

        const float maxDelay = static_cast<float> (juce::jmax (32, copy.bufferSize - 16));
        const float baseDelaySamples = juce::jlimit (
            16.0f,
            maxDelay - 64.0f,
            static_cast<float> (copyDelayMs[c] * 0.001 * currentSampleRate));

        const float panNorm = juce::jlimit (-1.0f, 1.0f, copyPan[c] / 100.0f);
        const float angle   = (panNorm + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
        const float gainL   = std::cos (angle);
        const float gainR   = std::sin (angle);

        const float lfoInc = copyModSpeed[c] * phaseInc;

        // Static detune in semitones/cents adds subtle chorus/doubler character
        // while LFO depth smoothly modulates the delay time for a natural wobble
        const float detuneRatio = std::pow (2.0f, copyDetune[c] / 1200.0f);
        const float wobbleDepthMs = copyModDepth[c] * 5.0f; // up to +-3.5 ms at 100% depth
        const float wobbleDepthSamples = wobbleDepthMs * 0.001f * static_cast<float> (currentSampleRate);

        float currentLfoPhase = copy.lfoPhase.load();

        for (int i = 0; i < numSamples; ++i)
        {
            const float lfoValue = std::sin (currentLfoPhase);
            currentLfoPhase += lfoInc;
            if (currentLfoPhase >= juce::MathConstants<float>::twoPi)
                currentLfoPhase -= juce::MathConstants<float>::twoPi;

            // Target delay combines the voice's Delay setting + LFO wobble
            const float targetDelay = juce::jlimit (
                8.0f,
                maxDelay,
                baseDelaySamples + (lfoValue * wobbleDepthSamples));

            float outL = 0.0f, outR = 0.0f;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto& tap = copy.shifter[ch];

                if (!tap.initialized)
                {
                    tap.smoothDelaySamples = targetDelay;
                    tap.initialized = true;
                }
                else
                {
                    // Light smoothing prevents clicks when parameters move
                    tap.smoothDelaySamples += 0.2f * (targetDelay - tap.smoothDelaySamples);
                }

                // Read at a pitch-shifted rate relative to writePos if detune != 0,
                // or directly at smoothDelaySamples for smooth LFO wobble
                float readPos = static_cast<float> (copy.writePos) - tap.smoothDelaySamples;
                while (readPos < 0.0f)
                    readPos += static_cast<float> (copy.bufferSize);
                while (readPos >= static_cast<float> (copy.bufferSize))
                    readPos -= static_cast<float> (copy.bufferSize);

                const int i0 = static_cast<int> (readPos);
                const float frac = readPos - static_cast<float> (i0);
                const int i1 = (i0 + 1) % copy.bufferSize;

                // Cubic Hermite interpolation for clean, non-grainy audio
                const int iPrev = (i0 - 1 + copy.bufferSize) % copy.bufferSize;
                const int iNext2 = (i1 + 1) % copy.bufferSize;

                const float y0 = copy.delayBuffer.getSample (ch, iPrev);
                const float y1 = copy.delayBuffer.getSample (ch, i0);
                const float y2 = copy.delayBuffer.getSample (ch, i1);
                const float y3 = copy.delayBuffer.getSample (ch, iNext2);

                const float c0 = y1;
                const float c1 = 0.5f * (y2 - y0);
                const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
                const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

                float voiceSample = ((c3 * frac + c2) * frac + c1) * frac + c0;

                // Subtle detune blend if Detune knob is non-zero
                if (std::abs (copyDetune[c]) > 0.05f)
                {
                    const float detuneBlend = juce::jlimit (0.0f, 0.8f, std::abs (copyDetune[c]) / 100.0f);
                    float detuneReadPos = static_cast<float> (copy.writePos)
                                        - (tap.smoothDelaySamples * detuneRatio);
                    while (detuneReadPos < 0.0f)
                        detuneReadPos += static_cast<float> (copy.bufferSize);
                    while (detuneReadPos >= static_cast<float> (copy.bufferSize))
                        detuneReadPos -= static_cast<float> (copy.bufferSize);

                    const int d0 = static_cast<int> (detuneReadPos);
                    const float dFrac = detuneReadPos - static_cast<float> (d0);
                    const int d1 = (d0 + 1) % copy.bufferSize;
                    const float dSample = copy.delayBuffer.getSample (ch, d0) * (1.0f - dFrac)
                                        + copy.delayBuffer.getSample (ch, d1) * dFrac;

                    voiceSample = voiceSample * (1.0f - detuneBlend) + dSample * detuneBlend;
                }

                if (lowCutOn)
                    voiceSample = copy.lowCut[ch].processSample (voiceSample);

                if (ch == 0) outL = voiceSample;
                else         outR = voiceSample;
            }

            auto sanitise = [] (float s)
            {
                return std::isfinite (s) ? juce::jlimit (-10.0f, 10.0f, s) : 0.0f;
            };

            if (numChannels >= 2)
            {
                scratchBuffer.addSample (0, i, sanitise (outL * gainL * copyGain[c]));
                scratchBuffer.addSample (1, i, sanitise (outR * gainR * copyGain[c]));
            }
            else
            {
                scratchBuffer.addSample (0, i, sanitise (outL * copyGain[c]));
            }

            for (int ch = 0; ch < numChannels; ++ch)
                copy.delayBuffer.setSample (ch, copy.writePos, buffer.getSample (ch, i));

            copy.writePos = (copy.writePos + 1) % copy.bufferSize;
        }

        copy.lfoPhase.store (currentLfoPhase);
    }

    for (int ch = 0; ch < numChannels; ++ch)
        buffer.copyFrom (ch, 0, scratchBuffer, ch, 0, numSamples);

        // --- Compute stereo correlation (for the spread meter) ---
    if (numChannels >= 2)
    {
        const auto* left  = buffer.getReadPointer (0);
        const auto* right = buffer.getReadPointer (1);

        float sumLR = 0.0f;
        float sumLL = 0.0f;
        float sumRR = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            sumLR += left[i] * right[i];
            sumLL += left[i] * left[i];
            sumRR += right[i] * right[i];
        }

        const float eps = 1.0e-9f;
        float correlation = sumLR / std::sqrt ((sumLL + eps) * (sumRR + eps));

        // Smooth it so the meter doesn't jitter
        float current = stereoCorrelation.load();
        stereoCorrelation.store (current * 0.85f + correlation * 0.15f);
    }

    // --- Fill the goniometer ring buffer ---
    if (numChannels >= 2)
    {
        const auto* left  = buffer.getReadPointer (0);
        const auto* right = buffer.getReadPointer (1);
        int wp = gonioWritePos.load();

        for (int i = 0; i < numSamples; ++i)
        {
            gonioL[wp] = left[i];
            gonioR[wp] = right[i];
            wp = (wp + 1) & (kGonioSize - 1);
        }

        gonioWritePos.store (wp);
    }

        // Apply master output gain
    {
        const float outputDb = apvts.getRawParameterValue (outputGainId())->load();
        const float outputGain = juce::Decibels::decibelsToGain (outputDb);
        if (std::abs (outputGain - 1.0f) > 0.001f)
        {
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.applyGain (ch, 0, numSamples, outputGain);
        }
    }

    outputMeterSource.pushSamples (buffer, 0, numSamples);

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