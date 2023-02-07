#ifndef __SENSINT_BLE_CHAR_H__
#define __SENSINT_BLE_CHAR_H__

#include <Arduino.h>
#include <BLE2904.h>
#include <BLEService.h>

#include <sstream>

namespace sensint {
namespace ble {

#define kBLE_UUID_USER_DESCRIPTION 0x2901
#define kBLE_UUID_CCCD 0x2902
#define kBLE_UUID_CPF 0x2904

#ifdef DEVICE_NANO_BLE
// https://github.com/arduino-libraries/ArduinoBLE/blob/master/src/BLEProperty.h
#define kBLE_PERMISSION_READ 0X02
#define kBLE_PERMISSION_WRITE 0X08
#define kBLE_PERMISSION_NOTIFY 0X10
#define kBLE_PERMISSION_BROADCAST 0X01
#define kBLE_PERMISSION_INDICATE 0X20
#define kBLE_PERMISSION_WRITEWITHOUTRESPONSE 0X04
#else
#define kBLE_PERMISSION_READ 0X01
#define kBLE_PERMISSION_WRITE 0X02
#define kBLE_PERMISSION_NOTIFY 0X04
#define kBLE_PERMISSION_BROADCAST 0X08
#define kBLE_PERMISSION_INDICATE 0X10
#define kBLE_PERMISSION_WRITEWITHOUTRESPONSE 0X20
#endif  // DEBUG

// Data types for characteristics
#define kBLE_CPF_RFU 0x00
#define kBLE_CPF_BOOLEAN 0x01
#define kBLE_CPF_2BIT 0x02
#define kBLE_CPF_NIBBLE 0x03
#define kBLE_CPF_UINT8 0x04
#define kBLE_CPF_UINT12 0x05
#define kBLE_CPF_UINT16 0x06
#define kBLE_CPF_UINT24 0x07
#define kBLE_CPF_UINT32 0x08
#define kBLE_CPF_UINT48 0x09
#define kBLE_CPF_UINT64 0x0A
#define kBLE_CPF_UINT128 0x0B
#define kBLE_CPF_SINT8 0x0C
#define kBLE_CPF_SINT12 0x0D
#define kBLE_CPF_SINT16 0x0E
#define kBLE_CPF_SINT24 0x0F
#define kBLE_CPF_SINT32 0x10
#define kBLE_CPF_SINT48 0x11
#define kBLE_CPF_SINT64 0x12
#define kBLE_CPF_SINT128 0x13
#define kBLE_CPF_FLOAT32 0x14
#define kBLE_CPF_FLOAT64 0x15
#define kBLE_CPF_SFLOAT 0x16
#define kBLE_CPF_FLOAT 0x17
#define kBLE_CPF_DUINT16 0x18
#define kBLE_CPF_UTF8S 0x19
#define kBLE_CPF_UTF16S 0x1A
#define kBLE_CPF_STRUCT 0x1B

struct BleCharacteristicDescription {
  std::string user_description;
  std::string uuid;
  uint32_t access_property;
  uint8_t cccd_values[2];
  uint8_t presentation_format;
};

void SetDecriptorsInBLECharacteristic(BLECharacteristic *characteristic,
                                      BleCharacteristicDescription *description);

}  // namespace ble
}  // namespace sensint

#endif  // __SENSINT_BLE_CHAR_H__
