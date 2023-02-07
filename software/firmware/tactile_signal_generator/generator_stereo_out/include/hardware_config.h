#ifndef __SENSINT_HARDWARE_CONFIG_H__
#define __SENSINT_HARDWARE_CONFIG_H__

/**
 * @brief This file provides the configuration for the hardware and software
 * components, e.g. pins or ports and parameters for communication protocols.
 * These are the parameters that are not changed during the operation of the
 * system.
 *
 * If you want to change the parameters, you have to change the corresponding
 * values in the file settings.h.
 */

namespace sensint {
namespace config {

// pins
static constexpr int kAnalogSensingPinA = A1;
static constexpr int kAnalogSensingPinB = A0;

}  // namespace config
}  // namespace sensint

#endif  // __SENSINT_HARDWARE_CONFIG_H__
