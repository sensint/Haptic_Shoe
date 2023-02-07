#ifndef __SENSINT_BLE_CONFIG_H__
#define __SENSINT_BLE_CONFIG_H__

#include <string>

#include "ble_characteristic_description.h"

namespace sensint {
namespace ble {
namespace config {

// this is the same for all UUIDs
static const std::string kUUIDSuffix("-e3da-4a9a-9c2b-b8fe2dd451d1");

static const auto kServiceUUID = "d0ff495f" + kUUIDSuffix;

namespace characteristics {

/*******************************************************************************
 *                        Characteristic UUIDs schema
 * -----------------------------------------------------------------------------
 *
 * xxxxGGCC-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 *
 * where x is a fixed hexadecimal digit,
 * G is a group, and
 * C is a characteristic.
 * This allows for a maximum of 255 groups and 255 characteristics per group.
 *
 * Groups:
 *  00: status and config
 *  01: left shoe
 *  02: right shoe
 *  03: motion tracking system
 ******************************************************************************/

/*******************************************************************************
                              status and config group
 ******************************************************************************/
static const BleCharacteristicDescription kRecordingStatus = {
    "Recording Status",
    std::string("d0ff0000" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY,
    {0x01, 0x01},
    kBLE_CPF_UINT32};

static const BleCharacteristicDescription kRecordingInterval = {
    "Recording Interval",
    std::string("d0ff0001" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY,
    {0x01, 0x01},
    kBLE_CPF_UINT32};

static const BleCharacteristicDescription kAugmentationActive = {
    "Augmentation active",
    std::string("d0ff0002" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY,
    {0x01, 0x01},
    kBLE_CPF_BOOLEAN};

static const BleCharacteristicDescription kIMUReinitialize = {
    "IMU reinitialize",
    std::string("d0ff0003" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY,
    {0x01, 0x01},
    kBLE_CPF_BOOLEAN};

/*******************************************************************************
                                left shoe group
 ******************************************************************************/
static const BleCharacteristicDescription kLeftShoeConnected = {
    "Left Shoe connected",
    std::string("d0ff01a0" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_BOOLEAN};

static const BleCharacteristicDescription kLeftShoeTimestamp = {
    "Left Shoe Timestamp",
    std::string("d0ff0100" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_UINT32};

static const BleCharacteristicDescription kLeftShoeSensorData = {
    "Left Shoe Sensor Data",
    std::string("d0ff0101" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kLeftShoeIMU = {
    "Left Shoe IMU",
    std::string("d0ff0102" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kLeftShoeSequence = {
    "Left Shoe Sequence",
    std::string("d0ff0103" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY,
    {0x01, 0x01},
    kBLE_CPF_UINT8};

/*******************************************************************************
                                right shoe group
 ******************************************************************************/
static const BleCharacteristicDescription kRightShoeConnected = {
    "Right Shoe connected",
    std::string("d0ff02a0" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_BOOLEAN};

static const BleCharacteristicDescription kRightShoeTimestamp = {
    "Right Shoe Timestamp",
    std::string("d0ff0200" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_UINT32};

const BleCharacteristicDescription kRightShoeSensorData = {
    "Right Shoe Sensor Data",
    std::string("d0ff0201" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kRightShoeIMU = {
    "Left Shoe IMU",
    std::string("d0ff0202" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kRightShoeSequence = {
    "Right Shoe Sequence",
    std::string("d0ff0203" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY,
    {0x01, 0x01},
    kBLE_CPF_UINT8};

/*******************************************************************************
                                 tracking group
 ******************************************************************************/
static const BleCharacteristicDescription kTrackingConnected = {
    "Tracking connected",
    std::string("d0ff03a0" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_BOOLEAN};

static const BleCharacteristicDescription kTrackingNumberOfIMUs = {
    "Tracking Number of IMUs",
    std::string("d0ff03a1" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_UINT8};

static const BleCharacteristicDescription kTrackingConnectedIMUs = {
    "Tracking connected IMU IDs",
    std::string("d0ff03a2" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_UINT8};

static const BleCharacteristicDescription kTrackingDataSelection = {
    "Tracking Data Selection",
    std::string("d0ff03a3" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY,
    {0x01, 0x01},
    kBLE_CPF_UINT8};

static const BleCharacteristicDescription kTrackingIMU1 = {
    "Tracking IMU 1",
    std::string("d0ff0301" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kTrackingIMU2 = {
    "Tracking IMU 2",
    std::string("d0ff0302" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kTrackingIMU3 = {
    "Tracking IMU 3",
    std::string("d0ff0303" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kTrackingIMU4 = {
    "Tracking IMU 4",
    std::string("d0ff0304" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kTrackingIMU5 = {
    "Tracking IMU 5",
    std::string("d0ff0305" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kTrackingIMU6 = {
    "Tracking IMU 6",
    std::string("d0ff0306" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kTrackingIMU7 = {
    "Tracking IMU 7",
    std::string("d0ff0307" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

static const BleCharacteristicDescription kTrackingIMU8 = {
    "Tracking IMU 8",
    std::string("d0ff0308" + kUUIDSuffix),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE,
    {0x01, 0x01},
    kBLE_CPF_STRUCT};

}  // namespace characteristics
}  // namespace config
}  // namespace ble
}  // namespace sensint

#endif  // __SENSINT_BLE_CONFIG_H__
