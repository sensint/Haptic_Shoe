#ifndef __SENSINT_TYPES_H__
#define __SENSINT_TYPES_H__

#include <Arduino.h>

#include <string>
#include <vector>

#include "debug.h"

namespace sensint {

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kVector2DNumFields = 2;
template <typename T>
struct Vector2D {
  T x;
  T y;
};

#ifdef SENSINT_DEBUG
template <typename T>
static void PrintVector2D(const Vector2D<T> &vector, bool is_float = true) {
  if (is_float) {
    Serial.printf("x=%f, y=%f\n", vector.x, vector.y);
  } else {
    Serial.printf("x=%d, y=%d\n", (int)vector.x, (int)vector.y);
  }
}
#endif  // SENSINT_DEBUG

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kVector3DNumFields = 3;
template <typename T>
struct Vector3D {
  T x;
  T y;
  T z;
};

#ifdef SENSINT_DEBUG
template <typename T>
static void PrintVector3D(const Vector3D<T> &vector, bool is_float = true) {
  if (is_float) {
    Serial.printf("x=%f, y=%f, z=%f\n", vector.x, vector.y, vector.z);
  } else {
    Serial.printf("x=%d, y=%d, z=%d\n", (int)vector.x, (int)vector.y, (int)vector.z);
  }
}
#endif  // SENSINT_DEBUG

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kVector4DNumFields = 4;
template <typename T>
struct Vector4D {
  T w;
  T x;
  T y;
  T z;
};

#ifdef SENSINT_DEBUG
template <typename T>
static void PrintVector4D(const Vector4D<T> &vector, bool is_float = true) {
  if (is_float) {
    Serial.printf("w=%f, x=%f, y=%f, z=%f\n", vector.w, vector.x, vector.y, vector.z);
  } else {
    Serial.printf("w=%d, x=%d, y=%d, z=%d\n", (int)vector.w, (int)vector.x, (int)vector.y,
                  (int)vector.z);
  }
}
#endif  // SENSINT_DEBUG

template <typename T>
union SerializableStruct {
  T data;
  uint8_t serialized[sizeof(T)];
};

template <typename T, std::size_t N>
union SerializableArray {
  T data[N];
  uint8_t serialized[N * sizeof(T)];
};

typedef uint16_t analog_sensor_t;

enum class RecordingStatus : int {
  kIdle = 0,
  kRecording = 1,
};

/**
 * @brief Enumeration of waveforms that can be used for the signal generation.
 * The values are copied from the Teensy TactileAudio Library.
 */
enum class Waveform : uint8_t {
  kSine = 0,
  kSawtooth = 1,
  kSquare = 2,
  kTriangle = 3,
  kArbitrary = 4,
  kPulse = 5,
  kSawtoothReverse = 6
};

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kEnvelopeParameterNumFields = 4;
/**
 * @brief A data structure to specify the parameters of an ADSR envelope.
 * (A) Attack
 * (D) Decay
 * (S) Sustain
 * (R) Release
 *
 * amp
 *  │      ■ A
 *  │     / \
 *  │    /   ■────────■ S
 *  │   /    D         \
 *  │  ■                ■ R
 *  └─────────────────────── time
 */
struct EnvelopeParameters {
  /**
   * @brief Duration in milliseconds for the attack phase, i.e. the time it
   * should take from level (amplitude) 0.0 to the final signal level (peak).
   */
  float attack = 0.f;
  /**
   * @brief Duration in milliseconds from peak (i.e. after attack) to the
   * sustain level.
   */
  float decay = 0.f;
  /**
   * @brief The signal level (0.0 - 1.0) to hold until the release phase begins.
   */
  float sustain = 1.f;
  /**
   * @brief Duration in milliseconds to fade out the signal when noteOff() is
   * called.
   */
  float release = 0.f;
};

#ifdef SENSINT_DEBUG
static void PrintEnvelopeParameters(const EnvelopeParameters &adsr) {
  Serial.printf("envelope \t A:%f D:%f S:%f R:%f\n", adsr.attack, adsr.decay, adsr.sustain,
                adsr.release);
}
#endif  // SENSINT_DEBUG

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kRawSignalParameterNumFields = 3;
/**
 * @brief A data structure to specify the parameters a the raw signal (i.e.
 * wave). These parameters will be dynamically set by the user in an external
 * tool on the PC.
 */
struct RawSignalParameters {
  /**
   * @brief The basic waveform of the signal.
   */
  Waveform waveform = Waveform::kSine;
  /**
   * @brief The frequency (Hz) of the raw signal. To account for the mechanical
   * limits of the actuators this parameter should be probably less than 400Hz.
   */
  float frequency = 200.f;
  /**
   * @brief The signal level of the raw signal (0.0 - 1.0). This parameter
   * should be as high as possible to keep the output quality. Set the global
   * max. level at the amplifier attached to the system.
   */
  float amplitude = 1.f;
};

#ifdef SENSINT_DEBUG
static void PrintRawSignalParameters(const RawSignalParameters &signal) {
  Serial.printf("signal \t wave:%d freq:%f ampl:%f\n", static_cast<int>((uint8_t)signal.waveform),
                signal.frequency, signal.amplitude);
}
#endif  // SENSINT_DEBUG

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kFilterParameterNumFields = 4;
/**
 * @brief A data structure to specify the audio filter parameters.
 *                        |    UNIT    | RANGE       | COMMENT
 * float highCutFrequency |    Hz      | >0          | usually ~200 - 600
 * float highCutResonance |    Q       | >0, <0.7071 | if higher, requires
 * pre-gain reduction to avoid clipping
 *  float lowCutFrequency  |    Hz      |
 * >0          | usually ~10 - 100
 *  float lowCutResonance  |    Q       | >0,
 * <0.7071 | if higher, requires pre-gain reduction to avoid clipping
 */
struct FilterParameters {
  float highCutFrequency = 600.f;
  float highCutResonance = 0.5f;
  float lowCutFrequency = 10.f;
  float lowCutResonance = 0.5f;
};

#ifdef SENSINT_DEBUG
static void PrintFilterParameters(const FilterParameters &filter) {
  Serial.printf("filter \t freqCH:%f resCH:%f freqCL:%f resCL:%f\n", filter.highCutFrequency,
                filter.highCutResonance, filter.lowCutFrequency, filter.lowCutResonance);
}
#endif  // SENSINT_DEBUG

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kGrainParameterNumFields =
    kRawSignalParameterNumFields + kEnvelopeParameterNumFields + kFilterParameterNumFields + 2;

/**
 * @brief A data structure to combine all parameters of a haptic grain.
 */
struct GrainParameters {
  RawSignalParameters raw_signal_params;
  EnvelopeParameters envelope_params;
  FilterParameters filter_params;
  /**
   * @brief Duration of a single pulse in microseconds (usually max ~400000).
   */
  float duration = 10000.0f;
  /**
   * @brief If this parameter is set to true, the duration parameter is ignored.
   */
  bool is_continuous = false;
};

#ifdef SENSINT_DEBUG
static void PrintGrainParameters(const GrainParameters &grain) {
  Serial.println("Grain Parameters");
  PrintRawSignalParameters(grain.raw_signal_params);
  PrintEnvelopeParameters(grain.envelope_params);
  PrintFilterParameters(grain.filter_params);
  Serial.printf("grain \t dur:%f cont:%d\n", grain.duration, grain.is_continuous);
}
#endif  // SENSINT_DEBUG

static constexpr uint8_t kDefaultMaterialID = 0x00;

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kMaterialNumFields = kGrainParameterNumFields + 1;
/**
 * @brief A data structure that defines tactile material properties. These
 * properties could also be just applied to a smaller region (in sensor range)
 * of a material.
 */
struct Material {
  /**
   * @brief The identifier of a material.
   */
  uint8_t id = kDefaultMaterialID;
  /**
   * @brief The properties of this particular material.
   */
  GrainParameters grain_params;
};

#ifdef SENSINT_DEBUG
static void PrintMaterial(const Material &material) {
  Serial.printf("Material >>> id:%d\n", (int)material.id);
  PrintGrainParameters(material.grain_params);
}
#endif  // SENSINT_DEBUG

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kGrainNumFields = 3;
/**
 * @brief A data structure that defines the position (or region) of a tactile
 * grain (i.e. pulse) and its material identifier.
 */
struct Grain {
  uint8_t material_id = kDefaultMaterialID;
  analog_sensor_t pos_start = 0;
  analog_sensor_t pos_end = 0;
};

#ifdef SENSINT_DEBUG
static void PrintGrain(const Grain &grain) {
  Serial.printf("grain mat:%d start:%d end:%d\n", (int)grain.material_id, grain.pos_start,
                grain.pos_end);
}
#endif  // SENSINT_DEBUG

static constexpr uint8_t kDefaultGrainSequenceID = 0x00;

/**
 * @brief  A data structure to define a tactile material as a sequence of grains
 * at certain positions.
 */
struct GrainSequence {
  uint8_t id = kDefaultGrainSequenceID;
  std::vector<sensint::Grain> grains;
};

#ifdef SENSINT_DEBUG
static void PrintGrainSequence(const GrainSequence &sequence) {
  Serial.printf("Grain Sequence >>> id:%d\n", (int)sequence.id);
  for (size_t i = 0; i < sequence.grains.size(); i++) {
    Serial.printf("[%d] ", (int)i);
    PrintGrain(sequence.grains[i]);
  }
}
#endif  // SENSINT_DEBUG

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kAnalogSensorDataNumFields = 2;

struct AnalogSensorData {
  uint8_t device_id;
  analog_sensor_t value;
};

#ifdef SENSINT_DEBUG
static void PrintAnalogSensorData(const AnalogSensorData &data) {
  Serial.printf("sensor device:%d value:%d\n", (int)data.device_id, (int)data.value);
}
#endif  // SENSINT_DEBUG

/**
 * @brief IMU data type selection.
 */
enum class ImuDataType : uint8_t {
  // All
  kAll = 0b11111111,
  // Orientation
  kOrientation = 0b00000001,
  // Linear Acceleration Vector for the three physical axes (w/o gravity)
  kAccelerationLin = 0b00000010,
  //! Everything below isn't needed for this project but might be interesting for other projects.
  //! Hence, we prepared the enum for future use.
  // Acceleration Vector  for the three physical axes (w gravity)
  // kAcceleration = 0b00000100,
  // Angular Velocity Vector  for the three physical axes
  // kAngularVelocity = 0b00001000,
  // Magnetic Field Strength Vector
  // kMagnetic = 0b00010000,
  // Gravity Vector
  // kGravity = 0b00100000,
};

//! Update this parameter if the number of fields in the struct changes.
static constexpr uint32_t kImuDataNumFields = 9;

/**
 * @brief Data storage for a single IMU. This includes the data from the IMU, as well as the
 * calibration information and time offset.
 */
struct ImuData {
  /** Orientation (Quaternion Vector, 100Hz)
   */
  Vector4D<float> orientation_quaternion;

