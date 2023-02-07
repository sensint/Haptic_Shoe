#ifndef __SENSINT_ANALOG_SENSOR_H__
#define __SENSINT_ANALOG_SENSOR_H__

#include <Arduino.h>
#include <types.h>

namespace sensint {
namespace sensor {

enum class SetupType : uint8_t { kInput = 0x00, kInputPullUp = 0x01 };

class AnalogSensor {
 public:
  static constexpr uint8_t kDefaultOutputResolution = 10;
  static constexpr uint8_t kDefaultJitterThreshold = 7;
  static constexpr float kDefaultFilterWeight = 0.08;
  static constexpr analog_sensor_t kDefaultLowerLimit = 0;
  static constexpr analog_sensor_t kDefaultUpperLimit = (1 << kDefaultOutputResolution) - 1;

  /**
   * @brief Construct a new Analog Sensor object
   */
  AnalogSensor();

  /**
   * @brief Construct a new Analog Sensor object
   *
   * @param sensing_pin the analog pin to read from
   * @param setup either with a pull-up resistor or without
   * @param output_resolution number of bits to use for the output
   * @param lower_limit lowest value that should be used
   * @param upper_limit highest value that should be used
   * @param jitter_threshold the (one-side) distance of the grain position to
   * the current sensor position
   */
  explicit AnalogSensor(const uint8_t sensing_pin, const SetupType setup,
                        const uint8_t output_resolution = kDefaultOutputResolution,
                        const analog_sensor_t lower_limit = kDefaultLowerLimit,
                        const analog_sensor_t upper_limit = kDefaultUpperLimit,
                        const analog_sensor_t jitter_threshold = kDefaultJitterThreshold);

  /**
   * @brief Destroy the Analog Sensor object
   */
  ~AnalogSensor();

  /**
   * @brief Initialize the sensor before using it. This can be done in the
   * setup() function.
   */
  void Initialize();

  /**
   * @brief Get the pin to read from.
   *
   * @return the analog pin
   */
  const uint8_t GetSensingPin() const;

  /**
   * @brief Set the pin to read from. This causes a reinitialization of the
   * sensor.
   *
   * @param pin the analog pin
   */
  void SetSensingPin(const uint8_t pin);

  /**
   * @brief Get the output resolution (number of bits).
   *
   * @return the output resolution
   */
  const uint8_t GetOutputResolution() const;

  /**
   * @brief Set the output resolution (number of bits).
   *
   * @param bits the output resolution
   */
  void SetOutputResolution(const uint8_t bits);

  /**
   * @brief Read the sensor value in the predefined resolution.
   *
   * @return the sensor value
   */
  analog_sensor_t Read();

  /**
   * @brief Read the sensor value in the predefined resolution and apply a
   * filter.
   *
   * @return the sensor value
   */
  analog_sensor_t ReadFiltered(const float weight = kDefaultFilterWeight);

  /**
   * @brief Get the last measured value without reading the sensor again
   *
   * @return analog_sensor_t
   */
  analog_sensor_t GetLastRawValue() const;

  /**
   * @brief Get the last filtered value without reading the sensor again
   *
   * @return analog_sensor_t
   */
  analog_sensor_t GetLastFilteredValue() const;

  /**
   * @brief Get the lowest value that should be used (threshold).
   *
   * @return analog_sensor_t
   */
  analog_sensor_t GetLowerLimit() const;

  /**
   * @brief Set the lowest value that should be used (threshold).
   */
  void SetLowerLimit(const analog_sensor_t limit);

  /**
   * @brief Get the highest value that should be used (threshold).
   *
   * @return analog_sensor_t
   */
  analog_sensor_t GetUpperLimit() const;

  /**
   * @brief Set the highest value that should be used (threshold).
   */
  void SetUpperLimit(const analog_sensor_t limit);

  /**
   * @brief Get the threshold used to compensate for jitter.
   *
   * @return analog_sensor_t
   */
  analog_sensor_t GetJitterThreshold() const;

  /**
   * @brief Set the threshold used to compensate for jitter.
   */
  void SetJitterThreshold(const analog_sensor_t threshold);

 private:
  uint8_t sensing_pin_;
  SetupType setup_;
  uint8_t output_resolution_;
  analog_sensor_t max_value_;
  analog_sensor_t jitter_threshold_;
  analog_sensor_t last_raw_value_;
  analog_sensor_t last_filtered_value_;
  analog_sensor_t lower_limit_;
  analog_sensor_t upper_limit_;
  bool initialized_;

  /**
   * @brief This function is used to smooth jittered sensor readings.
   *
   * @param raw_value the jittered sensor reading
   * @param filtered_value the previously smoothed value
   * @param weight the amount of smoothing [0.0 – 1.0], where lower values
   * result in smoother values
   *
   * @return smoothed sensor reading
   */
  analog_sensor_t Apply1DFilter(const analog_sensor_t raw_value,
                                const analog_sensor_t filtered_value, const float weight);
};

}  // namespace sensor
}  // namespace sensint

#endif  // __SENSINT_ANALOG_SENSOR_H__
