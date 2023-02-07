#include "ble_characteristic_description.h"

namespace sensint {
namespace ble {

void SetDecriptorsInBLECharacteristic(BLECharacteristic *characteristic,
                                      BleCharacteristicDescription *description) {
  BLE2904 *ble2904;
  BLEDescriptor *ble_desc;

  // CCCD values
  ble_desc = new BLEDescriptor(BLEUUID((uint16_t)kBLE_UUID_CCCD));
  ble_desc->setValue(description->cccd_values, 2);
  characteristic->addDescriptor(ble_desc);

  // User description
  ble_desc = new BLEDescriptor(BLEUUID((uint16_t)kBLE_UUID_USER_DESCRIPTION));
  ble_desc->setValue(description->user_description);
  characteristic->addDescriptor(ble_desc);

  // Presentation format
  // TODO Make complete cpf
  ble2904 = new BLE2904();
  ble2904->setUnit(description->presentation_format);
  characteristic->addDescriptor(ble2904);
}

}  // namespace ble
}  // namespace sensint