  /** Linear Acceleration Vector (100Hz)
   * Three axis of linear acceleration data (acceleration minus gravity) in m/s^2
   */
  Vector3D<float> acceleration_linear;

  /** Calibration Status
   * Bitmask of calibration status (8bit per status).
   */
  uint32_t calibration;

  /** Time Offset (milliseconds)
   * All IMUs are read sequentially. Hence, this is the time offset between readings.
   */
  uint32_t time_offset;

  //! Everything below isn't needed for this project but might be interesting for other projects.
  //! Hence, we prepared the struct for future use.

  /** Orientation (Euler Vector, 100Hz)
   * Three axis orientation data based on a 360° sphere
   * (roll: y, pitch: z, yaw: x) in degrees.
   * x = [0, 360]; y = [-90, 90]; z = [-180, 180]
   */
  // Vector3D<float> orientation_euler;

  /** Acceleration Vector (100Hz)
   * Three axis of acceleration (gravity + linear motion) in m/s^2
   */
  // Vector3D<float> acceleration;

  /** Angular Velocity Vector (100Hz)
   * Three axis (gyroscope) of 'rotation speed' in rad/s
   */
  // Vector3D<float> angular_velocity;

  /** Magnetic Field Strength Vector (20Hz)
   * Three axis of magnetic field sensing in micro Tesla (uT)
   */
  // Vector3D<float> magnetic;

