#include <Arduino.h>
#include <elapsedMillis.h>

#include <sstream>
#include <string>

// include shared libraries
#include <build.h>
#include <communication.h>
#include <debug.h>
#include <global_settings.h>
#include <helper.h>
#include <imu/imu.h>
#include <types.h>

// include project headers
#include "ble_server.h"
#include "local_settings.h"

// !Declare variables in unnamed namespace to prevent name clashes with other files.
namespace {
using namespace sensint;

std::string serialized_input_msg;

// timers
elapsedMillis recording_update_time;
// ble device
ble::BleServer ble_server;
uint8_t ble_connected_devices = 0;

/*******************************************************************************
                                 mock data
 ******************************************************************************/
Vector4D<analog_sensor_t> empty_fsr{.w = 0, .x = 0, .y = 0, .z = 0};
ImuData empty_imu{.orientation_quaternion = {.w = 0.0, .x = 0.0, .y = 0.0, .z = 0.0},
                  .acceleration_linear{.x = 0.0, .y = 0.0, .z = 0.0},
                  .calibration = 0,
                  .time_offset = 0};

/*******************************************************************************
                                left shoe group
 ******************************************************************************/
bool left_shoe_connected = false;
uint32_t left_shoe_timestamp = 0;
union SerializableStruct<ImuData> left_shoe_imu;
union SerializableStruct<Vector4D<analog_sensor_t>> left_shoe_sensor_data = {
  .data = {.w = 0, .x = 0, .y = 0, .z = 0 }
};

/*******************************************************************************
                                right shoe group
 ******************************************************************************/
bool right_shoe_connected = false;
uint32_t right_shoe_timestamp = 0;
union SerializableStruct<ImuData> right_shoe_imu;
union SerializableStruct<Vector4D<analog_sensor_t>> right_shoe_sensor_data = {
  .data = {.w = 0, .x = 0, .y = 0, .z = 0 }
};

/*******************************************************************************
                                 tracking group
 ******************************************************************************/
bool tracking_connected = false;
uint8_t tracking_num_imu = 0;
uint8_t tracking_connected_imu = 0b00000000;
union SerializableStruct<ImuData> tracking_imu_1;
union SerializableStruct<ImuData> tracking_imu_2;
union SerializableStruct<ImuData> tracking_imu_3;
union SerializableStruct<ImuData> tracking_imu_4;
union SerializableStruct<ImuData> tracking_imu_5;
union SerializableStruct<ImuData> tracking_imu_6;
union SerializableStruct<ImuData> tracking_imu_7;
union SerializableStruct<ImuData> tracking_imu_8;

/*******************************************************************************
                              extracted functions
 ******************************************************************************/

inline void SetupSerial() __attribute__((always_inline));
inline void SetupBLECallbacks() __attribute__((always_inline));
inline void SetupBLEServer() __attribute__((always_inline));
inline void SetupBLEData() __attribute__((always_inline));
inline void ResetAllTimer() __attribute__((always_inline));
#ifdef SENSINT_DEBUG
inline void LogIMUData() __attribute__((always_inline));
inline void PrintConfig() __attribute__((always_inline));
#endif  // SENSINT_DEBUG
inline void HandleTrackingUpdate() __attribute__((always_inline));
inline void HandleLeftShoeUpdate() __attribute__((always_inline));
inline void HandleRightShoeUpdate() __attribute__((always_inline));
inline void HandleMessageFromSerial() __attribute__((always_inline));
inline void SendTrackingDataToPC() __attribute__((always_inline));
inline void SendShoeDataToPC() __attribute__((always_inline));

void SetupSerial() {
  Serial.begin(settings::global::baud_rate);
  while (!Serial) {
    ;
  }
}

void SetupBLECallbacks() {
  using namespace sensint::settings;
  /*******************************************************************************
                              status and config group
   ******************************************************************************/
  ble_server.recording_status_cb_.value_ = &local::recording_status;
  ble_server.recording_interval_cb_.value_ = &local::recording_interval;
  ble_server.augmentation_active_cb_.value_ = &local::augmentation_active;
  ble_server.imu_reinitialize_cb_.value_ = &local::imu_reinitialize;

  /*******************************************************************************
                                  left shoe group
   ******************************************************************************/
  ble_server.left_shoe_connected_cb_.value_ = &left_shoe_connected;
  ble_server.left_shoe_timestamp_cb_.value_ = &left_shoe_timestamp;
  ble_server.left_shoe_sensors_cb_.value_ = (uint8_t*)&left_shoe_sensor_data;
  ble_server.left_shoe_imu_cb_.value_ = (uint8_t*)&left_shoe_imu;
  ble_server.left_shoe_sequence_cb_.value_ = &local::left_shoe_sequence;

  /*******************************************************************************
                                  right shoe group
   ******************************************************************************/
  ble_server.right_shoe_connected_cb_.value_ = &right_shoe_connected;
  ble_server.right_shoe_timestamp_cb_.value_ = &right_shoe_timestamp;
  ble_server.right_shoe_sensors_cb_.value_ = (uint8_t*)&right_shoe_sensor_data;
  ble_server.right_shoe_imu_cb_.value_ = (uint8_t*)&right_shoe_imu;
  ble_server.right_shoe_sequence_cb_.value_ = &local::right_shoe_sequence;

  /*******************************************************************************
                                   tracking group
   ******************************************************************************/
  ble_server.tracking_connected_cb_.value_ = &tracking_connected;
  ble_server.tracking_num_imu_cb_.value_ = &tracking_num_imu;
  ble_server.tracking_connected_imu_cb_.value_ = &tracking_connected_imu;
  ble_server.tracking_data_selection_cb_.value_ = &local::tracking_data_selection;
  ble_server.tracking_imu_1_cb_.value_ = (uint8_t*)&tracking_imu_1;
  ble_server.tracking_imu_2_cb_.value_ = (uint8_t*)&tracking_imu_2;
  ble_server.tracking_imu_3_cb_.value_ = (uint8_t*)&tracking_imu_3;
  ble_server.tracking_imu_4_cb_.value_ = (uint8_t*)&tracking_imu_4;
  ble_server.tracking_imu_5_cb_.value_ = (uint8_t*)&tracking_imu_5;
  ble_server.tracking_imu_6_cb_.value_ = (uint8_t*)&tracking_imu_6;
  ble_server.tracking_imu_7_cb_.value_ = (uint8_t*)&tracking_imu_7;
  ble_server.tracking_imu_8_cb_.value_ = (uint8_t*)&tracking_imu_8;
}

void SetupBLEServer() {
  ble_server.Init();
  ble_server.Advertize();
}

void SetupBLEData() {
  using namespace sensint::settings;
  /*******************************************************************************
                              status and config group
   ******************************************************************************/
  ble_server.recording_status_char_->setValue(local::recording_status);

  ble_server.recording_interval_char_->setValue(local::recording_interval);

  auto tmp_augmentation_active = local::augmentation_active ? 1 : 0;
  ble_server.augmentation_active_char_->setValue(tmp_augmentation_active);

  auto tmp_imu_reinitialize = local::imu_reinitialize ? 1 : 0;
  ble_server.imu_reinitialize_char_->setValue(tmp_imu_reinitialize);

  /*******************************************************************************
                                  left shoe group
   ******************************************************************************/
  auto tmp_left_shoe_connected = left_shoe_connected ? 1 : 0;
  ble_server.left_shoe_connected_char_->setValue(tmp_left_shoe_connected);

  ble_server.left_shoe_timestamp_char_->setValue(left_shoe_timestamp);

  ble_server.left_shoe_sensors_char_->setValue(left_shoe_sensor_data.serialized,
                                               ble_server.left_shoe_sensors_cb_.size_);

  ble_server.left_shoe_imu_char_->setValue(left_shoe_imu.serialized,
                                           ble_server.left_shoe_imu_cb_.size_);

  int tmp_left_shoe_sequence = local::left_shoe_sequence;
  ble_server.left_shoe_sequence_char_->setValue(tmp_left_shoe_sequence);

  /*******************************************************************************
                                  right shoe group
   ******************************************************************************/
  auto tmp_right_shoe_connected = right_shoe_connected ? 1 : 0;
  ble_server.right_shoe_connected_char_->setValue(tmp_right_shoe_connected);

  ble_server.right_shoe_timestamp_char_->setValue(right_shoe_timestamp);

  ble_server.right_shoe_sensors_char_->setValue(right_shoe_sensor_data.serialized,
                                                ble_server.right_shoe_sensors_cb_.size_);

  ble_server.right_shoe_imu_char_->setValue(right_shoe_imu.serialized,
                                            ble_server.right_shoe_imu_cb_.size_);

  int tmp_right_shoe_sequence = local::right_shoe_sequence;
  ble_server.right_shoe_sequence_char_->setValue(tmp_right_shoe_sequence);

  /*******************************************************************************
                                   tracking group
   ******************************************************************************/
  auto tmp_tracking_connected = tracking_connected ? 1 : 0;
  ble_server.tracking_connected_char_->setValue(tmp_tracking_connected);

  int tmp_tracking_num_imu = tracking_num_imu;
  ble_server.tracking_num_imu_char_->setValue(tmp_tracking_num_imu);

  int tmp_tracking_connected_imu = tracking_connected_imu;
  ble_server.tracking_connected_imu_char_->setValue(tmp_tracking_connected_imu);

  int tmp_tracking_data_selection = local::tracking_data_selection;
  ble_server.tracking_data_selection_char_->setValue(tmp_tracking_data_selection);

  ble_server.tracking_imu_1_char_->setValue(tracking_imu_1.serialized,
                                            ble_server.tracking_imu_1_cb_.size_);
  ble_server.tracking_imu_2_char_->setValue(tracking_imu_2.serialized,
                                            ble_server.tracking_imu_2_cb_.size_);
  ble_server.tracking_imu_3_char_->setValue(tracking_imu_3.serialized,
                                            ble_server.tracking_imu_3_cb_.size_);
  ble_server.tracking_imu_4_char_->setValue(tracking_imu_4.serialized,
                                            ble_server.tracking_imu_4_cb_.size_);
  ble_server.tracking_imu_5_char_->setValue(tracking_imu_5.serialized,
                                            ble_server.tracking_imu_5_cb_.size_);
  ble_server.tracking_imu_6_char_->setValue(tracking_imu_6.serialized,
                                            ble_server.tracking_imu_6_cb_.size_);
  ble_server.tracking_imu_7_char_->setValue(tracking_imu_7.serialized,
                                            ble_server.tracking_imu_7_cb_.size_);
  ble_server.tracking_imu_8_char_->setValue(tracking_imu_8.serialized,
                                            ble_server.tracking_imu_8_cb_.size_);
}

void ResetAllTimer() { recording_update_time = 0; }

#ifdef SENSINT_DEBUG
void LogImuData(const ImuData& imu_data, const uint8_t imu_id) {
  using namespace sensint::debug;
  Log("LogImuData", "data from tracking IMU : " + String(imu_id));
  if (kDebugLevel == DebugLevel::verbose) {
    PrintImuData(imu_data, settings::local::tracking_data_selection);
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
  Serial.println("============================================\n\n");
}
#endif  // SENSINT_DEBUG

void HandleTrackingUpdate() {
  using namespace sensint::debug;
  using namespace sensint::sensor;
  using namespace sensint::settings;
  tracking_connected = *ble_server.tracking_connected_cb_.value_;
#ifdef SENSINT_DEBUG
  Log("HandleTrackingUpdate", "tracking connected: " + String(tracking_connected ? "yes" : "no"),
      DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  if (!tracking_connected) {
    return;
  }
  // tracking_num_imu = *ble_server.tracking_num_imu_cb_.value_;
  tracking_num_imu = ble_server.tracking_num_imu_char_->getData()[0];
  // tracking_connected_imu = *ble_server.tracking_connected_imu_cb_.value_;
  tracking_connected_imu = ble_server.tracking_connected_imu_char_->getData()[0];
  // tracking_data_selection = *ble_server.tracking_data_selection_cb_.value_;
  local::tracking_data_selection = ble_server.tracking_data_selection_char_->getData()[0];
#ifdef SENSINT_DEBUG
  Log("HandleTrackingUpdate", "tracking # of IMUs: " + String((int)tracking_num_imu),
      DebugLevel::verbose);
  Log("HandleTrackingUpdate", "tracking connected IMUs: " + String((int)tracking_connected_imu),
      DebugLevel::verbose);
  Log("HandleTrackingUpdate",
      "tracking data selection: " + String((int)local::tracking_data_selection),
      DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  if (local::recording_status != static_cast<uint32_t>(RecordingStatus::kRecording)) {
    return;
  }
  for (uint8_t id = 0; id < global::defaults::kTrackingMaxNumberOfIMUs; id++) {
    if (tracking_connected_imu & (1 << id)) {
      switch (id) {
        case 0:
          memcpy(ble_server.tracking_imu_1_cb_.value_, tracking_imu_1.serialized,
                 ble_server.tracking_imu_1_cb_.size_);
#ifdef SENSINT_DEBUG
          LogImuData(tracking_imu_1.data, id);
#endif  // SENSINT_DEBUG
          break;
        case 1:
          memcpy(ble_server.tracking_imu_2_cb_.value_, tracking_imu_2.serialized,
                 ble_server.tracking_imu_2_cb_.size_);
#ifdef SENSINT_DEBUG
          LogImuData(tracking_imu_2.data, id);
#endif  // SENSINT_DEBUG
          break;
        case 2:
          memcpy(ble_server.tracking_imu_3_cb_.value_, tracking_imu_3.serialized,
                 ble_server.tracking_imu_3_cb_.size_);
#ifdef SENSINT_DEBUG
          LogImuData(tracking_imu_3.data, id);
#endif  // SENSINT_DEBUG
          break;
        case 3:
          memcpy(ble_server.tracking_imu_4_cb_.value_, tracking_imu_4.serialized,
                 ble_server.tracking_imu_4_cb_.size_);
#ifdef SENSINT_DEBUG
          LogImuData(tracking_imu_4.data, id);
#endif  // SENSINT_DEBUG
          break;
        case 4:
          memcpy(ble_server.tracking_imu_5_cb_.value_, tracking_imu_5.serialized,
                 ble_server.tracking_imu_5_cb_.size_);
#ifdef SENSINT_DEBUG
          LogImuData(tracking_imu_5.data, id);
#endif  // SENSINT_DEBUG
          break;
        case 5:
          memcpy(ble_server.tracking_imu_6_cb_.value_, tracking_imu_6.serialized,
                 ble_server.tracking_imu_6_cb_.size_);
#ifdef SENSINT_DEBUG
          LogImuData(tracking_imu_6.data, id);
#endif  // SENSINT_DEBUG
          break;
        case 6:
          memcpy(ble_server.tracking_imu_7_cb_.value_, tracking_imu_7.serialized,
                 ble_server.tracking_imu_7_cb_.size_);
#ifdef SENSINT_DEBUG
          LogImuData(tracking_imu_7.data, id);
#endif  // SENSINT_DEBUG
          break;
        case 7:
          memcpy(ble_server.tracking_imu_8_cb_.value_, tracking_imu_8.serialized,
                 ble_server.tracking_imu_8_cb_.size_);
#ifdef SENSINT_DEBUG
          LogImuData(tracking_imu_8.data, id);
#endif  // SENSINT_DEBUG
          break;
        default:
          break;
      }
    }
  }
}

void HandleLeftShoeUpdate() {
  using namespace sensint::debug;
  using namespace sensint::sensor;
  using namespace sensint::settings;
  left_shoe_connected = *ble_server.left_shoe_connected_cb_.value_;
#ifdef SENSINT_DEBUG
  Log("HandleLeftShoeUpdate", "left shoe connected: " + String(left_shoe_connected ? "yes" : "no"),
      DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  if (!left_shoe_connected) {
    return;
  }
  if (local::recording_status != static_cast<uint32_t>(RecordingStatus::kRecording)) {
    return;
  }
  memcpy(ble_server.left_shoe_imu_cb_.value_, left_shoe_imu.serialized,
         ble_server.left_shoe_imu_cb_.size_);
  memcpy(ble_server.left_shoe_sensors_cb_.value_, left_shoe_sensor_data.serialized,
         ble_server.left_shoe_sensors_cb_.size_);
#ifdef SENSINT_DEBUG
  Log("HandleLeftShoeUpdate", "IMU data from left shoe");
  if (kDebugLevel == DebugLevel::verbose) {
    PrintImuData(left_shoe_imu.data, local::tracking_data_selection);
  }
  Log("HandleLeftShoeUpdate", "sensor data from left shoe");
  if (kDebugLevel == DebugLevel::verbose) {
    PrintVector4D(left_shoe_sensor_data.data);
  }
#endif  // SENSINT_DEBUG
}

void HandleRightShoeUpdate() {
  using namespace sensint::debug;
  using namespace sensint::sensor;
  using namespace sensint::settings;
  right_shoe_connected = *ble_server.right_shoe_connected_cb_.value_;
#ifdef SENSINT_DEBUG
  Log("HandleRightShoeUpdate",
      "right shoe connected: " + String(right_shoe_connected ? "yes" : "no"), DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  if (!right_shoe_connected) {
    return;
  }
  if (local::recording_status != static_cast<uint32_t>(RecordingStatus::kRecording)) {
    return;
  }
  memcpy(ble_server.right_shoe_imu_cb_.value_, right_shoe_imu.serialized,
         ble_server.right_shoe_imu_cb_.size_);
  memcpy(ble_server.right_shoe_sensors_cb_.value_, right_shoe_sensor_data.serialized,
         ble_server.right_shoe_sensors_cb_.size_);
#ifdef SENSINT_DEBUG
  Log("HandleRightShoeUpdate", "IMU data from right shoe");
  if (kDebugLevel == DebugLevel::verbose) {
    PrintImuData(right_shoe_imu.data, local::tracking_data_selection);
  }
  Log("HandleRightShoeUpdate", "sensor data from right shoe");
  if (kDebugLevel == DebugLevel::verbose) {
    PrintVector4D(right_shoe_sensor_data.data);
  }
#endif  // SENSINT_DEBUG
}

void HandleRemoteSettingsUpdate(bool force_update = false) {
  using namespace sensint::debug;
  using namespace sensint::settings;

  if (force_update ||
      local::recording_status != local::recording_status_old && local::recording_status_changed) {
    ble_server.recording_status_char_->setValue(local::recording_status);
    ble_server.recording_status_char_->notify();
    local::recording_status_changed = false;
    local::recording_status_old = local::recording_status;
#ifdef SENSINT_DEBUG
    Log("HandleRemoteSettingsUpdate", "recording status: " + String(local::recording_status));
#endif  // SENSINT_DEBUG
  }

  if (force_update || local::recording_interval != local::recording_interval_old &&
                          local::recording_interval_changed) {
    ble_server.recording_interval_char_->setValue(local::recording_interval);
    ble_server.recording_interval_char_->notify();
    local::recording_interval_changed = false;
    local::recording_interval_old = local::recording_interval;
#ifdef SENSINT_DEBUG
    Log("HandleRemoteSettingsUpdate", "recording interval: " + String(local::recording_interval));
#endif  // SENSINT_DEBUG
  }

  if (force_update || local::augmentation_active != local::augmentation_active_old &&
                          local::augmentation_active_changed) {
    auto tmp_augmentation_active = local::augmentation_active ? 1 : 0;
    ble_server.augmentation_active_char_->setValue(tmp_augmentation_active);
    ble_server.augmentation_active_char_->notify();
    local::augmentation_active_changed = false;
    local::augmentation_active_old = local::augmentation_active;
#ifdef SENSINT_DEBUG
    Log("HandleRemoteSettingsUpdate", "augmentation active: " + String(local::augmentation_active));
#endif  // SENSINT_DEBUG
  }

  if (force_update ||
      local::imu_reinitialize != local::imu_reinitialize_old && local::imu_reinitialize_changed) {
    auto tmp_imu_reinitialize = local::imu_reinitialize ? 1 : 0;
    ble_server.imu_reinitialize_char_->setValue(tmp_imu_reinitialize);
    ble_server.imu_reinitialize_char_->notify();
    local::imu_reinitialize_changed = false;
    local::imu_reinitialize_old = local::imu_reinitialize;
#ifdef SENSINT_DEBUG
    Log("HandleRemoteSettingsUpdate", "IMU reinitialize: " + String(local::imu_reinitialize));
#endif  // SENSINT_DEBUG
  }

  if (force_update || local::tracking_data_selection_changed &&
                          local::tracking_data_selection != local::tracking_data_selection_old) {
    int tmp_tracking_data_selection = local::tracking_data_selection;
    ble_server.tracking_data_selection_char_->setValue(tmp_tracking_data_selection);
    ble_server.tracking_data_selection_char_->notify();
    local::tracking_data_selection_changed = false;
    local::tracking_data_selection_old = local::tracking_data_selection;
#ifdef SENSINT_DEBUG
    Log("HandleRemoteSettingsUpdate",
        "tracking data selection: " + String((int)local::tracking_data_selection));
#endif  // SENSINT_DEBUG
  }

  if (force_update || local::left_shoe_sequence_changed &&
                          local::left_shoe_sequence != local::left_shoe_sequence_old) {
    int tmp_left_shoe_sequence = (int)local::left_shoe_sequence;
    ble_server.left_shoe_sequence_char_->setValue(tmp_left_shoe_sequence);
    ble_server.left_shoe_sequence_char_->notify();
    local::left_shoe_sequence_changed = false;
    local::left_shoe_sequence_old = local::left_shoe_sequence;
#ifdef SENSINT_DEBUG
    Log("HandleRemoteSettingsUpdate",
        "left shoe sequence: " + String((int)local::left_shoe_sequence));
#endif  // SENSINT_DEBUG
  }

  if (force_update || local::right_shoe_sequence_changed &&
                          local::right_shoe_sequence != local::right_shoe_sequence_old) {
    int tmp_right_shoe_sequence = (int)local::right_shoe_sequence;
    ble_server.right_shoe_sequence_char_->setValue(tmp_right_shoe_sequence);
    ble_server.right_shoe_sequence_char_->notify();
    local::right_shoe_sequence_changed = false;
    local::right_shoe_sequence_old = local::right_shoe_sequence;
#ifdef SENSINT_DEBUG
    Log("HandleRemoteSettingsUpdate",
        "right shoe sequence: " + String((int)local::right_shoe_sequence));
#endif  // SENSINT_DEBUG
  }

  // reset the global flag
  local::settings_changed = false;
}

void HandleMessageFromSerial() {
  using namespace sensint::communication;
  using namespace sensint::debug;

  if (!GetSerializedDataFrameFromSerial(serialized_input_msg)) {
    return;
  }
  std::vector<std::string> tokens;
  if (!helper::SplitString(serialized_input_msg, tokens)) {
    return;
  }

  auto type = static_cast<MessageTypes>(atoi(tokens[1].c_str()));

  switch (type) {
    case MessageTypes::kStartAugmentation:
#ifdef SENSINT_DEBUG
      Log("HandleMessageFromSerial", "should start augmentation");
#endif  // SENSINT_DEBUG
      // TODO: toggle augmentation per shoe
      settings::local::augmentation_active = true;
      settings::local::augmentation_active_changed = true;
      break;

    case MessageTypes::kStopAugmentation:
#ifdef SENSINT_DEBUG
      Log("HandleMessageFromSerial", "should stop augmentation");
#endif  // SENSINT_DEBUG
      settings::local::augmentation_active = false;
      settings::local::augmentation_active_changed = true;
      break;

    case MessageTypes::kStartRecording:
#ifdef SENSINT_DEBUG
      Log("HandleMessageFromSerial", "should start recording");
#endif  // SENSINT_DEBUG
      settings::local::recording_status =
          static_cast<uint32_t>(sensint::RecordingStatus::kRecording);
      settings::local::recording_status_changed = true;
      break;

    case MessageTypes::kStopRecording:
#ifdef SENSINT_DEBUG
      Log("HandleMessageFromSerial", "should stop recording");
#endif  // SENSINT_DEBUG
      settings::local::recording_status = static_cast<uint32_t>(sensint::RecordingStatus::kIdle);
      settings::local::recording_status_changed = true;
      break;

    case MessageTypes::kChangeRecordingInterval:
#ifdef SENSINT_DEBUG
      Log("HandleMessageFromSerial", "should change recording interval");
#endif  // SENSINT_DEBUG
      if (tokens.size() == 4) {
        settings::local::recording_interval = static_cast<uint32_t>(atoi(tokens[3].c_str()));
        settings::local::recording_interval_changed = true;
      }
      break;

    case MessageTypes::kReinitializeIMU:
#ifdef SENSINT_DEBUG
      Log("HandleMessageFromSerial", "should reinitialize IMU");
#endif  // SENSINT_DEBUG
      settings::local::imu_reinitialize = true;
      settings::local::imu_reinitialize_changed = true;
      break;

    case MessageTypes::kSelectGrainSequence: {
#ifdef SENSINT_DEBUG
      Log("HandleMessageFromSerial", "should change sequence");
#endif  // SENSINT_DEBUG
      if (tokens.size() != 4) {
        return;
      }
      auto destination = atoi(tokens[0].c_str());
      switch (destination) {
        case static_cast<int>(Devices::kAll):
          settings::local::left_shoe_sequence = static_cast<uint32_t>(atoi(tokens[3].c_str()));
          settings::local::left_shoe_sequence_changed = true;
          settings::local::right_shoe_sequence = static_cast<uint32_t>(atoi(tokens[3].c_str()));
          settings::local::right_shoe_sequence_changed = true;
          break;
        case static_cast<int>(Devices::kLeftShoe):
          settings::local::left_shoe_sequence = static_cast<uint32_t>(atoi(tokens[3].c_str()));
          settings::local::left_shoe_sequence_changed = true;
          break;
        case static_cast<int>(Devices::kRightShoe):
          settings::local::right_shoe_sequence = static_cast<uint32_t>(atoi(tokens[3].c_str()));
          settings::local::right_shoe_sequence_changed = true;
          break;
        default:
#ifdef SENSINT_DEBUG
          Log("HandleMessageFromSerial", "destination device not supported");
#endif  // SENSINT_DEBUG
          break;
      }
      break;
    }

    default:
#ifdef SENSINT_DEBUG
      Log("undefined message type");
#endif  // SENSINT_DEBUG
      break;
  }
}

void SendTrackingDataToPC() {
#ifdef SENSINT_DEBUG
  debug::Log("SendTrackingDataToPC", "IMU data", debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  std::string msg;
  for (uint8_t id = 0; id < settings::global::defaults::kTrackingMaxNumberOfIMUs; id++) {
    if (tracking_connected_imu & (1 << id)) {
      switch (id) {
        case 0:
          communication::SerializeImuData(tracking_imu_1.data, msg, true);
          msg += communication::kMessageDelimiter;
          break;
        case 1:
          communication::SerializeImuData(tracking_imu_2.data, msg, true);
          msg += communication::kMessageDelimiter;
          break;
        case 2:
          communication::SerializeImuData(tracking_imu_3.data, msg, true);
          msg += communication::kMessageDelimiter;
          break;
        case 3:
          communication::SerializeImuData(tracking_imu_4.data, msg, true);
          msg += communication::kMessageDelimiter;
          break;
        case 4:
          communication::SerializeImuData(tracking_imu_5.data, msg, true);
          msg += communication::kMessageDelimiter;
          break;
        case 5:
          communication::SerializeImuData(tracking_imu_6.data, msg, true);
          msg += communication::kMessageDelimiter;
          break;
        case 6:
          communication::SerializeImuData(tracking_imu_7.data, msg, true);
          msg += communication::kMessageDelimiter;
          break;
        case 7:
          communication::SerializeImuData(tracking_imu_7.data, msg, true);
          msg += communication::kMessageDelimiter;
          break;
        default:
          break;
      }
    }
    if (msg.empty()) {
      return;
    }
    msg.erase(msg.end());
    Serial.println(msg.c_str());
  }

  //! This is the binary version.
  // for (uint8_t id = 0; id < settings::global::defaults::kTrackingMaxNumberOfIMUs; id++) {
  //   if (tracking_connected_imu & (1 << id)) {
  //     switch (id) {
  //       case 0:
  //         Serial.write(tracking_imu_1.serialized, sizeof(tracking_imu_1.serialized));
  //         break;
  //       case 1:
  //         Serial.write(tracking_imu_2.serialized, sizeof(tracking_imu_2.serialized));
  //         break;
  //       case 2:
  //         Serial.write(tracking_imu_3.serialized, sizeof(tracking_imu_3.serialized));
  //         break;
  //       case 3:
  //         Serial.write(tracking_imu_4.serialized, sizeof(tracking_imu_4.serialized));
  //         break;
  //       case 4:
  //         Serial.write(tracking_imu_5.serialized, sizeof(tracking_imu_5.serialized));
  //         break;
  //       case 5:
  //         Serial.write(tracking_imu_6.serialized, sizeof(tracking_imu_6.serialized));
  //         break;
  //       case 6:
  //         Serial.write(tracking_imu_7.serialized, sizeof(tracking_imu_7.serialized));
  //         break;
  //       case 7:
  //         Serial.write(tracking_imu_8.serialized, sizeof(tracking_imu_8.serialized));
  //         break;
  //       default:
  //         break;
  //     }
  //   }
  // }
}

/**
 * @brief send data of both shoes to the serial port
 * order of data: left shoe [4x fsr,imu], right shoe [4x fsr,imu]
 * @example
 * <1,68,1,0,0,0,0,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0,0,0,0,0,0,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0,0>
 *
 */
void SendShoeDataToPC() {
#ifdef SENSINT_DEBUG
  debug::Log("SendShoeDataToPC", "order of data: left shoe [fsr,imu], right shoe [fsr,imu]",
             debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  //"<1,68,1,"
  std::stringstream header;
  header << "<" << String((int)communication::Devices::kGUI).c_str()
         << communication::kMessageDelimiter
         << String((int)communication::MessageTypes::kShoeData).c_str()
         << communication::kMessageDelimiter << "1" << communication::kMessageDelimiter;
  std::string msg{header.str()};
  if (left_shoe_connected) {
    communication::SerializeVector4D<analog_sensor_t>(left_shoe_sensor_data.data, msg, true, false);
    msg += communication::kMessageDelimiter;
    communication::SerializeImuData(left_shoe_imu.data, msg, true);
  } else {
    communication::SerializeVector4D<analog_sensor_t>(empty_fsr, msg, true, false);
    msg += communication::kMessageDelimiter;
    communication::SerializeImuData(empty_imu, msg, true);
  }
  msg += communication::kMessageDelimiter;
  if (right_shoe_connected) {
    communication::SerializeVector4D<analog_sensor_t>(right_shoe_sensor_data.data, msg, true,
                                                      false);
    msg += communication::kMessageDelimiter;
    communication::SerializeImuData(left_shoe_imu.data, msg, true);
  } else {
    communication::SerializeVector4D<analog_sensor_t>(empty_fsr, msg, true, false);
    msg += communication::kMessageDelimiter;
    communication::SerializeImuData(empty_imu, msg, true);
  }
  msg.append(">");
  Serial.println(msg.c_str());

  //! This is the binary version.
  // if (left_shoe_connected) {
  //   Serial.write(left_shoe_sensor_data.serialized, sizeof(left_shoe_sensor_data.serialized));
  //   Serial.write(left_shoe_imu.serialized, sizeof(left_shoe_imu.serialized));
  // }
  // if (right_shoe_connected) {
  //   Serial.write(right_shoe_sensor_data.serialized, sizeof(right_shoe_sensor_data.serialized));
  //   Serial.write(right_shoe_imu.serialized, sizeof(right_shoe_imu.serialized));
  // }
}

}  // namespace

void setup() {
  SetupSerial();
#ifdef SENSINT_DEBUG
  PrintConfig();
#endif  // SENSINT_DEBUG
  SetupBLECallbacks();
  SetupBLEServer();
  SetupBLEData();
  ResetAllTimer();
}

void loop() {
  using namespace sensint;
  using namespace sensint::settings;

  HandleMessageFromSerial();

  if (ble_connected_devices != ble_server.connection_cb_.connected_devices_) {
    ble_connected_devices = ble_server.connection_cb_.connected_devices_;
    // broadcast current settings to all connected devices
    HandleRemoteSettingsUpdate(true);
#ifdef SENSINT_DEBUG
    debug::Log("loop", "connected devices: " + String(ble_connected_devices));
#endif  // SENSINT_DEBUG
  }

  // if (local::settings_changed && ble_connected_devices > 0) {
  if (ble_connected_devices > 0) {
    HandleRemoteSettingsUpdate();
  }

  if (recording_update_time >= local::recording_interval) {
    recording_update_time = 0;
    if (local::recording_status == static_cast<uint32_t>(RecordingStatus::kRecording) &&
        ble_connected_devices > 0) {
      // HandleTrackingUpdate();
      HandleLeftShoeUpdate();
      HandleRightShoeUpdate();
      // SendTrackingDataToPC();
      SendShoeDataToPC();
    }
  }
}
