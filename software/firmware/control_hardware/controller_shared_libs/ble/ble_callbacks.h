#ifndef __SENSINT_BLE_CALLBACKS_H__
#define __SENSINT_BLE_CALLBACKS_H__

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>

namespace sensint {
namespace ble {

class BleConnectionCallback : public BLEServerCallbacks {
 public:
  BleConnectionCallback();
  ~BleConnectionCallback();
  void onConnect(BLEServer *pServer);
  void onDisconnect(BLEServer *pServer);
  uint8_t connected_devices_ = 0;
};

class BleUInteger8Callback : public BLECharacteristicCallbacks {
 public:
  BleUInteger8Callback();
  ~BleUInteger8Callback();
  void onWrite(BLECharacteristic *pCharacteristic);
  void onRead(BLECharacteristic *pCharacteristic);
  uint8_t *value_ = nullptr;
};

class BleUInteger32Callback : public BLECharacteristicCallbacks {
 public:
  BleUInteger32Callback();
  ~BleUInteger32Callback();
  void onWrite(BLECharacteristic *pCharacteristic);
  void onRead(BLECharacteristic *pCharacteristic);
  uint32_t *value_ = nullptr;
};

class BleBooleanCallback : public BLECharacteristicCallbacks {
 public:
  BleBooleanCallback();
  ~BleBooleanCallback();
  void onWrite(BLECharacteristic *pCharacteristic);
  void onRead(BLECharacteristic *pCharacteristic);
  bool WasWritten();
  bool WasRead();
  bool *value_ = nullptr;

 private:
  bool was_written_ = false, was_read_ = false;
};

class BleByteArrayCallback : public BLECharacteristicCallbacks {
 public:
  BleByteArrayCallback();
  ~BleByteArrayCallback();
  void onWrite(BLECharacteristic *pCharacteristic);
  void onRead(BLECharacteristic *pCharacteristic);
  uint8_t *value_ = nullptr;
  uint16_t size_ = 0;
};

}  // namespace ble
}  // namespace sensint

#endif  // __SENSINT_BLE_CALLBACKS_H__
