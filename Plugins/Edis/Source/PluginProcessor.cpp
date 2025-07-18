#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace pond {


 /** Used to @ref perform_one_pole of a value.
* \f[
* \alpha = e^{\frac{-log(9)}{f_s \cdot T}}
* \f]
* Where \f$ f_s \f$ is the sample rate, and \f$ T \f$ is the delay time.
*/
template <typename T>
static inline T calculate_alpha_value(const T time_seconds, const T fs) noexcept {
  return expf(-logf(9.0f) / (fs * time_seconds));
}

template <typename T>
constexpr auto abs(T x) {
  return x < T{0} ? -x : x;
}

template <typename T>
constexpr static auto perform_one_pole(const T x, T alpha, const T prev_x) noexcept {
  return ((T{1.0} - alpha) * x) + alpha * prev_x;
}

template <typename T>
struct RingModArgs {
  T input;
  T sidechain;
  T amount;
//   T prev_smooth;
//   T attack_alpha;
//   T release_alpha;
};

template <typename T>
constexpr static auto attack_release_smoothing(
    const T x,
    T& prev_smooth,
    const T attack_alpha,
    const T release_alpha) noexcept {
  auto is_attacking = x > prev_smooth;
  auto alpha = is_attacking ? attack_alpha : release_alpha;
  auto smoothed_gain = pond::perform_one_pole(x, alpha, prev_smooth);
  return smoothed_gain;
}

template <typename T>
constexpr static auto rm_sidechain = [](const RingModArgs<T>& args, auto&& gain_filter_fn) noexcept {
    auto [channel, sidechain, amount] = args;
    auto gain = pond::abs(sidechain) * amount;
    auto rescaled_gain = 1.0f - gain;
    auto smoothed_gain = gain_filter_fn(rescaled_gain);
    return smoothed_gain;
};



} // namespace pond


EdisAudioProcessor::EdisAudioProcessor()
    : ProcessorBase(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    parameters.add(*this);
}

void EdisAudioProcessor::processBlock(juce::AudioBuffer<float>& inputs,
                                                   juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    if (!parameters.enable->get()) {
      return;
    }

    auto buffer = getBusBuffer(inputs, true, 0);
    auto sidechain_buffer = getBusBuffer(inputs, true, 1);

    auto amount = parameters.amount->get();

    // parameter in ms
    auto attack_s = parameters.attack->get() * 1e-3f;
    auto release_s = parameters.release->get() * 1e-3f;

    if (sidechain_buffer.getNumChannels() < 1) {
        return;
    }

     auto attack_alpha = pond::calculate_alpha_value(attack_s, fs);
     auto release_alpha = pond::calculate_alpha_value(release_s, fs);

    for (auto c = 0; c < buffer.getNumChannels(); ++c) {
        // for size_t
        auto c_sz = static_cast<size_t>(c);
        auto* channel = buffer.getWritePointer(c);
        auto* sidechain = sidechain_buffer.getReadPointer(c);

        for (auto i = 0; i < buffer.getNumSamples(); ++i) {
            auto prev_smooth = prev_smoothed_gain[c_sz];
            auto attack_release_smoothing_fn = [&prev_smooth, attack_alpha, release_alpha](auto x) noexcept {
                return pond::attack_release_smoothing(x, prev_smooth, attack_alpha, release_alpha);
            };

            auto gain = pond::rm_sidechain<float>({
                .input = channel[i],
                .sidechain = sidechain[i],
                .amount = amount,
            }, attack_release_smoothing_fn);

            channel[i] *= gain;
            prev_smoothed_gain[c_sz] = gain;
        }
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
