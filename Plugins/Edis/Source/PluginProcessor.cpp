#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace pond {

constexpr auto abs = [](auto x) {
  return x < 0 ? -x : x;
};

template <typename T>
constexpr static auto perform_one_pole(const T x, T alpha, const T prev_x) noexcept {
  return ((T{1.0} - alpha) * x) + alpha * prev_x;
}

template<typename T>
constexpr auto ring_mod_sidechain(T signal, T sidechain, T amount) {
  // whenever the sidechain value is large we want to attenuate the signal
  auto gain = T{1.0} - (pond::abs(sidechain) * amount);
  return signal * gain;
}

} // namespace pond


EdisAudioProcessor::EdisAudioProcessor()
    : ProcessorBase(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    parameters.add(*this);
}

void EdisAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                   juce::MidiBuffer& midiMessages)

{
    juce::ignoreUnused(midiMessages);

    if (parameters.enable->get())
    {
        auto mainInputOutput = getBusBuffer(buffer, true, 0);
        auto sideChainInput = getBusBuffer(buffer, true, 1);

        auto amount = parameters.amount->get();

        if (sideChainInput.getNumChannels() < 1)
        {
            return;
        }

        for (int j = 0; j < mainInputOutput.getNumChannels(); ++j)
        {
            auto* channel = mainInputOutput.getWritePointer(j);
            auto* sidechain = sideChainInput.getReadPointer(j);

            for (int i = 0; i < mainInputOutput.getNumSamples(); ++i)
            {
                channel[i] = pond::ring_mod_sidechain(channel[i], sidechain[i], amount);
            }
        }
    }
    else
    {
        buffer.clear();
    }
}

juce::AudioProcessorEditor* EdisAudioProcessor::createEditor()
{
    return new EdisAudioProcessorEditor(*this);
}

void EdisAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    //Serializes your parameters, and any other potential data into an XML:

    auto params = PluginHelpers::saveParamsTree(*this);

    auto pluginPreset = juce::ValueTree(getName());
    pluginPreset.appendChild(params, nullptr);
    //This a good place to add any non-parameters to your preset

    copyXmlToBinary(*pluginPreset.createXml(), destData);
}

void EdisAudioProcessor::setStateInformation(const void* data,
                                                          int sizeInBytes)
{
    //Loads your parameters, and any other potential data from an XML:

    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        auto preset = juce::ValueTree::fromXml(*xml);
        auto params = preset.getChildWithName("Params");

        PluginHelpers::loadParamsTree(*this, params);

        //Load your non-parameter data now
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EdisAudioProcessor();
}
