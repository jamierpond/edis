#pragma once

#include "Parameters.h"

class EdisAudioProcessor : public PluginHelpers::ProcessorBase
{
public:

    //==============================================================================
    EdisAudioProcessor();

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:

    Parameters parameters;
};
