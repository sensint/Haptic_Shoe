#ifndef __SENSINT_GLOBAL_SETTINGS_H__
#define __SENSINT_GLOBAL_SETTINGS_H__

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
namespace global {
namespace defaults {
static constexpr int kBaudRate = 115200;
}  // namespace defaults

static int baud_rate = defaults::kBaudRate;
}  // namespace global
}  // namespace settings
}  // namespace sensint

#endif  // __SENSINT_GLOBAL_SETTINGS_H__
