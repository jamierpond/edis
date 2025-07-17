#pragma once

#include <shared_plugin_helpers/shared_plugin_helpers.h>

struct Parameters
{
    void add(juce::AudioProcessor& processor) const
    {
        processor.addParameter(amount);
        processor.addParameter(enable);
        processor.addParameter(attack);
        processor.addParameter(release);

    }

    //Raw pointers. They will be owned by either the processor or the APVTS (if you use it)
    juce::AudioParameterFloat* amount =
        new juce::AudioParameterFloat({"Amount", 1}, "Amount", 0.0f, 1.0f, 1.0f);

    juce::AudioParameterBool* enable =
        new juce::AudioParameterBool({"Enable", 1}, "Enable", true);

    juce::AudioParameterFloat* attack =
        new juce::AudioParameterFloat({"Attack", 1}, "Attack", 0.0f, 100.0f, 0.0f);

    juce::AudioParameterFloat* release =
        new juce::AudioParameterFloat({"Release", 1}, "Release", 0.0f, 100.0f, 0.0f);
};