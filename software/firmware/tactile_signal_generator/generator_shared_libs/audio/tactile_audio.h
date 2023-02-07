#ifndef __SENSINT_TACTILE_AUDIO_H__
#define __SENSINT_TACTILE_AUDIO_H__

#include <Audio.h>
#include <types.h>

namespace sensint {
namespace tactile_audio {

/**
 * @brief Options for different ways how to route the audio signal from raw wave
 * to the output.
 *
 *     ┌──────────┐      ┌────────┐      ┌────────┐
 * 0 = │ waveform ├─────>│ filter ├─────>│ output │
 *     └──────────┘      └────────┘      └────────┘
 *
 *     ┌──────────┐      ┌────────┐      ┌──────────┐      ┌────────┐
 * 1 = │ waveform ├─────>│ filter ├─────>│ envelope ├─────>│ output │
 *     └──────────┘      └────────┘      └──────────┘      └────────┘
 *
 *     ┌──────────┐      ┌──────────┐      ┌────────┐      ┌────────┐
 * 2 = │ waveform ├─────>│ envelope ├─────>│ filter ├─────>│ output │
 *     └──────────┘      └──────────┘      └────────┘      └────────┘
 *
 *     ┌──────────┐      ┌──────────┐      ┌────────┐
 * 3 = │ waveform ├─────>│ envelope ├─────>│ output │
 *     └──────────┘      └──────────┘      └────────┘
 */
enum class SignalChain : uint8_t {
  //! Since we use an envelope to start and stop a signal, we always need an
  //! envelope in the signal chain.
  // Signal_Filter_Out = 0,
  Signal_Filter_Envelope_Out = 1,
  Signal_Envelope_Filter_Out = 2,
  Signal_Envelope_Out = 3
};

/**
 * @brief A data structure of all audio connection (i.e. patch cords) for a mono
 * signal chain. There are four audio objects (i.e. waveform, envelope, filter,
 * and output) that need to be connected in a given order. This needs just three
 * patch cords. The fourth patch cord is used to connect both sides of the ouput
 * with the same signal. Hence, it is in fact a stereo output but with a single
 * source.
 *                                                 con3
 *  ┌──────────┐ con1 ┌──────────┐ con2 ┌────────┐  ┌──>┌──────────────┐
 *  │ waveform ├─────>│ envelope ├─────>│ filter ├──┤   │ output (l/r) │
 *  └──────────┘      └──────────┘      └────────┘  └──>└──────────────┘
 *                                                 con4
 */
struct MonoSignalChainConnection {
  AudioConnection *con1 = nullptr;
  AudioConnection *con2 = nullptr;
  AudioConnection *con3 = nullptr;
  AudioConnection *con4 = nullptr;
};

/**
 * @brief A data structure of all audio connection (i.e. patch cords) for a
 * stereo signal chain. Each side of the output has its own signal chain (i.e.
 * waveform, envelope, and filter). This needs six patch cords.
 *
 *  ┌──────────┐ con1 ┌──────────┐ con2 ┌────────┐ con3
 *  │ waveform ├─────>│ envelope ├─────>│ filter ├──┐
 *  └──────────┘      └──────────┘      └────────┘  │ l
 *                                                  └──>┌──────────────┐
 *                                                      │ output (l/r) │
 *                                                  ┌──>└──────────────┘
 *  ┌──────────┐      ┌──────────┐      ┌────────┐  │ r
 *  │ waveform ├─────>│ envelope ├─────>│ filter ├──┘
 *  └──────────┘ con4 └──────────┘ con5 └────────┘ con6
 */
struct StereoSignalChainConnection {
  AudioConnection *con1 = nullptr;
  AudioConnection *con2 = nullptr;
  AudioConnection *con3 = nullptr;
  AudioConnection *con4 = nullptr;
  AudioConnection *con5 = nullptr;
  AudioConnection *con6 = nullptr;
};

/**
 * @brief A data structure to combine all audio objects including their
 * connections for a mono signal generator.
 */
struct MonoAudio {
  AudioSynthWaveform raw_signal;
  AudioEffectEnvelope envelope;
  AudioFilterBiquad filter;
  bool is_playing = false;
  elapsedMicros play_time = 0;
};

/**
 * @brief A data structure that represents the actual mono signal generator.
 */
struct MonoAudioChain {
  MonoAudio audio;
  AudioOutputPT8211 output;
  MonoSignalChainConnection patch_cords;
};

/**
 * @brief A data structure that represents the actual stereo signal generator.
 */
struct StereoAudioChain {
  MonoAudio audio_left;
  MonoAudio audio_right;
  AudioOutputPT8211 output;
  StereoSignalChainConnection patch_cords;
};

/**
 * @brief Set the raw signal's parameters of the audio.
 *
 * @param params signal parameters
 * @param signal the audio signal
 */
void ApplyRawSignalParameters(const RawSignalParameters &params, AudioSynthWaveform &signal);

/**
 * @brief Set the envelope parameters of the audio.
 *
 * @param params envelope parameters
 * @param envelope audio envelope
 */
void ApplyEnvelopeParameters(const EnvelopeParameters &params, AudioEffectEnvelope &envelope);

/**
 * @brief Set the filter parameters of the audio.
 *
 * @param params filter parameters
 * @param filter audio filter
 */
void ApplyFilterParameters(const FilterParameters &params, AudioFilterBiquad &filter);

/**
 * @brief Set the mono-signal chain as given by the chain parameter. The same
 * signal will be routed to both sides of the audio output.
 *
 * @param audio_chain audio objects of a mono-signal chain and their patch cords
 * @param chain_selection signal chain configuration
 *           ┌──────────┐      ┌──────────┐      ┌────────┐
 * default : │ waveform ├─────>│ envelope ├─────>│ output │
 *           └──────────┘      └──────────┘      └────────┘
 */
void PatchMonoSignalChain(MonoAudioChain &audio_chain,
                          const SignalChain chain_selection = SignalChain::Signal_Envelope_Out);

/**
 * @brief Set the left stereo-signal chain as given by the chain parameter. The
 * same signal will be routed to both sides of the audio output.
 *
 * @param patch_cords elements to link (patch) audio objects
 * @param audio the mono audio object for this side
 * @param output DAC
 * @param chain_selection signal chain configuration
 *           ┌──────────┐      ┌──────────┐      ┌────────┐
 * default : │ waveform ├─────>│ envelope ├─────>│ output │
 *           └──────────┘      └──────────┘      └────────┘
 */
void PatchLeftStereoSignalChain(
    StereoSignalChainConnection &patch_cords, MonoAudio &audio, AudioOutputPT8211 &output,
    const SignalChain chain_selection = SignalChain::Signal_Envelope_Out);

/**
 * @brief Set the left stereo-signal chain as given by the chain parameter. The
 * same signal will be routed to both sides of the audio output.
 *
 * @param patch_cords elements to link (patch) audio objects
 * @param audio the mono audio object for this side
 * @param output DAC
 * @param chain_selection signal chain configuration
 *           ┌──────────┐      ┌──────────┐      ┌────────┐
 * default : │ waveform ├─────>│ envelope ├─────>│ output │
 *           └──────────┘      └──────────┘      └────────┘
 */
void PatchRightStereoSignalChain(
    StereoSignalChainConnection &patch_cords, MonoAudio &audio, AudioOutputPT8211 &output,
    const SignalChain chain_selection = SignalChain::Signal_Envelope_Out);

/**
 * @brief Set the stereo-signal chain as given by the chain parameter.
 *
 * @param audio_chain audio objects of a stereo-signal chain and their patch
 * cords
 * @param chain_selection_left signal chain configuration
 *           ┌──────────┐      ┌──────────┐      ┌────────┐
 * default : │ waveform ├─────>│ envelope ├─────>│ output │
 *           └──────────┘      └──────────┘      └────────┘
 * @param chain_selection_right signal chain configuration
 *           ┌──────────┐      ┌──────────┐      ┌────────┐
 * default : │ waveform ├─────>│ envelope ├─────>│ output │
 *           └──────────┘      └──────────┘      └────────┘
 */
void PatchStereoSignalChain(
    StereoAudioChain &audio_chain,
    const SignalChain chain_selection_left = SignalChain::Signal_Envelope_Out,
    const SignalChain chain_selection_right = SignalChain::Signal_Envelope_Out);

/**
 * @brief Set the grain parameters of the audio. This includes the raw
 * signal, the envelope, and the filter.
 *
 * @param params grain parameters
 * @param audio audio object
 */
void ApplyGrainParameters(const GrainParameters &params, MonoAudio &audio);

/**
 * @brief Find the closest grain index based on a given sensor value. We apply a
 * binary search scheme.
 *
 * @param sequence vector of grains with their positions
 * @param sensor_val the sensor value to match with a grain position
 * @return int index of the closest grain
 */
int FindClosestGrainIndex(const std::vector<sensint::Grain> &sequence,
                          const sensint::analog_sensor_t sensor_val);

}  // namespace tactile_audio
}  // namespace sensint

#endif  // __SENSINT_TACTILE_AUDIO_H__
