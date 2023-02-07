#ifndef __SENSINT_SETTINGS_H__
#define __SENSINT_SETTINGS_H__

#include <communication.h>
#include <types.h>

#include <array>

#ifdef PICO
#include <pixeltypes.h>
#endif  // PICO

namespace sensint {
namespace settings {
namespace local {
namespace defaults {
#if SENSINT_SHOE == 0
static const std::string kBLEDeviceName = "senSInt Shoe left";
#else
static const std::string kBLEDeviceName = "senSInt Shoe right";
#endif  // SENSINT_SHOE

static constexpr uint32_t kLogIntervalMs = 2000;

static constexpr bool kAugmentationActive = true;

static constexpr uint8_t kIMUDataSelection = 0b11111111;
static constexpr bool kIMUReinitialize = false;

static constexpr uint8_t kSequence = 0;

}  // namespace defaults

static uint32_t log_interval_ms = defaults::kLogIntervalMs;

static RecordingStatus recording_status = RecordingStatus::kIdle;

static bool augmentation_active = defaults::kAugmentationActive;
static bool augmentation_active_changed = false;

static uint8_t imu_data_selection = defaults::kIMUDataSelection;
static bool imu_reinitialize = defaults::kIMUReinitialize;

// I2C addresses for the signal generators (slaves)
static const communication::Devices i2c_slave_vertical = communication::Devices::kPD1;
static const communication::Devices i2c_slave_horizontal = communication::Devices::kPD2;

static uint8_t sequence = defaults::kSequence;
static bool sequence_changed = false;

#ifdef SENSINT_PARALLEL_DATA
namespace pins {
static constexpr std::array<uint8_t, 4> kSequence{26, 18, 19, 25};
static constexpr uint8_t kAugmentation = 32;
}  // namespace pins
#endif  // SENSINT_PARALLEL_DATA

#ifdef PICO
namespace colors {
static const CRGB kIdle = CRGB::DarkOliveGreen;
static const CRGB kReadSerial = CRGB::Cyan;
static const CRGB kImuConnected = CRGB::DarkViolet;
static const CRGB kError = CRGB::Red;
static const CRGB kBleScan = CRGB::Blue;
static const CRGB kBleConnected = CRGB::Green;
static const CRGB kHandleData = CRGB::Yellow;
}  // namespace colors
#endif  // PICO

}  // namespace local
}  // namespace settings
}  // namespace sensint

#endif  // __SENSINT_SETTINGS_H__
