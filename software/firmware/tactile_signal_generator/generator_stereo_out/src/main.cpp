// include std headers
#include <array>
#include <cmath>
#include <vector>

// include library headers
#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>

// include shared libraries
#include <build.h>
#ifdef SENSINT_DEBUG
#include <debug.h>
#endif  // SENSINT_DEBUG
#include <analog_sensor.h>
#include <global_settings.h>
#include <helper.h>
#include <i2c.h>
#include <material_lib.h>
#include <sequence_lib.h>
#include <state_management.h>
#include <tactile_audio.h>
#include <types.h>

// include project headers
#include "hardware_config.h"
#include "local_settings.h"

namespace {
using namespace sensint;

/*******************************************************************************
                          (namespace) global variables
 ******************************************************************************/

std::string serialized_input_msg;
bool input_msg_start_found = false;

#ifdef SENSINT_PARALLEL_DATA
elapsedMillis control_update_timer;
#endif  // SENSINT_PARALLEL_DATA

// audio
sensint::tactile_audio::StereoAudioChain signal_chain;
//   A/a is connected to the left audio channel
//   B/b is connected to the right audio channel
//    ______________________________________
//   |    left foot     |    right foot     |
//   |------------------|-------------------|
//   |      ____.--.    |    .--.____       |
//   |    /     [A] \   |   / [A]     \     |
//   |   /          |   |   |          \    |
//   |  |       [a] |   |   | [a]       |   |
//   |  | [b]       |   |   |       [b] |   |
//   |  |         _/    |    \_         |   |
//   |  |       /       |       \       |   |
//   |  |      |        |        |      |   |
//   |  |      |        |        |      |   |
//   |  |       \       |       /       |   |
//   |   \  [B] /       |       \ [B]  /    |
//   |    \____/        |        \____/     |
//   |__________________|___________________|

// augmentation
MaterialLib material_lib;
SequenceLib sequence_lib;
AugmentationState state_a;
AugmentationState state_b;

// sensor
sensor::AnalogSensor sensor_a;
sensor::AnalogSensor sensor_b;

SerializableStruct<Vector2D<analog_sensor_t>> sensor_data = {.data = {.x = 0, .y = 0}};

/*******************************************************************************
                              extracted functions
 ******************************************************************************/

#ifdef SENSINT_DEVELOPMENT
inline void SetupSerial() __attribute__((always_inline));
inline void PrintConfig() __attribute__((always_inline));
#endif  // SENSINT_DEVELOPMENT

#ifndef SENSINT_PARALLEL_DATA
inline void SetupI2C() __attribute__((always_inline));
#else
inline void SetupControlPins() __attribute__((always_inline));
#endif  // SENSINT_PARALLEL_DATA

inline void SetupSensors() __attribute__((always_inline));
inline void SetupAudio() __attribute__((always_inline));
inline void SetupAugmentation() __attribute__((always_inline));
inline void LoadPresets() __attribute__((always_inline));
inline void HandleAugmentation() __attribute__((always_inline));
inline void UpdateConfig() __attribute__((always_inline));

#ifdef SENSINT_DEVELOPMENT
/**
 * @brief Setup the serial communication during the development phase.
 * This is mainly used for debugging.
 */
void SetupSerial() {
  Serial.begin(sensint::settings::global::baud_rate);
  while (!Serial && millis() < 5000) {
    ;
  }
}

void PrintConfig() {
  using namespace sensint::debug;
  Serial.println("\n\n============================================");
  Log(String(FW_NAME));
  Serial.println("--------------------------------------------");
  Log("version: " + String(GIT_TAG));
  Log("commit: " + String(GIT_REV));
  Log("build mode: " + String((SENSINT_BUILD_MODE == 0) ? "development" : "release"));
  Log("debug level: " + String(SENSINT_DEBUG));
  Serial.println("--------------------------------------------");
  Log("orientation: " + String((SENSINT_ORIENTATION == 0) ? "vertical [A,B]" : "horizontal [a,b]"));
  Log("sensor: " + String((SENSINT_SENSOR == 0) ? "FSR" : "slider, potentiometer"));
  Log("i2c bus: " + String((SENSINT_WIRE == 0) ? "Wire" : "Wire1"));
  Log("i2c addr: " + String((int)settings::local::i2c_address));
#ifdef SENSINT_PARALLEL_DATA
  Log("use parallel communication for control signals");
#endif  // SENSINT_PARALLEL_DATA
  Serial.println("============================================\n\n");
}
#endif  // SENSINT_DEVELOPMENT

#ifndef SENSINT_PARALLEL_DATA
/**
 * @brief This is a callback function which is called every time the main
 * controller sends new data to the peripheral controller (this).
 *
 * @param number_of_bytes The number of bytes available for reading.
 */
void HandleI2COnReceive(int number_of_bytes) {
  if (communication::GetSerializedDataFrameFromI2C(serialized_input_msg, input_msg_start_found)) {
    state_a.should_update_config = true;
    input_msg_start_found = false;
#ifdef SENSINT_DEBUG
    debug::Log("HandleI2COnReceive", serialized_input_msg.c_str(), debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  }
}

/**
 * @brief This is a callback function which is called as soon as the main
 * controller asks the peripheral controller (this) for new data (e.g. sensor
 * data).
 */
void HandleI2COnRequest() {
  if (state_a.should_send_sensor_data) {
    return;
  }
  state_a.should_send_sensor_data = true;
  sensor_data.data.x = sensor_a.GetLastFilteredValue();
  sensor_data.data.y = sensor_b.GetLastFilteredValue();
  size_t data_len = sizeof(sensor_data.serialized);
  auto write_len = SENSINT_I2C.write(sensor_data.serialized, data_len);
#ifdef SENSINT_DEBUG
  debug::Log("HandleI2COnRequest", "send sensor data", debug::DebugLevel::verbose);
  if (debug::kDebugLevel == debug::DebugLevel::verbose) {
    Serial.printf("bytes written: %d/%d\n", write_len, data_len);
    PrintVector2D<analog_sensor_t>(sensor_data.data, false);
  }
#endif  // SENSINT_DEBUG
  state_a.should_send_sensor_data = false;
}

/**
 * @brief This function initializes the I2C communication and sets a I2C-address
 * for this controller. This function should called in the setup() function or
 * whenever a re-initialization is needed.
 */
void SetupI2C() {
  SENSINT_I2C.begin(static_cast<uint8_t>(sensint::settings::local::i2c_address));
  SENSINT_I2C.onReceive(HandleI2COnReceive);
  SENSINT_I2C.onRequest(HandleI2COnRequest);
}
#endif  // SENSINT_PARALLEL_DATA

#ifdef SENSINT_PARALLEL_DATA
void SetupControlPins() {
  using namespace sensint::settings;
  pinMode(local::pins::kAugmentation, INPUT);
  pinMode(local::pins::kSequence[0], INPUT);
  pinMode(local::pins::kSequence[1], INPUT);
  pinMode(local::pins::kSequence[2], INPUT);
  pinMode(local::pins::kSequence[3], INPUT);
}
#endif  // SENSINT_PARALLEL_DATA

/**
 * @brief This function initializes the sensors (i.e. analog sensors) connected
 * to this controller. This function should be called once in the setup()
 * function.
 */
void SetupSensors() {
  using namespace sensint;
  using namespace sensint::sensor;

#if SENSINT_SENSOR == 0  // FSR
  sensor_a = AnalogSensor(config::kAnalogSensingPinA, SetupType::kInputPullUp,
                          AnalogSensor::kDefaultOutputResolution, 0, 1000);
  sensor_b = AnalogSensor(config::kAnalogSensingPinB, SetupType::kInputPullUp,
                          AnalogSensor::kDefaultOutputResolution, 0, 1000);
  state_a.allow_retrigger = false;
  state_b.allow_retrigger = false;
  sensor_a.SetJitterThreshold(5);
  sensor_b.SetJitterThreshold(5);
#ifdef SENSINT_DEBUG
  debug::Log("SetupSensor", "FSR");
#endif  // SENSINT_DEBUG
#else   // slider, potentiometer
  sensor_a = AnalogSensor(config::kAnalogSensingPinA, SetupType::kInput,
                          AnalogSensor::kDefaultOutputResolution);
  sensor_b = AnalogSensor(config::kAnalogSensingPinB, SetupType::kInput,
                          AnalogSensor::kDefaultOutputResolution);
  sensor_a.SetJitterThreshold(4);
  sensor_b.SetJitterThreshold(4);
#ifdef SENSINT_DEBUG
  debug::Log("SetupSensor", "Slider, Potentiometer");
#endif  // SENSINT_DEBUG
#endif  // SENSINT_SENSOR

  sensor_a.Initialize();
  sensor_b.Initialize();
}

/**
 * @brief This function initializes the audio objects as well as the signal
 * chain. This should be called only once in the setup function.
 */
void SetupAudio() {
  using namespace sensint::tactile_audio;
  AudioMemory(20);
  delay(50);
  PatchStereoSignalChain(signal_chain, SignalChain::Signal_Envelope_Out,
                         SignalChain::Signal_Envelope_Out);
}

/**
 * @brief This function is used to initialize the audio, signal chain, as well
 * as the default grain sequence and list of materials. This function should be
 * only called once in the setup function.
 */
void SetupAugmentation() {
  // set up the left channel (A/a)
  state_a.current_material = &material_lib.GetDefaultMaterial();
  tactile_audio::ApplyGrainParameters(state_a.current_material->grain_params,
                                      signal_chain.audio_left);
  state_a.current_sequence = &sequence_lib.GetDefaultSequence();
  state_a.closest_grain = &state_a.current_sequence->grains.front();
  state_a.last_grain = state_a.closest_grain;
  // set up the right channel (B/b)
  state_b.current_material = &material_lib.GetDefaultMaterial();
  tactile_audio::ApplyGrainParameters(state_b.current_material->grain_params,
                                      signal_chain.audio_right);
  state_b.current_sequence = &sequence_lib.GetDefaultSequence();
  state_b.closest_grain = &state_a.current_sequence->grains.front();
  state_b.last_grain = state_b.closest_grain;
}

void LoadPresets() {
  for (const auto& material : settings::local::presets::kMaterials) {
    serialized_input_msg = material;
    UpdateConfig();
  }
  for (const auto& sequence : settings::local::presets::kSequences) {
    serialized_input_msg = sequence;
    UpdateConfig();
  }
}

void HandleAugmentation() {
  using namespace sensint;
  using namespace sensint::debug;

  auto grain_idx_a = tactile_audio::FindClosestGrainIndex(state_a.current_sequence->grains,
                                                          state_a.current_sensor_value);
  auto grain_idx_b = tactile_audio::FindClosestGrainIndex(state_b.current_sequence->grains,
                                                          state_b.current_sensor_value);

  state_management::CheckAndStopContinuousVibration(grain_idx_a, state_a, signal_chain.audio_left);
  state_management::CheckAndStopContinuousVibration(grain_idx_b, state_b, signal_chain.audio_right);

  state_management::CheckAndStopGrain(grain_idx_a, state_a, signal_chain.audio_left);
  state_management::CheckAndStopGrain(grain_idx_b, state_b, signal_chain.audio_right);

  state_a.closest_grain = &state_a.current_sequence->grains[grain_idx_a];
  state_b.closest_grain = &state_b.current_sequence->grains[grain_idx_b];

  if (state_a.closest_grain->pos_start != state_a.closest_grain->pos_end) {
    state_management::CheckAndStartContinuousVibration(grain_idx_a, state_a,
                                                       signal_chain.audio_left, material_lib);
  } else {
    state_management::CheckAndStartGrain(grain_idx_a, state_a, signal_chain.audio_left,
                                         material_lib);
  }

  if (state_b.closest_grain->pos_start != state_b.closest_grain->pos_end) {
    state_management::CheckAndStartContinuousVibration(grain_idx_b, state_b,
                                                       signal_chain.audio_right, material_lib);
  } else {
    state_management::CheckAndStartGrain(grain_idx_b, state_b, signal_chain.audio_right,
                                         material_lib);
  }
}

void UpdateConfig() {
  using namespace sensint;
  using namespace sensint::communication;
  using namespace sensint::debug;

  std::vector<std::string> tokens;
  if (!helper::SplitString(serialized_input_msg, tokens)) {
    state_a.should_update_config = false;
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Tokenizing the string failed!");
#endif  // SENSINT_DEBUG
    return;
  }

  auto destination = static_cast<uint8_t>(atoi(tokens[0].c_str()));
  if (destination != static_cast<uint8_t>(settings::local::i2c_address) &&
      destination != static_cast<uint8_t>(Devices::kAll)) {
    state_a.should_update_config = false;
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Received message for different destination device!");
#endif  // SENSINT_DEBUG
    return;
  }

  auto msg_type = static_cast<MessageTypes>(atoi(tokens[1].c_str()));
  state_management::UpdateConfig(msg_type, tokens, state_a, material_lib, sequence_lib);
  state_a.should_update_config = false;
}

}  // namespace

void setup() {
  using namespace sensint;

#ifndef SENSINT_PARALLEL_DATA
  SetupI2C();
#else
  SetupControlPins();
  control_update_timer = 0;
#endif  // SENSINT_PARALLEL_DATA

#ifdef SENSINT_DEVELOPMENT
  SetupSerial();
#ifdef SENSINT_DEBUG
  PrintConfig();
#endif  // SENSINT_DEBUG
#endif  // SENSINT_DEVELOPMENT

  SetupSensors();
  SetupAudio();
  SetupAugmentation();
  LoadPresets();
}

void loop() {
#ifdef SENSINT_DEVELOPMENT
  using namespace sensint::communication;
  //! For testing only - comment if you want to use I2C communication!
  // if (GetSerializedDataFrameFromSerial(serialized_input_msg)) {
  //   state_a.should_update_config = true;
  // }
#endif  // SENSINT_DEVELOPMENT

#if SENSINT_SENSOR == 0  // FSR
  state_a.current_sensor_value = sensor_a.ReadFiltered(0.015);
  state_b.current_sensor_value = sensor_b.ReadFiltered(0.015);
#else   // slider, potentiometer
  state_a.current_sensor_value = sensor_a.ReadFiltered();
  state_b.current_sensor_value = sensor_b.ReadFiltered();
#endif  // SENSINT_SENSOR

  if (state_a.should_augment) {
    HandleAugmentation();
  }

#ifndef SENSINT_PARALLEL_DATA
  if (state_a.should_update_config) {
    UpdateConfig();
  }
#else
  if (control_update_timer > 1000) {
    state_a.should_augment = digitalRead(sensint::settings::local::pins::kAugmentation) == 1;
    // compose sequence id as 4bit value and set digital pins accordingly
    // LSB = pins::kSequence[0]
    uint8_t seq_id = digitalRead(sensint::settings::local::pins::kSequence[0]);
    seq_id |= digitalRead(sensint::settings::local::pins::kSequence[1]) << 1;
    seq_id |= digitalRead(sensint::settings::local::pins::kSequence[2]) << 2;
    seq_id |= digitalRead(sensint::settings::local::pins::kSequence[3]) << 3;

    if (seq_id != state_a.current_sequence->id && sequence_lib.SequenceExists(seq_id)) {
      sequence_lib.GetSequenceByID(seq_id, *state_a.current_sequence);
      state_a.should_reinitialize_material = true;
#ifdef SENSINT_DEBUG
      debug::Log("======== Active Sequence ========");
      PrintGrainSequence(*state_a.current_sequence);
#endif  // SENSINT_DEBUG
    }
    control_update_timer = 0;
  }
#endif  // SENSINT_PARALLEL_DATA
}
