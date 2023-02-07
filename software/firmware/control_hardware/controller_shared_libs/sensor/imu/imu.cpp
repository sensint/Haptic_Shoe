#include "imu.h"

namespace sensint {
namespace sensor {
// namespace imu {

IMU::IMU(const uint8_t id) {
  id_ = id;
  initialized_ = false;
}

uint8_t IMU::GetId() const { return id_; }

// }  // namespace imu
}  // namespace sensor
}  // namespace sensint
