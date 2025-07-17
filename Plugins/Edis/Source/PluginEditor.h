#pragma once

#include "PluginProcessor.h"

class EdisAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit EdisAudioProcessorEditor(EdisAudioProcessor&);

private:
    void paint(juce::Graphics&) override;
    void resized() override;

    juce::GenericAudioProcessorEditor editor {processor};
};
