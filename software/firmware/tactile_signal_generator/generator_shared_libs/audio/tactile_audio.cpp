#include "tactile_audio.h"

#include <debug.h>

#include <cmath>

namespace sensint {
namespace tactile_audio {

void ApplyRawSignalParameters(const RawSignalParameters &params, AudioSynthWaveform &signal) {
  signal.begin(static_cast<short>(params.waveform));
  signal.frequency(params.frequency);
  signal.amplitude(params.amplitude);

#ifdef SENSINT_DEBUG
  debug::Log("ApplyRawSignalParameters", debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
}

void ApplyEnvelopeParameters(const EnvelopeParameters &params, AudioEffectEnvelope &envelope) {
  envelope.attack(params.attack);
  envelope.hold(0.f);
  envelope.decay(params.decay);
  envelope.sustain(params.sustain);
  envelope.release(params.release);

#ifdef SENSINT_DEBUG
  debug::Log("ApplyEnvelopeParameters", debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
}

void ApplyFilterParameters(const FilterParameters &params, AudioFilterBiquad &filter) {
  filter.setHighpass(0, params.highCutFrequency, params.highCutResonance);
  // filter.setBandpass(1, 200, 0.7) //currently not used
  // filter.setNotch(2, 200, 0.1) //currently not used
  filter.setLowpass(3, params.lowCutFrequency, params.lowCutResonance);

#ifdef SENSINT_DEBUG
  debug::Log("ApplyFilterParameters", debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
}

void PatchMonoSignalChain(MonoAudioChain &audio_chain, const SignalChain chain_selection) {
  if (audio_chain.patch_cords.con1) {
    delete audio_chain.patch_cords.con1;
  }
  if (audio_chain.patch_cords.con2) {
    delete audio_chain.patch_cords.con2;
  }
  if (audio_chain.patch_cords.con3) {
    delete audio_chain.patch_cords.con3;
  }
  if (audio_chain.patch_cords.con4) {
    delete audio_chain.patch_cords.con4;
  }

  switch (chain_selection) {
    /*  ┌──────────┐      ┌────────┐      ┌────────┐
     *  │ waveform ├─────>│ filter ├─────>│ output │
     *  └──────────┘      └────────┘      └────────┘
     */
    //! Since we use an envelope to start and stop a signal, we always need an
    //! envelope in the signal chain.
    // case SignalChain::Signal_Filter_Out:
    //   audio_chain.patch_cords.con1 = new AudioConnection(
    //       audio_chain.audio.raw_signal, audio_chain.audio.filter);
    //   audio_chain.patch_cords.con2 = new AudioConnection(
    //       audio_chain.audio.filter, 0, audio_chain.audio.output, 0);
    //   audio_chain.patch_cords.con3 = new AudioConnection(
    //       audio_chain.audio.filter, 0, audio_chain.audio.output, 1);
    //   break;
    /*  ┌──────────┐      ┌──────────┐      ┌────────┐      ┌────────┐
     *  │ waveform ├─────>│ envelope ├─────>│ filter ├─────>│ output │
     *  └──────────┘      └──────────┘      └────────┘      └────────┘
     */
    case SignalChain::Signal_Envelope_Filter_Out:
      audio_chain.patch_cords.con1 =
          new AudioConnection(audio_chain.audio.raw_signal, audio_chain.audio.envelope);
      audio_chain.patch_cords.con2 =
          new AudioConnection(audio_chain.audio.envelope, audio_chain.audio.filter);
      audio_chain.patch_cords.con3 =
          new AudioConnection(audio_chain.audio.filter, 0, audio_chain.output, 0);
      audio_chain.patch_cords.con4 =
          new AudioConnection(audio_chain.audio.filter, 0, audio_chain.output, 1);
      break;
    /*  ┌──────────┐      ┌────────┐      ┌──────────┐      ┌────────┐
     *  │ waveform ├─────>│ filter ├─────>│ envelope ├─────>│ output │
     *  └──────────┘      └────────┘      └──────────┘      └────────┘
     */
    case SignalChain::Signal_Filter_Envelope_Out:
      audio_chain.patch_cords.con1 =
          new AudioConnection(audio_chain.audio.raw_signal, audio_chain.audio.filter);
      audio_chain.patch_cords.con2 =
          new AudioConnection(audio_chain.audio.filter, audio_chain.audio.envelope);
      audio_chain.patch_cords.con3 =
          new AudioConnection(audio_chain.audio.envelope, 0, audio_chain.output, 0);
      audio_chain.patch_cords.con4 =
          new AudioConnection(audio_chain.audio.envelope, 0, audio_chain.output, 1);
      break;
    /*  ┌──────────┐      ┌──────────┐      ┌────────┐
     *  │ waveform ├─────>│ envelope ├─────>│ output │
     *  └──────────┘      └──────────┘      └────────┘
     */
    case SignalChain::Signal_Envelope_Out:
      audio_chain.patch_cords.con1 =
          new AudioConnection(audio_chain.audio.raw_signal, audio_chain.audio.envelope);
      audio_chain.patch_cords.con2 =
          new AudioConnection(audio_chain.audio.envelope, 0, audio_chain.output, 0);
      audio_chain.patch_cords.con3 =
          new AudioConnection(audio_chain.audio.envelope, 0, audio_chain.output, 1);
      break;
  }

#ifdef SENSINT_DEBUG
  debug::Log("PatchMonoSignalChain", debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
}

void PatchLeftStereoSignalChain(StereoSignalChainConnection &patch_cords, MonoAudio &audio,
                                AudioOutputPT8211 &output, const SignalChain chain_selection) {
  if (patch_cords.con1) {
    delete patch_cords.con1;
  }
  if (patch_cords.con2) {
    delete patch_cords.con2;
  }
  if (patch_cords.con3) {
    delete patch_cords.con3;
  }

  switch (chain_selection) {
    /*  ┌──────────┐      ┌────────┐      ┌────────┐
     *  │ waveform ├─────>│ filter ├─────>│ output │
     *  └──────────┘      └────────┘      └────────┘
     */
    //! Since we use an envelope to start and stop a signal, we always need an
    //! envelope in the signal chain.
    // case SignalChain::Signal_Filter_Out:
    //   patch_cords.con1 = new AudioConnection(audio.raw_signal, audio.filter);
    //   patch_cords.con2 = new AudioConnection(audio.filter, 0, audio.output,
    //   0); break;
    /*  ┌──────────┐      ┌──────────┐      ┌────────┐      ┌────────┐
     *  │ waveform ├─────>│ envelope ├─────>│ filter ├─────>│ output │
     *  └──────────┘      └──────────┘      └────────┘      └────────┘
     */
    case SignalChain::Signal_Envelope_Filter_Out:
      patch_cords.con1 = new AudioConnection(audio.raw_signal, audio.envelope);
      patch_cords.con2 = new AudioConnection(audio.envelope, audio.filter);
      patch_cords.con3 = new AudioConnection(audio.filter, 0, output, 0);
      break;
    /*  ┌──────────┐      ┌────────┐      ┌──────────┐      ┌────────┐
     *  │ waveform ├─────>│ filter ├─────>│ envelope ├─────>│ output │
     *  └──────────┘      └────────┘      └──────────┘      └────────┘
     */
    case SignalChain::Signal_Filter_Envelope_Out:
      patch_cords.con1 = new AudioConnection(audio.raw_signal, audio.filter);
      patch_cords.con2 = new AudioConnection(audio.filter, audio.envelope);
      patch_cords.con3 = new AudioConnection(audio.envelope, 0, output, 0);
      break;
    /*  ┌──────────┐      ┌──────────┐      ┌────────┐
     *  │ waveform ├─────>│ envelope ├─────>│ output │
     *  └──────────┘      └──────────┘      └────────┘
     */
    case SignalChain::Signal_Envelope_Out:
      patch_cords.con1 = new AudioConnection(audio.raw_signal, audio.envelope);
      patch_cords.con2 = new AudioConnection(audio.envelope, 0, output, 0);
      break;
  }
}

void PatchRightStereoSignalChain(StereoSignalChainConnection &patch_cords, MonoAudio &audio,
                                 AudioOutputPT8211 &output, const SignalChain chain_selection) {
  if (patch_cords.con4) {
    delete patch_cords.con4;
  }
  if (patch_cords.con5) {
    delete patch_cords.con5;
  }
  if (patch_cords.con6) {
    delete patch_cords.con6;
  }

  switch (chain_selection) {
    /*  ┌──────────┐      ┌────────┐      ┌────────┐
     *  │ waveform ├─────>│ filter ├─────>│ output │
     *  └──────────┘      └────────┘      └────────┘
     */
    //! Since we use an envelope to start and stop a signal, we always need an
    //! envelope in the signal chain.
    // case SignalChain::Signal_Filter_Out:
    //   patch_cords.con4 = new AudioConnection(audio.raw_signal, audio.filter);
    //   patch_cords.con5 = new AudioConnection(audio.filter, 0, audio.output,
    //   1); break;
    /*  ┌──────────┐      ┌──────────┐      ┌────────┐      ┌────────┐
     *  │ waveform ├─────>│ envelope ├─────>│ filter ├─────>│ output │
     *  └──────────┘      └──────────┘      └────────┘      └────────┘
     */
    case SignalChain::Signal_Envelope_Filter_Out:
      patch_cords.con4 = new AudioConnection(audio.raw_signal, audio.envelope);
      patch_cords.con5 = new AudioConnection(audio.envelope, audio.filter);
      patch_cords.con6 = new AudioConnection(audio.filter, 0, output, 1);
      break;
    /*  ┌──────────┐      ┌────────┐      ┌──────────┐      ┌────────┐
     *  │ waveform ├─────>│ filter ├─────>│ envelope ├─────>│ output │
     *  └──────────┘      └────────┘      └──────────┘      └────────┘
     */
    case SignalChain::Signal_Filter_Envelope_Out:
      patch_cords.con4 = new AudioConnection(audio.raw_signal, audio.filter);
      patch_cords.con5 = new AudioConnection(audio.filter, audio.envelope);
      patch_cords.con6 = new AudioConnection(audio.envelope, 0, output, 1);
      break;
    /*  ┌──────────┐      ┌──────────┐      ┌────────┐
     *  │ waveform ├─────>│ envelope ├─────>│ output │
     *  └──────────┘      └──────────┘      └────────┘
     */
    case SignalChain::Signal_Envelope_Out:
      patch_cords.con4 = new AudioConnection(audio.raw_signal, audio.envelope);
      patch_cords.con5 = new AudioConnection(audio.envelope, 0, output, 1);
      break;
  }
}

void PatchStereoSignalChain(StereoAudioChain &audio_chain, const SignalChain chain_selection_left,
                            const SignalChain chain_selection_right) {
  PatchLeftStereoSignalChain(audio_chain.patch_cords, audio_chain.audio_left, audio_chain.output,
                             chain_selection_left);
  PatchRightStereoSignalChain(audio_chain.patch_cords, audio_chain.audio_right, audio_chain.output,
                              chain_selection_right);
#ifdef SENSINT_DEBUG
  debug::Log("PatchStereoSignalChain", debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
}

void ApplyGrainParameters(const GrainParameters &params, MonoAudio &audio) {
  ApplyRawSignalParameters(params.raw_signal_params, audio.raw_signal);
  ApplyEnvelopeParameters(params.envelope_params, audio.envelope);
  ApplyFilterParameters(params.filter_params, audio.filter);
#ifdef SENSINT_DEBUG
  debug::Log("ApplyGrainParameters", debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
}

int FindClosestGrainIndex(const std::vector<sensint::Grain> &sequence,
                          const sensint::analog_sensor_t sensor_val) {
  int max = sequence.size() - 1;
  if (sensor_val <= sequence.front().pos_start) {
    return 0;
  }
  if (sensor_val >= sequence.back().pos_end) {
    return max;
  }

  auto GetClosest = [&](const int a, const int b) {
    auto a_start = sequence[a].pos_start;
    auto a_end = sequence[a].pos_end;
    auto a_min = a_start;
    auto b_start = sequence[b].pos_start;
    auto b_end = sequence[b].pos_end;
    auto b_min = b_start;
    if (a_start != a_end) {
      a_min = (abs(sensor_val - a_start) <= abs(sensor_val - a_end)) ? a_start : a_end;
    }
    if (b_start != b_end) {
      b_min = (abs(sensor_val - b_start) <= abs(sensor_val - b_end)) ? b_start : b_end;
    }
    return (sensor_val - a_min >= b_min - sensor_val) ? b : a;
  };

  int left = 0, right = sequence.size(), mid = 0;
  while (left < right) {
    mid = (left + right) / 2;
    if (sequence[mid].pos_start == sensor_val || sequence[mid].pos_end == sensor_val) {
      return mid;
    }
    if (sensor_val < sequence[mid].pos_start) {
      if (mid > 0 && sensor_val > sequence[mid - 1].pos_end) {
        return GetClosest(mid - 1, mid);
      }
      right = mid;
    } else {
      if (mid < max && sensor_val < sequence[mid + 1].pos_start) {
        return GetClosest(mid, mid + 1);
      }
      left = mid + 1;
    }
  }
  return mid;
}

}  // namespace tactile_audio
}  // namespace sensint
