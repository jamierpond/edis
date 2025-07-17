#pragma once

#include "Parameters.h"

class EdisAudioProcessor : public PluginHelpers::ProcessorBase
{
public:

    //==============================================================================
    EdisAudioProcessor();

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
      fs = static_cast<float>(sampleRate);
      samples_per_block = samplesPerBlock;
    }

    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:

    constexpr static auto MaxNumChannels = 2;
    std::array<float, MaxNumChannels> prev_smoothed_gain{};

    float fs{};
    int samples_per_block{};

    Parameters parameters;
};
