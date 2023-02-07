#ifndef __SENSINT_GLOBAL_SETTINGS_H__
#define __SENSINT_GLOBAL_SETTINGS_H__

/**
 * @brief This file provides the settings for the senSInt project.
 * These are the parameters that could change during the operation of the system
 * - either in your own code or via the serial interface.
 */

namespace sensint {
namespace settings {
namespace global {
namespace defaults {
// serial interface
static constexpr int kBaudRate = 115200;
// ble interface
static constexpr uint16_t kMTU = 517;
// motion tracking system
static constexpr uint8_t kTrackingMaxNumberOfIMUs = 8;
}  // namespace defaults

static int baud_rate = defaults::kBaudRate;

}  // namespace global
}  // namespace settings
}  // namespace sensint

#endif  // __SENSINT_GLOBAL_SETTINGS_H__
