#include "ble_callbacks.h"

#ifdef SENSINT_DEBUG
#include <debug.h>
#endif  // SENSINT_DEBUG

namespace sensint {
namespace ble {

/******************************************************************************
                              BleConnectionCallback
 ******************************************************************************/

BleConnectionCallback::BleConnectionCallback() {}

BleConnectionCallback::~BleConnectionCallback() {}

void BleConnectionCallback::onConnect(BLEServer *pServer) {
  connected_devices_++;
  BLEDevice::startAdvertising();
}
void BleConnectionCallback::onDisconnect(BLEServer *pServer) {
  connected_devices_--;
  BLEDevice::startAdvertising();
}

/******************************************************************************
                              BleUInteger8Callback
 ******************************************************************************/

BleUInteger8Callback::BleUInteger8Callback() {}

BleUInteger8Callback::~BleUInteger8Callback() {}

void BleUInteger8Callback::onWrite(BLECharacteristic *pCharacteristic) {
  auto data_length = pCharacteristic->getValue().length();
  if (value_ && (data_length == sizeof(uint8_t))) {
    auto pData = pCharacteristic->getData();
    *value_ = pData[0];
#ifdef SENSINT_DEBUG
    debug::Log("BleUInteger8Callback::onWrite", "data: " + String((int)*value_),
               debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  }
#ifdef SENSINT_DEBUG
  else {
    debug::Log("BleUInteger8Callback::onWrite",
               "Wrong data length! Expected " + String(sizeof(uint8_t)) + " bytes, got " +
                   String(data_length),
               debug::DebugLevel::verbose);
  }
#endif  // SENSINT_DEBUG
}

void BleUInteger32Callback::onRead(BLECharacteristic *pCharacteristic) {}

/******************************************************************************
                              BleUInteger32Callback
 ******************************************************************************/

BleUInteger32Callback::BleUInteger32Callback() {}

BleUInteger32Callback::~BleUInteger32Callback() {}

void BleUInteger32Callback::onWrite(BLECharacteristic *pCharacteristic) {
  auto data_length = pCharacteristic->getValue().length();
  if (value_ && (data_length == sizeof(uint32_t))) {
    auto pData = pCharacteristic->getData();
    uint32_t tmp = pData[0] | (pData[1] << 8) | (pData[2] << 16) | (pData[3] << 24);
    *value_ = tmp;
#ifdef SENSINT_DEBUG
    debug::Log("BleUInteger32Callback::onWrite", "data: " + String((int)tmp),
               debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  }
#ifdef SENSINT_DEBUG
  else {
    debug::Log("BleUInteger32Callback::onWrite",
               "Wrong data length! Expected " + String(sizeof(uint32_t)) + " bytes, got " +
                   String(data_length),
               debug::DebugLevel::verbose);
  }
#endif  // SENSINT_DEBUG
}

void BleUInteger8Callback::onRead(BLECharacteristic *pCharacteristic) {}

/******************************************************************************
                             BleBooleanCallback
 ******************************************************************************/

BleBooleanCallback::BleBooleanCallback() {}

BleBooleanCallback::~BleBooleanCallback() {}

void BleBooleanCallback::onWrite(BLECharacteristic *pCharacteristic) {
  was_written_ = true;
  if (value_) {
    pCharacteristic->getData()[0] == 0 ? *value_ = false : *value_ = true;
  }
}

void BleBooleanCallback::onRead(BLECharacteristic *pCharacteristic) { was_read_ = true; }

/**
 * @return Whether the ble characteristic was read since the last function call
 * or not.
 */
bool BleBooleanCallback::WasRead() {
  if (was_read_) {
    was_read_ = false;
    return true;
  } else {
    return was_read_;
  }
}

/**
 * @return Whether the ble characteristic was read since the last function call
 * or not.
 */
bool BleBooleanCallback::WasWritten() {
  if (was_written_) {
    was_written_ = false;
    return true;
  } else {
    return was_written_;
  }
}

/******************************************************************************
                              BleByteArrayCallback
 ******************************************************************************/

BleByteArrayCallback::BleByteArrayCallback() {}

BleByteArrayCallback::~BleByteArrayCallback() {}

void BleByteArrayCallback::onWrite(BLECharacteristic *pCharacteristic) {
  auto data_length = pCharacteristic->getValue().length();
  if (value_ && (data_length == size_)) {
    auto pData = pCharacteristic->getData();
    for (int i = 0; i < size_; i++) {
      value_[i] = pData[i];
    }
#ifdef SENSINT_DEBUG
    debug::Log("BleByteArrayCallback::onWrite", "data size:" + String(data_length),
               debug::DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  }
#ifdef SENSINT_DEBUG
  else {
    debug::Log(
        "BleByteArrayCallback::onWrite",
        "Wrong data length! Expected " + String(size_) + " bytes, got " + String(data_length),
        debug::DebugLevel::verbose);
  }
#endif  // SENSINT_DEBUG
}

void BleByteArrayCallback::onRead(BLECharacteristic *pCharacteristic) {}

}  // namespace ble
}  // namespace sensint
