#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace pond {


 /** Used to @ref performOnePoleFilter of a value.
* \f[
* \alpha = e^{\frac{-log(9)}{f_s \cdot T}}
* \f]
* Where \f$ f_s \f$ is the sample rate, and \f$ T \f$ is the delay time.
*/
template <typename T>
static inline T calculate_alpha_value(const T time_seconds, const T fs) noexcept {
  return expf(-logf(9.0f) / (fs * time_seconds));
}

constexpr auto abs = [](auto x) {
  return x < 0 ? -x : x;
};

template <typename T>
constexpr static auto perform_one_pole(const T x, T alpha, const T prev_x) noexcept {
  return ((T{1.0} - alpha) * x) + alpha * prev_x;
}

template<typename T>
constexpr auto ring_mod_sidechain_gain(T sidechain, T amount) {
  // whenever the sidechain value is large we want to attenuate the signal
  return T{1.0} - (pond::abs(sidechain) * amount);
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

        // parameter in ms
        auto attack_s = parameters.attack->get() * 1e-3f;
        auto release_s = parameters.release->get() * 1e-3f;

        if (sideChainInput.getNumChannels() < 1)
        {
            return;
        }

        auto attack_alpha = pond::calculate_alpha_value(attack_s, fs);
        auto release_alpha = pond::calculate_alpha_value(release_s, fs);

        for (auto c = 0; c < mainInputOutput.getNumChannels(); ++c)
        {
            // for size_t
            auto c_sz = static_cast<size_t>(c);
            auto* channel = mainInputOutput.getWritePointer(c);
            auto* sidechain = sideChainInput.getReadPointer(c);

            for (auto i = 0; i < mainInputOutput.getNumSamples(); ++i)
            {
                const auto gain = pond::ring_mod_sidechain_gain(sidechain[i], amount);
                const auto prev_smooth = prev_smoothed_gain[c_sz];
                const auto is_attacking = gain < prev_smooth;
                const auto alpha = is_attacking ? attack_alpha : release_alpha;
                const auto smoothed_gain = pond::perform_one_pole(gain, alpha, prev_smooth);
                channel[i] *= smoothed_gain;
                prev_smoothed_gain[c_sz] = smoothed_gain;
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
