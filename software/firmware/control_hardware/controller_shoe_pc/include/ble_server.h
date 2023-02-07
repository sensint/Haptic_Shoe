#ifndef __SENSINT_BLE_SERVER_H__
#define __SENSINT_BLE_SERVER_H__

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "ble_callbacks.h"
#include "ble_characteristic_description.h"

namespace sensint {
namespace ble {

class BleServer {
 public:
  BleServer();
  ~BleServer();
  /*******************************************************************************
                              status and config group
   ******************************************************************************/
  BLECharacteristic *recording_status_char_;
  BleUInteger32Callback recording_status_cb_;

  BLECharacteristic *recording_interval_char_;
  BleUInteger32Callback recording_interval_cb_;

  BLECharacteristic *augmentation_active_char_;
  BleBooleanCallback augmentation_active_cb_;

  BLECharacteristic *imu_reinitialize_char_;
  BleBooleanCallback imu_reinitialize_cb_;

  BleConnectionCallback connection_cb_;

  /*******************************************************************************
                                  left shoe group
   ******************************************************************************/
  BLECharacteristic *left_shoe_connected_char_;
  BleBooleanCallback left_shoe_connected_cb_;

  BLECharacteristic *left_shoe_timestamp_char_;
  BleUInteger32Callback left_shoe_timestamp_cb_;

  BLECharacteristic *left_shoe_sensors_char_;
  BleByteArrayCallback left_shoe_sensors_cb_;

  BLECharacteristic *left_shoe_imu_char_;
  BleByteArrayCallback left_shoe_imu_cb_;

  BLECharacteristic *left_shoe_sequence_char_;
  BleUInteger8Callback left_shoe_sequence_cb_;

  /*******************************************************************************
                                  right shoe group
   ******************************************************************************/
  BLECharacteristic *right_shoe_connected_char_;
  BleBooleanCallback right_shoe_connected_cb_;

  BLECharacteristic *right_shoe_timestamp_char_;
  BleUInteger32Callback right_shoe_timestamp_cb_;

  BLECharacteristic *right_shoe_sensors_char_;
  BleByteArrayCallback right_shoe_sensors_cb_;

  BLECharacteristic *right_shoe_imu_char_;
  BleByteArrayCallback right_shoe_imu_cb_;

  BLECharacteristic *right_shoe_sequence_char_;
  BleUInteger8Callback right_shoe_sequence_cb_;

  /*******************************************************************************
                                   tracking group
   ******************************************************************************/
  BLECharacteristic *tracking_connected_char_;
  BleBooleanCallback tracking_connected_cb_;

  BLECharacteristic *tracking_num_imu_char_;
  BleUInteger8Callback tracking_num_imu_cb_;

  BLECharacteristic *tracking_connected_imu_char_;
  BleUInteger8Callback tracking_connected_imu_cb_;

  BLECharacteristic *tracking_data_selection_char_;
  BleUInteger8Callback tracking_data_selection_cb_;

  BLECharacteristic *tracking_imu_1_char_;
  BleByteArrayCallback tracking_imu_1_cb_;

  BLECharacteristic *tracking_imu_2_char_;
  BleByteArrayCallback tracking_imu_2_cb_;

  BLECharacteristic *tracking_imu_3_char_;
  BleByteArrayCallback tracking_imu_3_cb_;

  BLECharacteristic *tracking_imu_4_char_;
  BleByteArrayCallback tracking_imu_4_cb_;

  BLECharacteristic *tracking_imu_5_char_;
  BleByteArrayCallback tracking_imu_5_cb_;

  BLECharacteristic *tracking_imu_6_char_;
  BleByteArrayCallback tracking_imu_6_cb_;

  BLECharacteristic *tracking_imu_7_char_;
  BleByteArrayCallback tracking_imu_7_cb_;

  BLECharacteristic *tracking_imu_8_char_;
  BleByteArrayCallback tracking_imu_8_cb_;

  void Init();
  void Advertize();

 private:
  static const std::string kServiceUuid;
  static const std::string kDeviceName;
  // !This number has to be updated when adding or removing characteristics
  static const unsigned int kNumApplicationCharacteristics = 26;
  // needed to calculate amount of handles for ble service
  static const unsigned int kNumCharacteristics = kNumApplicationCharacteristics;

  BLEServer *server_;
  BLEService *service_;
};

}  // namespace ble
}  // namespace sensint

#endif  // __SENSINT_BLE_SERVER_H__
