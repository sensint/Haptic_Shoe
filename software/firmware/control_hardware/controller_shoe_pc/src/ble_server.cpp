#include "ble_server.h"

#include <global_settings.h>
#include <imu/imu.h>
#include <types.h>

#include "ble_config.h"
#include "local_settings.h"

namespace sensint {
namespace ble {

const std::string BleServer::kDeviceName = sensint::settings::local::defaults::kDeviceName;
const std::string BleServer::kServiceUuid = config::kServiceUUID;

BleServer::BleServer() {}

BleServer::~BleServer() {}

/**
 * Create ble service and characteristics. Use `advertize()` to make service
 * visible to other devices
 * */
void BleServer::Init() {
  BLEDevice::init(kDeviceName);
  BLEDevice::setMTU(sensint::settings::global::defaults::kMTU);
  server_ = BLEDevice::createServer();
  server_->setCallbacks(&connection_cb_);
  service_ = server_->createService(kServiceUuid, 1 + kNumCharacteristics * 5);

  {
    using namespace sensint::settings;
    using namespace sensint::ble::config;

    /*******************************************************************************
                                  status and config group
     ******************************************************************************/

    /* --------------------- recording status --------------------- */
    // create characteristic
    recording_status_char_ = service_->createCharacteristic(
        characteristics::kRecordingStatus.uuid, characteristics::kRecordingStatus.access_property);
    SetDecriptorsInBLECharacteristic(
        recording_status_char_, (BleCharacteristicDescription*)&characteristics::kRecordingStatus);
    // set callbacks
    recording_status_char_->setCallbacks(&recording_status_cb_);
    // assign value
    if (recording_status_cb_.value_) {
      recording_status_char_->setValue((uint8_t*)recording_status_cb_.value_, sizeof(uint32_t));
    }

    /* --------------------- recording interval --------------------- */
    // create characteristic
    recording_interval_char_ =
        service_->createCharacteristic(characteristics::kRecordingInterval.uuid,
                                       characteristics::kRecordingInterval.access_property);
    SetDecriptorsInBLECharacteristic(
        recording_interval_char_,
        (BleCharacteristicDescription*)&characteristics::kRecordingInterval);
    // set callbacks
    recording_interval_char_->setCallbacks(&recording_interval_cb_);
    // assign value
    if (recording_interval_cb_.value_) {
      recording_interval_char_->setValue((uint8_t*)recording_interval_cb_.value_, sizeof(uint32_t));
    }

    /* --------------------- augmentation active --------------------- */
    // create characteristic
    augmentation_active_char_ =
        service_->createCharacteristic(characteristics::kAugmentationActive.uuid,
                                       characteristics::kAugmentationActive.access_property);
    SetDecriptorsInBLECharacteristic(
        augmentation_active_char_,
        (BleCharacteristicDescription*)&characteristics::kAugmentationActive);
    // set callbacks
    augmentation_active_char_->setCallbacks(&augmentation_active_cb_);
    // assign value
    if (augmentation_active_cb_.value_) {
      augmentation_active_char_->setValue((uint8_t*)augmentation_active_cb_.value_, sizeof(bool));
    }

    /* --------------------- IMU reinitialize --------------------- */
    // create characteristic
    imu_reinitialize_char_ = service_->createCharacteristic(
        characteristics::kIMUReinitialize.uuid, characteristics::kIMUReinitialize.access_property);
    SetDecriptorsInBLECharacteristic(
        imu_reinitialize_char_, (BleCharacteristicDescription*)&characteristics::kIMUReinitialize);
    // set callbacks
    imu_reinitialize_char_->setCallbacks(&imu_reinitialize_cb_);
    // assign value
    if (imu_reinitialize_cb_.value_) {
      imu_reinitialize_char_->setValue((uint8_t*)imu_reinitialize_cb_.value_, sizeof(bool));
    }

    /*******************************************************************************
                                    left shoe group
     ******************************************************************************/

    /* --------------------- left shoe connected --------------------- */
    // create characteristic
    left_shoe_connected_char_ =
        service_->createCharacteristic(characteristics::kLeftShoeConnected.uuid,
                                       characteristics::kLeftShoeConnected.access_property);
    SetDecriptorsInBLECharacteristic(
        left_shoe_connected_char_,
        (BleCharacteristicDescription*)&characteristics::kLeftShoeConnected);
    // set callbacks
    left_shoe_connected_char_->setCallbacks(&left_shoe_connected_cb_);
    // assign value
    if (left_shoe_connected_cb_.value_) {
      left_shoe_connected_char_->setValue((uint8_t*)left_shoe_connected_cb_.value_, sizeof(bool));
    }

    /* --------------------- left shoe timestamp --------------------- */
    // create characteristic
    left_shoe_timestamp_char_ =
        service_->createCharacteristic(characteristics::kLeftShoeTimestamp.uuid,
                                       characteristics::kLeftShoeTimestamp.access_property);
    SetDecriptorsInBLECharacteristic(
        left_shoe_timestamp_char_,
        (BleCharacteristicDescription*)&characteristics::kLeftShoeTimestamp);
    // set callbacks
    left_shoe_timestamp_char_->setCallbacks(&left_shoe_timestamp_cb_);
    // assign value
    if (left_shoe_timestamp_cb_.value_) {
      left_shoe_timestamp_char_->setValue((uint8_t*)left_shoe_timestamp_cb_.value_,
                                          sizeof(uint32_t));
    }

    /* --------------------- left shoe sensor data --------------------- */
    // create characteristic
    left_shoe_sensors_char_ =
        service_->createCharacteristic(characteristics::kLeftShoeSensorData.uuid,
                                       characteristics::kLeftShoeSensorData.access_property);
    SetDecriptorsInBLECharacteristic(
        left_shoe_sensors_char_,
        (BleCharacteristicDescription*)&characteristics::kLeftShoeSensorData);
    // set callbacks
    left_shoe_sensors_char_->setCallbacks(&left_shoe_sensors_cb_);
    // assign value
    left_shoe_sensors_cb_.size_ = sizeof(Vector4D<analog_sensor_t>);
    if (left_shoe_sensors_cb_.value_) {
      left_shoe_sensors_char_->setValue((uint8_t*)left_shoe_sensors_cb_.value_,
                                        sizeof(left_shoe_sensors_cb_.size_));
    }

    /* --------------------- left shoe IMU data --------------------- */
    // create characteristic
    left_shoe_imu_char_ = service_->createCharacteristic(
        characteristics::kLeftShoeIMU.uuid, characteristics::kLeftShoeIMU.access_property);
    SetDecriptorsInBLECharacteristic(left_shoe_imu_char_,
                                     (BleCharacteristicDescription*)&characteristics::kLeftShoeIMU);
    // set callbacks
    left_shoe_imu_char_->setCallbacks(&left_shoe_imu_cb_);
    // assign value
    left_shoe_imu_cb_.size_ = sizeof(ImuData);
    if (left_shoe_imu_cb_.value_) {
      left_shoe_imu_char_->setValue((uint8_t*)left_shoe_imu_cb_.value_, left_shoe_imu_cb_.size_);
    }

    /* --------------------- left shoe sequence --------------------- */
    // create characteristic
    left_shoe_sequence_char_ =
        service_->createCharacteristic(characteristics::kLeftShoeSequence.uuid,
                                       characteristics::kLeftShoeSequence.access_property);
    SetDecriptorsInBLECharacteristic(
        left_shoe_sequence_char_,
        (BleCharacteristicDescription*)&characteristics::kLeftShoeSequence);
    // set callbacks
    left_shoe_sequence_char_->setCallbacks(&left_shoe_sequence_cb_);
    // assign value
    if (left_shoe_sequence_cb_.value_) {
      left_shoe_sequence_char_->setValue((uint8_t*)left_shoe_sequence_cb_.value_, sizeof(uint8_t));
    }

    /*******************************************************************************
                                    right shoe group
     ******************************************************************************/

    /* --------------------- right shoe connected --------------------- */
    // create characteristic
    right_shoe_connected_char_ =
        service_->createCharacteristic(characteristics::kRightShoeConnected.uuid,
                                       characteristics::kRightShoeConnected.access_property);
    SetDecriptorsInBLECharacteristic(
        right_shoe_connected_char_,
        (BleCharacteristicDescription*)&characteristics::kRightShoeConnected);
    // set callbacks
    right_shoe_connected_char_->setCallbacks(&right_shoe_connected_cb_);
    // assign value
    if (right_shoe_connected_cb_.value_) {
      right_shoe_connected_char_->setValue((uint8_t*)right_shoe_connected_cb_.value_, sizeof(bool));
    }

    /* --------------------- right shoe timestamp --------------------- */
    // create characteristic
    right_shoe_timestamp_char_ =
        service_->createCharacteristic(characteristics::kRightShoeTimestamp.uuid,
                                       characteristics::kRightShoeTimestamp.access_property);
    SetDecriptorsInBLECharacteristic(
        right_shoe_timestamp_char_,
        (BleCharacteristicDescription*)&characteristics::kRightShoeTimestamp);
    // set callbacks
    right_shoe_timestamp_char_->setCallbacks(&right_shoe_timestamp_cb_);
    // assign value
    if (right_shoe_timestamp_cb_.value_) {
      right_shoe_timestamp_char_->setValue((uint8_t*)right_shoe_timestamp_cb_.value_,
                                           sizeof(uint32_t));
    }

    /* --------------------- right shoe sensor data --------------------- */
    // create characteristic
    right_shoe_sensors_char_ =
        service_->createCharacteristic(characteristics::kRightShoeSensorData.uuid,
                                       characteristics::kRightShoeSensorData.access_property);
    SetDecriptorsInBLECharacteristic(
        right_shoe_sensors_char_,
        (BleCharacteristicDescription*)&characteristics::kRightShoeSensorData);
    // set callbacks
    right_shoe_sensors_char_->setCallbacks(&right_shoe_sensors_cb_);
    // assign value
    right_shoe_sensors_cb_.size_ = sizeof(Vector4D<analog_sensor_t>);
    if (right_shoe_sensors_cb_.value_) {
      right_shoe_sensors_char_->setValue((uint8_t*)right_shoe_sensors_cb_.value_,
                                         sizeof(right_shoe_sensors_cb_.size_));
    }

    /* --------------------- right shoe IMU data --------------------- */
    // create characteristic
    right_shoe_imu_char_ = service_->createCharacteristic(
        characteristics::kRightShoeIMU.uuid, characteristics::kRightShoeIMU.access_property);
    SetDecriptorsInBLECharacteristic(
        right_shoe_imu_char_, (BleCharacteristicDescription*)&characteristics::kRightShoeIMU);
    // set callbacks
    right_shoe_imu_char_->setCallbacks(&right_shoe_imu_cb_);
    // assign value
    right_shoe_imu_cb_.size_ = sizeof(ImuData);
    if (right_shoe_imu_cb_.value_) {
      right_shoe_imu_char_->setValue((uint8_t*)right_shoe_imu_cb_.value_, right_shoe_imu_cb_.size_);
    }

    /* --------------------- right shoe sequence --------------------- */
    // create characteristic
    right_shoe_sequence_char_ =
        service_->createCharacteristic(characteristics::kRightShoeSequence.uuid,
                                       characteristics::kRightShoeSequence.access_property);
    SetDecriptorsInBLECharacteristic(
        right_shoe_sequence_char_,
        (BleCharacteristicDescription*)&characteristics::kRightShoeSequence);
    // set callbacks
    right_shoe_sequence_char_->setCallbacks(&right_shoe_sequence_cb_);
    // assign value
    if (right_shoe_sequence_cb_.value_) {
      right_shoe_sequence_char_->setValue((uint8_t*)right_shoe_sequence_cb_.value_,
                                          sizeof(uint8_t));
    }

    /*******************************************************************************
                                     tracking group
     ******************************************************************************/

    /* --------------------- tracking connected --------------------- */
    // create characteristic
    tracking_connected_char_ =
        service_->createCharacteristic(characteristics::kTrackingConnected.uuid,
                                       characteristics::kTrackingConnected.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_connected_char_,
        (BleCharacteristicDescription*)&characteristics::kTrackingConnected);
    // set callbacks
    tracking_connected_char_->setCallbacks(&tracking_connected_cb_);
    // assign value
    if (tracking_connected_cb_.value_) {
      tracking_connected_char_->setValue((uint8_t*)tracking_connected_cb_.value_, sizeof(bool));
    }

    /* --------------------- tracking number of IMUs --------------------- */
    // create characteristic
    tracking_num_imu_char_ =
        service_->createCharacteristic(characteristics::kTrackingNumberOfIMUs.uuid,
                                       characteristics::kTrackingNumberOfIMUs.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_num_imu_char_,
        (BleCharacteristicDescription*)&characteristics::kTrackingNumberOfIMUs);
    // set callbacks
    tracking_num_imu_char_->setCallbacks(&tracking_num_imu_cb_);
    // assign value
    if (tracking_num_imu_cb_.value_) {
      tracking_num_imu_char_->setValue((uint8_t*)tracking_num_imu_cb_.value_, sizeof(uint8_t));
    }

    /* --------------------- tracking connected IMU --------------------- */
    // create characteristic
    tracking_connected_imu_char_ =
        service_->createCharacteristic(characteristics::kTrackingConnectedIMUs.uuid,
                                       characteristics::kTrackingConnectedIMUs.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_connected_imu_char_,
        (BleCharacteristicDescription*)&characteristics::kTrackingConnectedIMUs);
    // set callbacks
    tracking_connected_imu_char_->setCallbacks(&tracking_connected_imu_cb_);
    // assign value
    if (tracking_connected_imu_cb_.value_) {
      tracking_connected_imu_char_->setValue((uint8_t*)tracking_connected_imu_cb_.value_,
                                             sizeof(uint8_t));
    }

    /* --------------------- tracking data selection --------------------- */
    // create characteristic
    tracking_data_selection_char_ =
        service_->createCharacteristic(characteristics::kTrackingDataSelection.uuid,
                                       characteristics::kTrackingDataSelection.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_data_selection_char_,
        (BleCharacteristicDescription*)&characteristics::kTrackingDataSelection);
    // set callbacks
    tracking_data_selection_char_->setCallbacks(&tracking_data_selection_cb_);
    // assign value
    if (tracking_data_selection_cb_.value_) {
      tracking_data_selection_char_->setValue((uint8_t*)tracking_data_selection_cb_.value_,
                                              sizeof(uint8_t));
    }

    /* --------------------- tracking IMU 1 --------------------- */
    // create characteristic
    tracking_imu_1_char_ = service_->createCharacteristic(
        characteristics::kTrackingIMU1.uuid, characteristics::kTrackingIMU1.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_imu_1_char_, (BleCharacteristicDescription*)&characteristics::kTrackingIMU1);
    // set callbacks
    tracking_imu_1_char_->setCallbacks(&tracking_imu_1_cb_);
    // assign value
    tracking_imu_1_cb_.size_ = sizeof(ImuData);
    if (tracking_imu_1_cb_.value_) {
      tracking_imu_1_char_->setValue((uint8_t*)tracking_imu_1_cb_.value_, tracking_imu_1_cb_.size_);
    }

    /* --------------------- tracking IMU 2 --------------------- */
    // create characteristic
    tracking_imu_2_char_ = service_->createCharacteristic(
        characteristics::kTrackingIMU2.uuid, characteristics::kTrackingIMU2.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_imu_2_char_, (BleCharacteristicDescription*)&characteristics::kTrackingIMU2);
    // set callbacks
    tracking_imu_2_char_->setCallbacks(&tracking_imu_2_cb_);
    // assign value
    tracking_imu_2_cb_.size_ = sizeof(ImuData);
    if (tracking_imu_2_cb_.value_) {
      tracking_imu_2_char_->setValue((uint8_t*)tracking_imu_2_cb_.value_, tracking_imu_2_cb_.size_);
    }

    /* --------------------- tracking IMU 3 --------------------- */
    // create characteristic
    tracking_imu_3_char_ = service_->createCharacteristic(
        characteristics::kTrackingIMU3.uuid, characteristics::kTrackingIMU3.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_imu_3_char_, (BleCharacteristicDescription*)&characteristics::kTrackingIMU3);
    // set callbacks
    tracking_imu_3_char_->setCallbacks(&tracking_imu_3_cb_);
    // assign value
    tracking_imu_3_cb_.size_ = sizeof(ImuData);
    if (tracking_imu_3_cb_.value_) {
      tracking_imu_3_char_->setValue((uint8_t*)tracking_imu_3_cb_.value_, tracking_imu_3_cb_.size_);
    }

    /* --------------------- tracking IMU 4 --------------------- */
    // create characteristic
    tracking_imu_4_char_ = service_->createCharacteristic(
        characteristics::kTrackingIMU4.uuid, characteristics::kTrackingIMU4.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_imu_4_char_, (BleCharacteristicDescription*)&characteristics::kTrackingIMU4);
    // set callbacks
    tracking_imu_4_char_->setCallbacks(&tracking_imu_4_cb_);
    // assign value
    tracking_imu_4_cb_.size_ = sizeof(ImuData);
    if (tracking_imu_4_cb_.value_) {
      tracking_imu_4_char_->setValue((uint8_t*)tracking_imu_4_cb_.value_, tracking_imu_4_cb_.size_);
    }

    /* --------------------- tracking IMU 5 --------------------- */
    // create characteristic
    tracking_imu_5_char_ = service_->createCharacteristic(
        characteristics::kTrackingIMU5.uuid, characteristics::kTrackingIMU5.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_imu_5_char_, (BleCharacteristicDescription*)&characteristics::kTrackingIMU5);
    // set callbacks
    tracking_imu_5_char_->setCallbacks(&tracking_imu_5_cb_);
    // assign value
    tracking_imu_5_cb_.size_ = sizeof(ImuData);
    if (tracking_imu_5_cb_.value_) {
      tracking_imu_5_char_->setValue((uint8_t*)tracking_imu_5_cb_.value_, tracking_imu_5_cb_.size_);
    }

    /* --------------------- tracking IMU 6 --------------------- */
    // create characteristic
    tracking_imu_6_char_ = service_->createCharacteristic(
        characteristics::kTrackingIMU6.uuid, characteristics::kTrackingIMU6.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_imu_6_char_, (BleCharacteristicDescription*)&characteristics::kTrackingIMU6);
    // set callbacks
    tracking_imu_6_char_->setCallbacks(&tracking_imu_6_cb_);
    // assign value
    tracking_imu_6_cb_.size_ = sizeof(ImuData);
    if (tracking_imu_6_cb_.value_) {
      tracking_imu_6_char_->setValue((uint8_t*)tracking_imu_6_cb_.value_, tracking_imu_6_cb_.size_);
    }

    /* --------------------- tracking IMU 7 --------------------- */
    // create characteristic
    tracking_imu_7_char_ = service_->createCharacteristic(
        characteristics::kTrackingIMU7.uuid, characteristics::kTrackingIMU7.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_imu_7_char_, (BleCharacteristicDescription*)&characteristics::kTrackingIMU7);
    // set callbacks
    tracking_imu_7_char_->setCallbacks(&tracking_imu_7_cb_);
    // assign value
    tracking_imu_7_cb_.size_ = sizeof(ImuData);
    if (tracking_imu_7_cb_.value_) {
      tracking_imu_7_char_->setValue((uint8_t*)tracking_imu_7_cb_.value_, tracking_imu_7_cb_.size_);
    }

    /* --------------------- tracking IMU 8 --------------------- */
    // create characteristic
    tracking_imu_8_char_ = service_->createCharacteristic(
        characteristics::kTrackingIMU8.uuid, characteristics::kTrackingIMU8.access_property);
    SetDecriptorsInBLECharacteristic(
        tracking_imu_8_char_, (BleCharacteristicDescription*)&characteristics::kTrackingIMU8);
    // set callbacks
    tracking_imu_8_char_->setCallbacks(&tracking_imu_8_cb_);
    // assign value
    tracking_imu_8_cb_.size_ = sizeof(ImuData);
    if (tracking_imu_8_cb_.value_) {
      tracking_imu_8_char_->setValue((uint8_t*)tracking_imu_8_cb_.value_, tracking_imu_8_cb_.size_);
    }
  }

  service_->start();
}

/**
 * Adds the service created with `init()` to the advertisement and start
 * advertising, so that other devices can see the service
 */
void BleServer::Advertize() {
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(kServiceUuid);
  advertising->setScanResponse(true);
  // functions that help with iPhone connections issue
  advertising->setMinPreferred(0x06);
  advertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

}  // namespace ble
}  // namespace sensint
