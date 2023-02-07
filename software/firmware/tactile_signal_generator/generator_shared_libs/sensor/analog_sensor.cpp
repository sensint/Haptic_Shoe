#include "analog_sensor.h"

namespace sensint {
namespace sensor {

AnalogSensor::AnalogSensor() {
  SetOutputResolution(kDefaultOutputResolution);
  initialized_ = false;
}

AnalogSensor::AnalogSensor(const uint8_t sensing_pin, const SetupType setup,
                           const uint8_t output_resolution, const analog_sensor_t lower_limit,
                           const analog_sensor_t upper_limit,
                           const analog_sensor_t jitter_threshold) {
  sensing_pin_ = sensing_pin;
  setup_ = setup;
  SetOutputResolution(output_resolution);
  if (lower_limit < upper_limit) {
    lower_limit_ = lower_limit;
    upper_limit_ = upper_limit;
  } else {
    lower_limit_ = upper_limit;
    upper_limit_ = lower_limit;
  }
  jitter_threshold_ = jitter_threshold;
  initialized_ = false;
}

AnalogSensor::~AnalogSensor() {}

void AnalogSensor::Initialize() {
  switch (setup_) {
    case SetupType::kInput:
      pinMode(sensing_pin_, INPUT);
      break;
    case SetupType::kInputPullUp:
      pinMode(sensing_pin_, INPUT_PULLUP);
      break;
    default:
      pinMode(sensing_pin_, INPUT);
      break;
  }
//! This is a tentative fix for a library bug in the Teensy pin configuration
//! (see pins_teensy.c in void pinMode - line 1205)
//! PLEASE MAKE SURE TO NOT USE THIS FOR DIGITAL PINS!!
#ifdef TEENSY35
  volatile uint32_t *config;
  config = portConfigRegister(sensing_pin_);
  *config = PORT_PCR_MUX(0);
  initialized_ = true;
#endif  // TEENSY35
}

const uint8_t AnalogSensor::GetSensingPin() const { return this->sensing_pin_; }

void AnalogSensor::SetSensingPin(const uint8_t pin) {
  sensing_pin_ = pin;
  Initialize();
}

const uint8_t AnalogSensor::GetOutputResolution() const { return output_resolution_; }

void AnalogSensor::SetOutputResolution(const uint8_t bits) {
  switch (bits) {
    case 10:
    case 12:
    case 16:
      analogReadResolution(bits);
      output_resolution_ = bits;
      max_value_ = (1 << bits) - 1;
    default:
      output_resolution_ = kDefaultOutputResolution;
      max_value_ = (1 << bits) - 1;
      break;
  }
}

analog_sensor_t AnalogSensor::Read() {
#if SENSINT_SENSOR == 0  // FSR
  last_raw_value_ =
      constrain((analog_sensor_t)analogRead(sensing_pin_), lower_limit_, upper_limit_);
#else   // slider, potentiometer
  last_raw_value_ =
      constrain(max_value_ - (analog_sensor_t)analogRead(sensing_pin_), lower_limit_, upper_limit_);
#endif  // SENSINT_SENSOR
  return last_raw_value_;
}

analog_sensor_t AnalogSensor::ReadFiltered(const float weight) {
  Read();
  last_filtered_value_ = constrain(Apply1DFilter(last_raw_value_, last_filtered_value_, weight),
                                   lower_limit_, upper_limit_);
  return last_filtered_value_;
}

analog_sensor_t AnalogSensor::GetLastRawValue() const { return last_raw_value_; }

analog_sensor_t AnalogSensor::GetLastFilteredValue() const { return last_filtered_value_; }

analog_sensor_t AnalogSensor::GetLowerLimit() const { return lower_limit_; }

void AnalogSensor::SetLowerLimit(const analog_sensor_t limit) {
  if (limit < upper_limit_) {
    lower_limit_ = limit;
  }
}

analog_sensor_t AnalogSensor::GetUpperLimit() const { return upper_limit_; }

void AnalogSensor::SetUpperLimit(const analog_sensor_t limit) {
  if (limit > lower_limit_) {
    upper_limit_ = limit;
  }
}

analog_sensor_t AnalogSensor::GetJitterThreshold() const { return jitter_threshold_; }

void AnalogSensor::SetJitterThreshold(const analog_sensor_t threshold) {
  jitter_threshold_ = threshold;
}

analog_sensor_t AnalogSensor::Apply1DFilter(const analog_sensor_t raw_value,
                                            const analog_sensor_t filtered_value,
                                            const float weight) {
  return ((1.0 - weight) * filtered_value) + (weight * raw_value);
}

}  // namespace sensor
}  // namespace sensint