  /** Gravity Vector (100Hz)
   * Three axis of gravitational acceleration (minus any movement) in m/s^2
   */
  // Vector3D<float> gravity;
};

#ifdef SENSINT_DEBUG
static void PrintImuData(const ImuData &data, const uint8_t data_types) {
  Serial.printf("---- IMU data ----\n > dt=%d\n > calib=%d\n", data.time_offset, data.calibration);
  if (data_types & static_cast<uint8_t>(ImuDataType::kOrientation)) {
    // Serial.print(" > Ori: ");
    // PrintVector3D(data.orientation_euler);
    Serial.print(" > Ori q: ");
    PrintVector4D(data.orientation_quaternion);
  }
  if (data_types & static_cast<uint8_t>(ImuDataType::kAccelerationLin)) {
    Serial.print(" > Acc L: ");
    PrintVector3D(data.acceleration_linear);
  }
  //! Everything below isn't needed for this project but might be interesting for other projects.
  //! Hence, we prepared the code for future use.
  // if (data_types & static_cast<uint8_t>(ImuDataType::kAcceleration)) {
  //   Serial.print(" > Acc: ");
  //   PrintVector3D(data.acceleration);
  // }
  // if (data_types & static_cast<uint8_t>(ImuDataType::kAngularVelocity)) {
  //   Serial.print(" > Ang V: ");
  //   PrintVector3D(data.angular_velocity);
  // }
  // if (data_types & static_cast<uint8_t>(ImuDataType::kMagnetic)) {
  //   Serial.print(" > Mag: ");
  //   PrintVector3D(data.magnetic);
  // }
  // if (data_types & static_cast<uint8_t>(ImuDataType::kGravity)) {
  //   Serial.print(" > Gra: ");
  //   PrintVector3D(data.gravity);
  // }
}
#endif  // SENSINT_DEBUG

}  // namespace sensint

#endif  // __SENSINT_TYPES_H__
