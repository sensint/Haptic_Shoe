#ifndef __SENSINT_IMU_H__
#define __SENSINT_IMU_H__

#include <Arduino.h>

#include "types.h"

namespace sensint {
namespace sensor {
// namespace imu {

class IIMU {
 public:
  /**
   * @brief Default constructor to construct a new BNO055 object.
   */
  virtual ~IIMU(){};

  /**
   * @brief Initialize the IMU.
   *
   * @return True if initialization was successful.
   */
  virtual bool Init() = 0;

  /**
   * @brief Acquire new data from the IMU. This just updates all internal data members. To get the
   * actual data, use the GetData() function.
   *
   * @param data_types Define the type of data that should be updated. The default is to update all
   * data members.
   */
  virtual void UpdateData(const uint8_t data_types = static_cast<uint8_t>(ImuDataType::kAll)) = 0;

  /**
   * @brief Get the data object.
   * @return ImuData
   */
  virtual ImuData GetData() const = 0;

  /**
   * @brief Check if the IMU's calibration is good enough to get reliable data. If not, you can
   * still get new data but you should be aware of its low reliability.
   *
   * @return bool
   */
  virtual bool IsCalibrated() const = 0;
};

class IMU : public IIMU {
 public:
  static constexpr uint8_t kDefaultId = 0;
  IMU(const uint8_t id);
  uint8_t GetId() const;
  virtual void SetTimeOffset(const uint32_t offset_ms) = 0;

 protected:
  uint8_t id_ = 0xFF;
  ImuData data_;
  bool initialized_ = false;
  uint32_t time_offset_ = 0;
};

// }  // namespace imu
}  // namespace sensor
}  // namespace sensint

#endif  // __SENSINT_IMU_H__
