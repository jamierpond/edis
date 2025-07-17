#pragma once

#include <shared_plugin_helpers/shared_plugin_helpers.h>

template <typename T>
struct TimeParamRanges {
  constexpr static auto start = T{0.0};
  constexpr static auto end = T{100.0};
  constexpr static auto interval = T{0.0};
  constexpr static auto skew = T{0.1f};
  constexpr static auto default_value = T{0.0};
};

using T = TimeParamRanges<float>;

struct Parameters
{
    void add(juce::AudioProcessor& processor) const
    {
        processor.addParameter(amount);
        processor.addParameter(enable);
        processor.addParameter(attack);
        processor.addParameter(release);
    }
//     AudioParameterFloat (const ParameterID& parameterID,
//                          const String& parameterName,
//                          NormalisableRange<float> normalisableRange,
//                          float defaultValue,
//                          const AudioParameterFloatAttributes& attributes = {});

    //Raw pointers. They will be owned by either the processor or the APVTS (if you use it)
    juce::AudioParameterFloat* amount =
        new juce::AudioParameterFloat({"Amount", 1}, "Amount", 0.0f, 1.0f, 1.0f);

    juce::AudioParameterBool* enable =
        new juce::AudioParameterBool({"Enable", 1}, "Enable", true);

    juce::AudioParameterFloat* attack =
        new juce::AudioParameterFloat({"Attack", 1}, "Attack", {T::start, T::end, T::interval, T::skew}, T::default_value);

    juce::AudioParameterFloat* release =
        new juce::AudioParameterFloat({"Release", 1}, "Release", {T::start, T::end, T::interval, T::skew}, T::default_value);
};