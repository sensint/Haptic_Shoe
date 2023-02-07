#ifndef __SENSINT_BLE_CLIENT_H__
#define __SENSINT_BLE_CLIENT_H__

// include system headers
#include <Arduino.h>

// include library headers
#include "BLEDevice.h"

// include shared libraries
#include <ble_config.h>
#include <debug.h>
#include <global_settings.h>

// include project headers
#include "local_settings.h"

namespace sensint {
namespace ble {
namespace client {

static BLEAdvertisedDevice* device;

static BLEUUID service_UUID(config::kServiceUUID);

static BLEUUID data_selection_char_UUID(config::characteristics::kTrackingDataSelection.uuid);
static BLERemoteCharacteristic* data_selection_char;

static BLEUUID recording_status_char_UUID(config::characteristics::kRecordingStatus.uuid);
static BLERemoteCharacteristic* recording_status_char;

static BLEUUID recording_interval_char_UUID(config::characteristics::kRecordingInterval.uuid);
static BLERemoteCharacteristic* recording_interval_char;

static BLEUUID augmentation_active_UUID(config::characteristics::kAugmentationActive.uuid);
static BLERemoteCharacteristic* augmentation_active_char;

static BLEUUID imu_reinitialize_char_UUID(config::characteristics::kIMUReinitialize.uuid);
static BLERemoteCharacteristic* imu_reinitialize_char;

#if SENSINT_SHOE == 0 /* LEFT */
static BLEUUID imu_UUID(config::characteristics::kLeftShoeIMU.uuid);
static BLERemoteCharacteristic* imu_char;

static BLEUUID connected_char_UUID(config::characteristics::kLeftShoeConnected.uuid);
static BLERemoteCharacteristic* connected_char;

static BLEUUID sensor_char_UUID(config::characteristics::kLeftShoeSensorData.uuid);
static BLERemoteCharacteristic* sensor_char;

static BLEUUID sequence_char_UUID(config::characteristics::kLeftShoeSequence.uuid);
static BLERemoteCharacteristic* sequence_char;
#else   /* RIGHT */
static BLEUUID imu_UUID(config::characteristics::kLeftShoeIMU.uuid);
static BLERemoteCharacteristic* imu_char;

static BLEUUID connected_char_UUID(config::characteristics::kRightShoeConnected.uuid);
static BLERemoteCharacteristic* connected_char;

static BLEUUID sensor_char_UUID(config::characteristics::kRightShoeSensorData.uuid);
static BLERemoteCharacteristic* sensor_char;

static BLEUUID sequence_char_UUID(config::characteristics::kRightShoeSequence.uuid);
static BLERemoteCharacteristic* sequence_char;
#endif  // SENSINT_SHOE

static void NotifyRecordingStatusCallback(BLERemoteCharacteristic* remote_characteristic,
                                          uint8_t* data, size_t length, bool is_notify) {
  uint32_t tmp = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#ifdef SENSINT_DEBUG
  debug::Log("NotifyRecordingStatusCallback", String(tmp), debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  settings::local::recording_status = static_cast<RecordingStatus>(tmp);
}

static void NotifyRecordingIntervalCallback(BLERemoteCharacteristic* remote_characteristic,
                                            uint8_t* data, size_t length, bool is_notify) {
  uint32_t tmp = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#ifdef SENSINT_DEBUG
  debug::Log("NotifyRecordingIntervalCallback", String(tmp), debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  settings::local::log_interval_ms = tmp;
}

static void NotifyAugmentationActiveCallback(BLERemoteCharacteristic* remote_characteristic,
                                             uint8_t* data, size_t length, bool is_notify) {
  bool tmp = (data[0] == 1);
#ifdef SENSINT_DEBUG
  debug::Log("NotifyAugmentationActiveCallback", String(tmp), debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  settings::local::augmentation_active = tmp;
  settings::local::augmentation_active_changed = true;
}

static void NotifyDataSelectionCallback(BLERemoteCharacteristic* remote_characteristic,
                                        uint8_t* data, size_t length, bool is_notify) {
  uint8_t tmp = data[0];
#ifdef SENSINT_DEBUG
  debug::Log("NotifyDataSelectionCallback", String(tmp), debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  settings::local::imu_data_selection = tmp;
}

static void NotifySequenceCallback(BLERemoteCharacteristic* remote_characteristic, uint8_t* data,
                                   size_t length, bool is_notify) {
  uint8_t tmp = data[0];
#ifdef SENSINT_DEBUG
  debug::Log("NotifySequenceCallback", String(tmp), debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  settings::local::sequence = tmp;
  settings::local::sequence_changed = true;
}

static void NotifyIMUReinitializeCallback(BLERemoteCharacteristic* remote_characteristic,
                                          uint8_t* data, size_t length, bool is_notify) {
  uint32_t tmp = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#ifdef SENSINT_DEBUG
  debug::Log("NotifyIMUReinitializeCallback", String(tmp), debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  settings::local::imu_reinitialize = (tmp == 1);
}

static boolean do_connect = false;
static boolean connected = false;
static boolean do_scan = false;

class ClientConnectionCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* client) { client->setMTU(settings::global::defaults::kMTU); }
  void onDisconnect(BLEClient* client) { connected = false; }
};

bool ConnectToServer() {
  BLEClient* client = BLEDevice::createClient();
  client->setClientCallbacks(new ClientConnectionCallback());
  client->connect(device);
  BLERemoteService* remote_service = client->getService(service_UUID);
  if (remote_service == nullptr) {
#ifdef SENSINT_DEBUG
    sensint::debug::Log("ConnectToServer", __LINE__, "[ERR] failed to connect to service");
#endif  // SENSINT_DEBUG
    client->disconnect();
    return false;
  }

  /** connected
   * this value will be send to the server
   */
  connected_char = remote_service->getCharacteristic(connected_char_UUID);
  if (connected_char == nullptr) {
#ifdef SENSINT_DEBUG
    sensint::debug::Log("ConnectToServer", __LINE__, "[ERR] failed to connect to characteristic");
#endif  // SENSINT_DEBUG
    client->disconnect();
    return false;
  }

  /** recording status
   * Will be modified by the server. Hence we need to subscribe to it.
   */
  recording_status_char = remote_service->getCharacteristic(recording_status_char_UUID);
  if (recording_status_char == nullptr) {
#ifdef SENSINT_DEBUG
    sensint::debug::Log("ConnectToServer", __LINE__, "[ERR] failed to connect to characteristic");
#endif  // SENSINT_DEBUG
    client->disconnect();
    return false;
  }
  if (recording_status_char->canNotify()) {
    recording_status_char->registerForNotify(NotifyRecordingStatusCallback);
  }

  /** IMU data selection
   * Will be modified by the server. Hence we need to subscribe to it.
   */
  data_selection_char = remote_service->getCharacteristic(data_selection_char_UUID);
  if (data_selection_char == nullptr) {
#ifdef SENSINT_DEBUG
    sensint::debug::Log("ConnectToServer", __LINE__, "[ERR] failed to connect to characteristic");
#endif  // SENSINT_DEBUG
    client->disconnect();
    return false;
  }
  if (data_selection_char->canNotify()) {
    data_selection_char->registerForNotify(NotifyDataSelectionCallback);
  }

  /** recording interval
   * Will be modified by the server. Hence we need to subscribe to it.
   */
  recording_interval_char = remote_service->getCharacteristic(recording_interval_char_UUID);
  if (recording_interval_char == nullptr) {
#ifdef SENSINT_DEBUG
    sensint::debug::Log("ConnectToServer", __LINE__, "[ERR] failed to connect to characteristic");
#endif  // SENSINT_DEBUG
    client->disconnect();
    return false;
  }
  if (recording_interval_char->canNotify()) {
    recording_interval_char->registerForNotify(NotifyRecordingIntervalCallback);
  }

  /** augmentation active
   * Will be modified by the server. Hence we need to subscribe to it.
   */
  augmentation_active_char = remote_service->getCharacteristic(augmentation_active_UUID);
  if (augmentation_active_char == nullptr) {
#ifdef SENSINT_DEBUG
    sensint::debug::Log("ConnectToServer", __LINE__, "[ERR] failed to connect to characteristic");
#endif  // SENSINT_DEBUG
    client->disconnect();
    return false;
  }
  if (augmentation_active_char->canNotify()) {
    augmentation_active_char->registerForNotify(NotifyAugmentationActiveCallback);
  }

  /** sequence selection
   * Will be modified by the server. Hence we need to subscribe to it.
   */
  sequence_char = remote_service->getCharacteristic(sequence_char_UUID);
  if (sequence_char == nullptr) {
#ifdef SENSINT_DEBUG
    sensint::debug::Log("ConnectToServer", __LINE__, "[ERR] failed to connect to characteristic");
#endif  // SENSINT_DEBUG
    client->disconnect();
    return false;
  }
  if (sequence_char->canNotify()) {
    sequence_char->registerForNotify(NotifySequenceCallback);
  }

  /** IMU
   * Will be send to the server.
   */
  imu_char = remote_service->getCharacteristic(imu_UUID);
  if (imu_char == nullptr) {
#ifdef SENSINT_DEBUG
    sensint::debug::Log("ConnectToServer", __LINE__, "[ERR] failed to connect to characteristic");
#endif  // SENSINT_DEBUG
    client->disconnect();
    return false;
  }

  /** IMU reinitialize
   * Will be modified by the server. Hence we need to subscribe to it.
   */
  imu_reinitialize_char = remote_service->getCharacteristic(imu_reinitialize_char_UUID);
  if (imu_reinitialize_char == nullptr) {
#ifdef SENSINT_DEBUG
    sensint::debug::Log("ConnectToServer", __LINE__, "[ERR] failed to connect to characteristic");
#endif  // SENSINT_DEBUG
    client->disconnect();
    return false;
  }
  if (imu_reinitialize_char->canNotify()) {
    imu_reinitialize_char->registerForNotify(NotifyIMUReinitializeCallback);
  }

  /** sensor data
   * Will be send to the server.
   */
  sensor_char = remote_service->getCharacteristic(sensor_char_UUID);
  if (sensor_char == nullptr) {
#ifdef SENSINT_DEBUG
    sensint::debug::Log("ConnectToServer", __LINE__, "[ERR] failed to connect to characteristic");
#endif  // SENSINT_DEBUG
    client->disconnect();
    return false;
  }

  connected = true;
  return true;
}

class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertised_device) {
    if (advertised_device.haveServiceUUID() &&
        advertised_device.isAdvertisingService(service_UUID)) {
      BLEDevice::getScan()->stop();
      device = new BLEAdvertisedDevice(advertised_device);
      do_connect = true;
      do_scan = true;
    }
  }
};

void Init() {
  BLEDevice::init(settings::local::defaults::kBLEDeviceName);
  BLEDevice::setMTU(settings::global::defaults::kMTU);
  BLEScan* BLE_scan = BLEDevice::getScan();
  BLE_scan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  BLE_scan->setInterval(1349);
  BLE_scan->setWindow(449);
  BLE_scan->setActiveScan(true);
  BLE_scan->start(5, false);
}

}  // namespace client
}  // namespace ble
}  // namespace sensint

#endif  // __SENSINT_BLE_CLIENT_H__
