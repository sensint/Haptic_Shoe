// include std headers
#include <string>
#include <vector>

// include library headers
#include <Arduino.h>
#include <Wire.h>
#include <elapsedMillis.h>
#ifdef PICO
#include <FastLED.h>
#include <pixeltypes.h>
#endif  // PICO

// include shared libraries
#include <build.h>
#include <communication.h>
#include <debug.h>
#include <global_settings.h>
#include <helper.h>
#include <i2c.h>
#include <imu/bno055.h>
#include <types.h>

// include project headers
#include "ble_client.h"
#include "local_settings.h"

// !Declare variables in unnamed namespace to prevent name clashes with other files.
namespace {
using namespace sensint;

elapsedMillis last_log_update_ms;
elapsedMillis imu_time_offset_ms;
elapsedMillis fsr_time_offset_ms;

sensor::BNO055 *imu;

#ifdef PICO
CRGB led;
static constexpr uint8_t kButtonPin = 39;
bool reset_pressed = false;
#endif  // PICO

std::string serialized_input_msg;

union SerializableStruct<ImuData> imu_data_packet;

/**
 * @brief sensor  mapping - each shoe has 4 pressure sensors with the
 * following configuration:
 *   - sensor 1 [w]: great toe
 *   - sensor 2 [x]: heel (center)
 *   - sensor 3 [y]: medial forefoot
 *   - sensor 4 [z]: lateral forefoot
 *    ______________________________________
 *   |    left foot     |    right foot     |
 *   |------------------|-------------------|
 *   |      ____.--.    |    .--.____       |
 *   |    /     [w] \   |   / [w]     \     |
 *   |   /          |   |   |          \    |
 *   |  |       [y] |   |   | [y]       |   |
 *   |  | [z]       |   |   |       [z] |   |
 *   |  |         _/    |    \_         |   |
 *   |  |       /       |       \       |   |
 *   |  |      |        |        |      |   |
 *   |  |      |        |        |      |   |
 *   |  |       \       |       /       |   |
 *   |   \  [x] /       |       \ [x]  /    |
 *   |    \____/        |        \____/     |
 *   |__________________|___________________|
 *
 * Two sensors are connected to the same generator MCU:
 *   1. vertical generator: [w] and [x]
 *   2. horizontal generator: [y] and [z]
 */

// this is pressure sensor data from both generators (vertical and horizontal)
// the data will be sent to PC
union SerializableStruct<Vector4D<analog_sensor_t>> shoe_sensor_data_packet = {
  .data = {.w = 0, .x = 0, .y = 0, .z = 0 }
};
// this is the pressure sensor data from a single generator (vertical or horizontal)
// the data will be retrieved via I2C from the corresponding generator
SerializableStruct<Vector2D<analog_sensor_t>> sensor_data_vertical = {.data = {.x = 0, .y = 0}};
SerializableStruct<Vector2D<analog_sensor_t>> sensor_data_horizontal = {.data = {.x = 0, .y = 0}};

/*******************************************************************************
                              extracted functions
 ******************************************************************************/

#ifdef SENSINT_DEVELOPMENT
inline void SetupSerial() __attribute__((always_inline));
inline void PrintConfig() __attribute__((always_inline));
#endif  // SENSINT_DEVELOPMENT

#ifdef PICO
inline void SetupLED() __attribute__((always_inline));
inline void SetupButton() __attribute__((always_inline));
#endif  // PICO

inline void SetupI2C() __attribute__((always_inline));

#ifndef SENSINT_PARALLEL_DATA
inline void ReadFromI2C(const communication::Devices address, uint8_t *data, uint8_t length)
    __attribute__((always_inline));
inline void ForwardMessageUnicast(const communication::Devices device_id, const std::string &msg)
    __attribute__((always_inline));
inline void ForwardMessageBroadcast(const std::string &msg) __attribute__((always_inline));
inline void GetFSRData() __attribute__((always_inline));
inline void SendFSRData() __attribute__((always_inline));
#else
inline void SetupControlPins() __attribute__((always_inline));
inline void ConvertSequenceToBinary() __attribute__((always_inline));
#endif  // SENSINT_PARALLEL_DATA

inline void HandleMessage(const std::vector<std::string> &tokens) __attribute__((always_inline));
inline void SetupIMU() __attribute__((always_inline));
inline void GetIMUData() __attribute__((always_inline));
inline void SendIMUData() __attribute__((always_inline));
inline void HandleBLEConnection() __attribute__((always_inline));

#ifdef SENSINT_DEVELOPMENT
/**
 * @brief Setup the serial communication during the development phase.
 * This is mainly used for debugging.
 */
void SetupSerial() {
  Serial.begin(settings::global::baud_rate);
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
  Log("shoe: " + String((SENSINT_SHOE == 0) ? "left" : "right"));
  Log("i2c bus: " + String((SENSINT_WIRE == 0) ? "Wire" : "Wire1"));
#ifdef SENSINT_PARALLEL_DATA
  Log("use parallel communication for control signals");
#endif  // SENSINT_PARALLEL_DATA
  Serial.println("============================================\n\n");
}
#endif  // SENSINT_DEVELOPMENT

#ifdef PICO
/**
 * @brief The M5Stamp Pico has a on board RGB LED. This function initializes the LED for status
 * outputs.
 */
void SetupLED() {
  FastLED.addLeds<SK6812, 27, GRB>(&led, 1);
  FastLED.setBrightness(160);
  led = CRGB::Red;
  FastLED.show();
  delay(1000);
}

void SetupButton() { pinMode(kButtonPin, INPUT); }
#endif  // PICO

/**
 * @brief This function initializes the I2C communication and sets a
 * I2C-address for this controller. This function should called in the
 * setup() function or whenever a re-initialization is needed.
 */
void SetupI2C() {
#ifndef PICO
  SENSINT_I2C.begin();
#else
  SENSINT_I2C.begin(21, 22);
#endif  // PICO
}

void SetupIMU() {
  imu = new sensor::BNO055();
  auto connected = imu->Init();
#ifdef PICO
  led = connected ? settings::local::colors::kImuConnected : settings::local::colors::kError;
  FastLED.show();
#endif  // PICO
#ifdef SENSINT_DEBUG
  if (connected) {
    debug::Log("SetupIMU", "IMU connected");
  } else {
    debug::Log("SetupIMU", "No IMU");
  }
#endif  // SENSINT_DEBUG
}

#ifdef SENSINT_PARALLEL_DATA
void SetupControlPins() {
  using namespace sensint::settings;
  pinMode(local::pins::kAugmentation, OUTPUT);
  pinMode(local::pins::kSequence[0], OUTPUT);
  pinMode(local::pins::kSequence[1], OUTPUT);
  pinMode(local::pins::kSequence[2], OUTPUT);
  pinMode(local::pins::kSequence[3], OUTPUT);
  digitalWrite(local::pins::kAugmentation, (local::augmentation_active) ? HIGH : LOW);
  ConvertSequenceToBinary();
}

void ConvertSequenceToBinary() {
  using namespace sensint::settings;
  // LSB = pins::kSequence[0]
  digitalWrite(local::pins::kSequence[0], ((local::sequence & 1) == 1) ? HIGH : LOW);
  digitalWrite(local::pins::kSequence[1], ((local::sequence & 2) == 2) ? HIGH : LOW);
  digitalWrite(local::pins::kSequence[2], ((local::sequence & 4) == 4) ? HIGH : LOW);
  digitalWrite(local::pins::kSequence[3], ((local::sequence & 8) == 8) ? HIGH : LOW);
}
#endif  // SENSINT_PARALLEL_DATA

#ifndef SENSINT_PARALLEL_DATA
void ReadFromI2C(const communication::Devices address, uint8_t *data, uint8_t length) {
  SENSINT_I2C.requestFrom(static_cast<uint8_t>(address), length);
  auto read_len = SENSINT_I2C.readBytes(data, length);
#ifdef SENSINT_DEBUG
  if (debug::kDebugLevel == debug::DebugLevel::verbose) {
    Serial.printf("ReadFromI2C >>> read %d of %d bytes", (int)read_len, (int)length);
  }
#endif  // SENSINT_DEBUG
}

void ForwardMessageUnicast(const communication::Devices device_id, const std::string &msg) {
  std::string data = msg;
  data.insert(0, 1, '<');
  data.append(">");
  int limit = 32;
  int data_len = data.length();
  if (data_len < limit) {
    SENSINT_I2C.beginTransmission(static_cast<uint8_t>(device_id));
    SENSINT_I2C.write(data.c_str());
    SENSINT_I2C.endTransmission();
#ifdef SENSINT_DEBUG
    debug::Log("ForwardMessageUnicast", "send as single packet");
    if (debug::kDebugLevel == debug::DebugLevel::verbose) {
      Serial.println(data.c_str());
    }
#endif  // SENSINT_DEBUG
  } else {
#ifdef SENSINT_DEBUG
    debug::Log("ForwardMessageUnicast", "send as multiple packets");
#endif  // SENSINT_DEBUG
    for (int i = 0; i < data_len; i += limit) {
      std::string sub_data = data.substr(i, limit);
      SENSINT_I2C.beginTransmission(static_cast<uint8_t>(device_id));
      SENSINT_I2C.write(sub_data.c_str());
      SENSINT_I2C.endTransmission();
#ifdef SENSINT_DEBUG
      if (debug::kDebugLevel == debug::DebugLevel::verbose) {
        Serial.printf("packet [%d]: %s\n", i / limit, sub_data.c_str());
      }
#endif  // SENSINT_DEBUG
    }
  }
}

void ForwardMessageBroadcast(std::string &msg) {
  msg[0] = String((int)settings::local::i2c_slave_vertical).c_str()[0];
  ForwardMessageUnicast(settings::local::i2c_slave_vertical, msg);
  msg[0] = String((int)settings::local::i2c_slave_horizontal).c_str()[0];
  ForwardMessageUnicast(settings::local::i2c_slave_horizontal, msg);
}
#endif  // SENSINT_PARALLEL_DATA

void HandleMessage(const std::vector<std::string> &tokens) {
  using namespace sensint::communication;
  using namespace sensint::debug;

  auto type = static_cast<MessageTypes>(atoi(tokens[1].c_str()));

  switch (type) {
    case MessageTypes::kStartRecording:
#ifdef SENSINT_DEBUG
      Log("start recording");
#endif  // SENSINT_DEBUG
      settings::local::recording_status = sensint::RecordingStatus::kRecording;
      break;
    case MessageTypes::kStopRecording:
#ifdef SENSINT_DEBUG
      Log("stop recording");
#endif  // SENSINT_DEBUG
      settings::local::recording_status = sensint::RecordingStatus::kIdle;
      break;
    case MessageTypes::kChangeRecordingInterval:
#ifdef SENSINT_DEBUG
      Log("change recording interval");
#endif  // SENSINT_DEBUG
      if (tokens.size() == 4) {
        settings::local::log_interval_ms = static_cast<uint32_t>(atoi(tokens[3].c_str()));
      }
      break;
    case MessageTypes::kReinitializeIMU:
#ifdef SENSINT_DEBUG
      Log("reinitialize IMU");
#endif  // SENSINT_DEBUG
      settings::local::imu_reinitialize = true;
      break;
    default:
#ifdef SENSINT_DEBUG
      Log("undefined message type");
#endif  // SENSINT_DEBUG
      break;
  }
}

void GetIMUData() {
  using namespace sensint::sensor;
  using namespace sensint::settings;
  imu->UpdateData(local::imu_data_selection);
  imu->SetTimeOffset(imu_time_offset_ms);
#ifdef SENSINT_DEBUG
  sensint::debug::Log("GetIMUData", "read IMU data", debug::DebugLevel::verbose);
  if (debug::kDebugLevel == debug::DebugLevel::verbose) {
    PrintImuData(imu->GetData(), local::imu_data_selection);
  }
#endif  // SENSINT_DEBUG
}

void SendIMUData() {
  using namespace sensint::ble;
  using namespace sensint::sensor;
  using namespace sensint::settings;
  if (!client::connected) {
    return;
  }
  imu_data_packet.data = imu->GetData();
  client::imu_char->writeValue((uint8_t *)&imu_data_packet.serialized, sizeof(ImuData));
}

#ifndef SENSINT_PARALLEL_DATA
void GetFSRData() {
  ReadFromI2C(settings::local::i2c_slave_vertical, sensor_data_vertical.serialized,
              sizeof(sensor_data_vertical.serialized));
  ReadFromI2C(settings::local::i2c_slave_horizontal, sensor_data_horizontal.serialized,
              sizeof(sensor_data_horizontal.serialized));
  shoe_sensor_data_packet.data.w = sensor_data_vertical.data.x;
  shoe_sensor_data_packet.data.x = sensor_data_vertical.data.y;
  shoe_sensor_data_packet.data.y = sensor_data_horizontal.data.x;
  shoe_sensor_data_packet.data.z = sensor_data_horizontal.data.y;
#ifdef SENSINT_DEBUG
  if (debug::kDebugLevel == debug::DebugLevel::verbose) {
    PrintVector4D(shoe_sensor_data_packet.data, false);
  }
#endif  // SENSINT_DEBUG
}

void SendFSRData() {
  using namespace sensint::ble;
  if (!client::connected) {
    return;
  }
  client::sensor_char->writeValue((uint8_t *)&shoe_sensor_data_packet.serialized,
                                  sizeof(Vector4D<analog_sensor_t>));
}
#endif  // SENSINT_PARALLEL_DATA

void HandleBLEConnection() {
  using namespace sensint::ble;
  if (client::do_connect) {
    if (client::ConnectToServer()) {
      client::connected_char->writeValue((uint8_t *)&client::connected, sizeof(bool));
    } else {
#ifdef SENSINT_DEBUG
      debug::Log("HandleBLEConnection", "Could not connect to BLE server!");
#endif  // SENSINT_DEBUG
    }
    client::do_connect = false;
  }
  if (!client::connected && client::do_scan) {
#ifdef PICO
    led = settings::local::colors::kBleScan;
    FastLED.show();
#endif  // PICO
    BLEDevice::getScan()->start(0);
  }

#ifdef PICO
  if (client::connected) {
    led = settings::local::colors::kBleConnected;
    FastLED.show();
  }
#endif  // PICO
}

}  // namespace

