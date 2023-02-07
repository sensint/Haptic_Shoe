#ifndef __SENSINT_SETTINGS_H__
#define __SENSINT_SETTINGS_H__

/**
 * @brief This file provides the settings for the senSInt project.
 * These are the parameters that could change during the operation of the system
 * - either in your own code or via the serial interface. An example of such a
 * parameter is implemented in the function
 * @ref UpdateSettingsFromSerialInput.
 *
 * If you want to set global parameters (configurations) that should not change
 * (constants), add them to the file config.h.
 */

namespace sensint {
namespace settings {
namespace local {
namespace defaults {
static const std::string kDeviceName = "senSInt central";
static constexpr uint32_t kRecordingDelayMs = 2000;
static constexpr RecordingStatus kRecordingStatus = RecordingStatus::kIdle;
static constexpr bool kAugmentationActive = true;
static const uint8_t kSequenceID = 0;
static constexpr bool kIMUReinitialize = false;
static constexpr uint8_t kTrackingDataSelection = 0b11111111;
}  // namespace defaults

static uint32_t recording_status = static_cast<uint32_t>(defaults::kRecordingStatus);
static bool recording_status_changed = false;
//! This is a dirty hack to enable setting this value from a remote connected device (e.g.
//! smartphone) via BLE.
static uint32_t recording_status_old = recording_status;

static uint32_t recording_interval = defaults::kRecordingDelayMs;
static bool recording_interval_changed = false;
//! This is a dirty hack to enable setting this value from a remote connected device (e.g.
//! smartphone) via BLE.
static uint32_t recording_interval_old = recording_interval;

static bool augmentation_active = defaults::kAugmentationActive;
static bool augmentation_active_changed = false;
//! This is a dirty hack to enable setting this value from a remote connected device (e.g.
//! smartphone) via BLE.
static bool augmentation_active_old = augmentation_active;

static uint8_t left_shoe_sequence = defaults::kSequenceID;
static bool left_shoe_sequence_changed = false;
//! This is a dirty hack to enable setting this value from a remote connected device (e.g.
//! smartphone) via BLE.
static uint8_t left_shoe_sequence_old = left_shoe_sequence;

static uint8_t right_shoe_sequence = defaults::kSequenceID;
static bool right_shoe_sequence_changed = false;
//! This is a dirty hack to enable setting this value from a remote connected device (e.g.
//! smartphone) via BLE.
static uint8_t right_shoe_sequence_old = right_shoe_sequence;

static uint8_t tracking_data_selection = defaults::kTrackingDataSelection;
static bool tracking_data_selection_changed = false;
//! This is a dirty hack to enable setting this value from a remote connected device (e.g.
//! smartphone) via BLE.
static uint8_t tracking_data_selection_old = tracking_data_selection;

static bool imu_reinitialize = defaults::kIMUReinitialize;
static bool imu_reinitialize_changed = false;
//! This is a dirty hack to enable setting this value from a remote connected device (e.g.
//! smartphone) via BLE.
static bool imu_reinitialize_old = imu_reinitialize;

static bool settings_changed = false;

}  // namespace local
}  // namespace settings
}  // namespace sensint

#endif  // __SENSINT_SETTINGS_H__
