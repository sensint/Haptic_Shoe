#include "bno055.h"

#include <quaternion.h>

#include "debug.h"

namespace sensint {
namespace sensor {
// namespace imu {

BNO055::BNO055() : BNO055(kDefaultId) {}

BNO055::BNO055(const uint8_t id, const uint8_t address) : IMU(id) {
  address_ = address;
  device_ = new Adafruit_BNO055(id_, address_);
}

BNO055::~BNO055() { delete device_; }

bool BNO055::Init() {
  if (initialized_) {
    return true;
  }
  // We use the OPERATION_MODE_IMUPLUS to the IMU's relative orientation. The zero-rotation [0,0,0]
  // (initial orientaion) is set at initialization of the IMU. To reset the orientation one need to
  // reinitialize the IMU.
  initialized_ = device_->begin(adafruit_bno055_opmode_t::OPERATION_MODE_IMUPLUS);
  return initialized_;
}

void BNO055::UpdateData(const uint8_t data_types) {
  if (!initialized_) {
    return;
  }
  data_.calibration = GetCalibration();
  data_.time_offset = time_offset_;
  if (data_types & static_cast<uint8_t>(ImuDataType::kOrientation)) {
    // device_->getEvent(&orientation_data_, Adafruit_BNO055::VECTOR_EULER);
    // ConvertEventToVector(orientation_data_.orientation, data_.orientation_euler);
    imu::Quaternion orientation_quat = device_->getQuat();
    data_.orientation_quaternion.w = (float)orientation_quat.w();
    data_.orientation_quaternion.x = (float)orientation_quat.x();
    data_.orientation_quaternion.y = (float)orientation_quat.y();
    data_.orientation_quaternion.z = (float)orientation_quat.z();
  }
  if (data_types & static_cast<uint8_t>(ImuDataType::kAccelerationLin)) {
    device_->getEvent(&linear_accel_data_, Adafruit_BNO055::VECTOR_LINEARACCEL);
    ConvertEventToVector(linear_accel_data_.acceleration, data_.acceleration_linear);
  }
  //! Everything below isn't needed for this project but might be interesting for other projects.
  //! Hence, we prepared the struct for future use.
  // if (data_types & static_cast<uint8_t>(ImuDataType::kAngularVelocity)) {
  //   device_->getEvent(&ang_velocity_data_, Adafruit_BNO055::VECTOR_GYROSCOPE);
  //   ConvertEventToVector(ang_velocity_data_.gyro, data_.angular_velocity);
  // }
  // if (data_types & static_cast<uint8_t>(ImuDataType::kAcceleration)) {
  //   device_->getEvent(&accelerometer_data_, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  //   ConvertEventToVector(accelerometer_data_.acceleration, data_.acceleration);
  // }
  // if (data_types & static_cast<uint8_t>(ImuDataType::kMagnetic)) {
  //   device_->getEvent(&magnetometer_data_, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  //   ConvertEventToVector(magnetometer_data_.magnetic, data_.magnetic);
  // }
  // if (data_types & static_cast<uint8_t>(ImuDataType::kGravity)) {
  //   device_->getEvent(&gravity_data_, Adafruit_BNO055::VECTOR_GRAVITY);
  //   ConvertEventToVector(gravity_data_.acceleration, data_.gravity);
  // }
}

ImuData BNO055::GetData() const { return data_; }

uint32_t BNO055::GetCalibration() {
  if (!initialized_) {
    return 0;
  }
  device_->getCalibration(&cal_sys_, &cal_gyr_, &cal_acc_, &cal_mag_);
  return (cal_mag_ & 0xF) | ((cal_acc_ & 0xF) << 4) | ((cal_gyr_ & 0xF) << 8) |
         ((cal_sys_ & 0xF) << 12);
}

bool BNO055::IsCalibrated() const {
  if (!initialized_) {
    return false;
  }
#ifdef SENSINT_DEBUG
  if (debug::kDebugLevel == debug::DebugLevel::verbose) {
    Serial.println();
    Serial.print("Calibration: Sys=");
    Serial.print(cal_sys_);
    Serial.print(" Gyr=");
    Serial.print(cal_gyr_);
    Serial.print(" Acc=");
    Serial.print(cal_acc_);
    Serial.print(" Mag=");
    Serial.println(cal_mag_);
  }
#endif  // SENSINT_DEBUG
  return (cal_sys_ + cal_gyr_ + cal_acc_ + cal_mag_) >= kCalibrationThreshold;
}

void BNO055::SetTimeOffset(const uint32_t offset_ms) { time_offset_ = offset_ms; }

void BNO055::ConvertEventToVector(const sensors_vec_t &event, Vector3D<float> &vector) {
  vector.x = event.x;
  vector.y = event.y;
  vector.z = event.z;
}

// }  // namespace imu
}  // namespace sensor
}  // namespace sensint