void setup() {
  using namespace sensint::debug;

#ifdef SENSINT_DEVELOPMENT
  SetupSerial();
#ifdef SENSINT_DEBUG
  PrintConfig();
#endif  // SENSINT_DEBUG
#endif  // SENSINT_DEVELOPMENT

#ifdef PICO
  SetupLED();
  SetupButton();
#endif  // PICO

#ifdef SENSINT_PARALLEL_DATA
  SetupControlPins();
#endif  // SENSINT_PARALLEL_DATA

  SetupI2C();

  // initialize peripherals
  SetupIMU();
  sensint::ble::client::Init();

  // reset the update timers
  imu_time_offset_ms = 0;
  fsr_time_offset_ms = 0;
  last_log_update_ms = 0;
}

void loop() {
  using namespace sensint::debug;
  using namespace sensint::communication;
  using namespace sensint::settings;

#ifdef PICO
  bool btn_state = digitalRead(kButtonPin);
  if (!btn_state) {
    if (!reset_pressed) {
#ifdef SENSINT_DEBUG
      Log("reset");
#endif  // SENSINT_DEBUG
      reset_pressed = true;
      ESP.restart();
    }
  } else {
    reset_pressed = false;
  }
#endif  // PICO

#ifdef SENSINT_DEVELOPMENT
  if (GetSerializedDataFrameFromSerial(serialized_input_msg)) {
#ifdef PICO
    led = settings::local::colors::kReadSerial;
    FastLED.show();
#endif  // PICO
    std::vector<std::string> tokens;
    if (helper::SplitString(serialized_input_msg, tokens)) {
      auto destination = atoi(tokens[0].c_str());
      switch (destination) {
        case static_cast<int>(settings::local::i2c_slave_horizontal):
#ifdef SENSINT_DEBUG
          Log("message for horizontal augmentation device");
          Log(serialized_input_msg.c_str(), DebugLevel::verbose);
#endif  // SENSINT_DEBUG
#ifndef SENSINT_PARALLEL_DATA
          ForwardMessageUnicast(settings::local::i2c_slave_horizontal, serialized_input_msg);
#endif  // SENSINT_PARALLEL_DATA
          break;
        case static_cast<int>(settings::local::i2c_slave_vertical):
#ifdef SENSINT_DEBUG
          Log("message for vertical augmentation device");
          Log(serialized_input_msg.c_str(), DebugLevel::verbose);
#endif  // SENSINT_DEBUG
#ifndef SENSINT_PARALLEL_DATA
          ForwardMessageUnicast(settings::local::i2c_slave_vertical, serialized_input_msg);
#endif  // SENSINT_PARALLEL_DATA
          break;
        case static_cast<int>(Devices::kAll):
#ifdef SENSINT_DEBUG
          Log("message for all augmentation devices");
          Log(serialized_input_msg.c_str(), DebugLevel::verbose);
#endif  // SENSINT_DEBUG
#ifndef SENSINT_PARALLEL_DATA
          ForwardMessageBroadcast(serialized_input_msg);
#endif  // SENSINT_PARALLEL_DATA
          break;
        case static_cast<int>(Devices::kMST):
#ifdef SENSINT_DEBUG
          Log("message for master controller");
          Log(serialized_input_msg.c_str(), DebugLevel::verbose);
#endif  // SENSINT_DEBUG
          HandleMessage(tokens);
          break;
        default:
#ifdef SENSINT_DEBUG
          Log("undefined destination (device)");
          Log(serialized_input_msg.c_str(), DebugLevel::verbose);
#endif  // SENSINT_DEBUG
          break;
      }
    }
#ifdef PICO
    led = settings::local::colors::kIdle;
    FastLED.show();
#endif  // PICO
  }
#endif  // SENSINT_DEVELOPMENT

  if (local::imu_reinitialize) {
    SetupIMU();
    uint32_t tmp = 0;
    ble::client::imu_reinitialize_char->writeValue((uint8_t *)&tmp, sizeof(uint32_t));
    local::imu_reinitialize = false;
  }

  // regularly check for BLE connection
  HandleBLEConnection();

  // the haptic augmentation can be enabled/disabled from the PC
  // the ble-client implements a notification mechanism for this
  if (local::augmentation_active_changed) {
#ifndef SENSINT_PARALLEL_DATA
    std::string msg = (local::augmentation_active) ? "0,32,0,-" : "0,33,0,-";
    ForwardMessageBroadcast(msg);
#else
    digitalWrite(local::pins::kAugmentation, (local::augmentation_active) ? HIGH : LOW);
#endif  // SENSINT_PARALLEL_DATA
    local::augmentation_active_changed = false;
  }

  // the the can be changed from the PC
  // the ble-client implements a notification mechanism for this
  if (local::sequence_changed) {
#ifndef SENSINT_PARALLEL_DATA
    std::string msg = "0,34,1,";
    msg += String((int)local::sequence).c_str();
    ForwardMessageBroadcast(msg);
#else
    ConvertSequenceToBinary();
#endif  // SENSINT_PARALLEL_DATA
    local::sequence_changed = false;
  }

  // the frequency can be changed from the PC
  // the ble-client implements a notification mechanism for this
  if (last_log_update_ms >= local::log_interval_ms) {
    last_log_update_ms = 0;
    // the recording status can be changed from the PC
    // the ble-client implements a notification mechanism for this
    if (local::recording_status == sensint::RecordingStatus::kRecording) {
      fsr_time_offset_ms = 0;
      imu_time_offset_ms = 0;
#ifdef PICO
      led = settings::local::colors::kHandleData;
      FastLED.show();
#endif  // PICO
      GetIMUData();
      SendIMUData();
#ifndef SENSINT_PARALLEL_DATA
      GetFSRData();
      SendFSRData();
#endif  // SENSINT_PARALLEL_DATA
#ifdef PICO
      led = settings::local::colors::kIdle;
      FastLED.show();
#endif  // PICO
    }
  }
}
