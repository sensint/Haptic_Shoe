#ifndef __SENSINT_BNO055_H__
#define __SENSINT_BNO055_H__

#include <Adafruit_BNO055.h>
#include <types.h>

#include "imu.h"

namespace sensint {
namespace sensor {
// namespace imu {

class BNO055 : public IMU {
 public:
  // !connect the ADR-pin to VCC to enforce the address 0x29
  static constexpr uint8_t kDefaultAddress = 0x29;

  /**
   * @brief Default constructor to construct a new BNO055 object.
   */
  BNO055();

  /**
   * @brief Construct a new BNO055 object.
   *
   * @param id The number of the multiplexer port, where this IMU is connected to.
   * @param address
   */
  BNO055(const uint8_t id, const uint8_t address = kDefaultAddress);

  /**
   * @brief Destroy the BNO055 object
   */
  ~BNO055();

  /**
   * @brief Initialize the IMU.
   *
   * @return True if initialization was successful.
   */
  virtual bool Init() override;

  /**
   * @brief Acquire new data from the IMU. This just updates all internal data members. To get the
   * actual data, use the GetData() function.
   *
   * @param data_types Define the type of data that should be updated. The default is to update all
   * data members.
   */
  virtual void UpdateData(
      const uint8_t data_types = static_cast<uint8_t>(ImuDataType::kAll)) override;

  /**
   * @brief Get the data object.
   * @return ImuData
   */
  virtual ImuData GetData() const override;

  /**
   * @brief Check if the IMU's calibration is good enough to get reliable data. If not, you can
   * still get new data but you should be aware of its low reliability.
   *
   * @return bool
   */
  virtual bool IsCalibrated() const override;

  /**
   * @brief Set the time offset for this IMU. This might be the time difference from another action,
   * e.g. the IMU that was retrieved before this one.
   *
   * @param offset_ms Time offset in milliseconds.
   */
  virtual void SetTimeOffset(const uint32_t offset_ms) override;

  /**
   * @brief Get the combined calibration information. Each of the four values takes 8bit and are
   * ordered as follows: sys (MSB), gyro, accel, mag.
   *
   * @return uint32_t Combined calibration of the IMU.
   */
  uint32_t GetCalibration();

 private:
  uint8_t address_;
  Adafruit_BNO055 *device_;
  // BNO055 has 4 parameters (sys, acc, gyr, mag) with value of 0 to 3.
  // 0 if not calibrated and 3 if fully calibrated (see section 34.3.54)
  // If all 4 parameters are 2 we can assume the sensor is calibrated.
  static constexpr uint8_t kCalibrationThreshold = 4 * 2;
  uint8_t cal_sys_, cal_gyr_, cal_acc_, cal_mag_ = 0;
  sensors_event_t orientation_data_, linear_accel_data_;
  //   sensors_event_t ang_velocity_data_, magnetometer_data_, accelerometer_data_, gravity_data_;
  void ConvertEventToVector(const sensors_vec_t &event, Vector3D<float> &vector);
};

// }  // namespace imu
}  // namespace sensor
}  // namespace sensint

#endif  // __SENSINT_BNO055_H__
