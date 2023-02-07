
#include "communication.h"

#include <Arduino.h>
#include <Wire.h>
#include <helper.h>

#include "debug.h"
#include "i2c.h"

namespace sensint {
namespace communication {

void ClearSerialPort() {
  while (Serial.available()) {
    Serial.read();
  }
}

bool GetSerializedDataFrameFromSerial(std::string &str) {
  auto res = false;
  if (!str.empty()) {
    str.clear();
  }
  while (Serial.available()) {
    if (Serial.read() == DataFrame::start) {
      res = true;
      break;
    }
  }
  if (res) {
#ifdef ESP32
    str = Serial.readStringUntil(DataFrame::end).c_str();
#else
    str = Serial.readStringUntil(DataFrame::end, kMaxPayload).c_str();
#endif
  }
  return res;
}

bool GetSerializedDataFrameFromI2C(std::string &str, bool &start_found) {
  if (!start_found) {
    if (!str.empty()) {
      str.clear();
    }
    while (SENSINT_I2C.available()) {
      if (SENSINT_I2C.read() == DataFrame::start) {
        start_found = true;
        break;
      }
    }
  }
  while (SENSINT_I2C.available()) {
    auto tmp = SENSINT_I2C.read();
    if (tmp == DataFrame::end) {
      return true;
    }
    str += tmp;
  }
  return false;
}

void SerializeDataFrame(const DataFrame &src, std::string &dest, const bool append,
                        const char delimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  dest += src.start;
  dest += String((int)src.destination).c_str();
  dest += delimiter;
  dest += String((int)src.type).c_str();
  dest += delimiter;
  dest += String(src.length).c_str();
  dest += delimiter;
  dest += src.payload;
  dest += src.end;
}

bool ParseRawSignalParameters(const std::string &src, RawSignalParameters &dest) {
  std::vector<std::string> tokens;
  if (helper::SplitString(src, tokens)) {
    return ParseRawSignalParameters(tokens, dest);
  }
  return false;
}

bool ParseRawSignalParameters(const std::vector<std::string> &tokens, RawSignalParameters &dest) {
  if (tokens.size() != kRawSignalParameterNumFields) {
    return false;
  }
  dest.waveform = static_cast<Waveform>((uint8_t)atoi(tokens[0].c_str()));
  dest.frequency = (float)atof(tokens[1].c_str());
  dest.amplitude = (float)atof(tokens[2].c_str());
  return true;
}

void SerializeRawSignalParameters(const RawSignalParameters src, std::string &dest,
                                  const bool append, const char delimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  dest += String((uint8_t)src.waveform).c_str();
  dest += delimiter;
  dest += String(src.frequency, 2).c_str();
  dest += delimiter;
  dest += String(src.amplitude, 2).c_str();
}

bool ParseEnvelopeParameters(const std::string &src, EnvelopeParameters &dest) {
  std::vector<std::string> tokens;
  if (helper::SplitString(src, tokens)) {
    return ParseEnvelopeParameters(tokens, dest);
  }
  return false;
}

bool ParseEnvelopeParameters(const std::vector<std::string> &tokens, EnvelopeParameters &dest) {
  if (tokens.size() != kEnvelopeParameterNumFields) {
    return false;
  }
  dest.attack = (float)atof(tokens[0].c_str());
  dest.decay = (float)atof(tokens[1].c_str());
  dest.sustain = (float)atof(tokens[2].c_str());
  dest.release = (float)atof(tokens[3].c_str());
  return true;
}

void SerializeEnvelopeParameters(const EnvelopeParameters &src, std::string &dest,
                                 const bool append, const char delimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  dest += String(src.attack, 2).c_str();
  dest += delimiter;
  dest += String(src.decay, 2).c_str();
  dest += delimiter;
  dest += String(src.sustain, 2).c_str();
  dest += delimiter;
  dest += String(src.release, 2).c_str();
}

bool ParseFilterParameters(const std::string &src, FilterParameters &dest) {
  std::vector<std::string> tokens;
  if (helper::SplitString(src, tokens)) {
    ParseFilterParameters(tokens, dest);
  }
  return false;
}

bool ParseFilterParameters(const std::vector<std::string> &tokens, FilterParameters &dest) {
  if (tokens.size() != kFilterParameterNumFields) {
    return false;
  }
  dest.highCutFrequency = (float)atof(tokens[0].c_str());
  dest.highCutResonance = (float)atof(tokens[1].c_str());
  dest.lowCutFrequency = (float)atof(tokens[2].c_str());
  dest.lowCutFrequency = (float)atof(tokens[3].c_str());
  return true;
}

void SerializeFilterParameters(const FilterParameters &src, std::string &dest, const bool append,
                               const char delimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  dest += String(src.highCutFrequency, 2).c_str();
  dest += delimiter;
  dest += String(src.highCutResonance, 2).c_str();
  dest += delimiter;
  dest += String(src.lowCutFrequency, 2).c_str();
  dest += delimiter;
  dest += String(src.lowCutResonance, 2).c_str();
}

bool ParseGrainParameters(const std::string &src, GrainParameters &dest) {
  std::vector<std::string> tokens;
  if (helper::SplitString(src, tokens)) {
    return ParseGrainParameters(tokens, dest);
  }
  return false;
}

bool ParseGrainParameters(const std::vector<std::string> &tokens, GrainParameters &dest) {
  if (tokens.size() !=
      kRawSignalParameterNumFields + kEnvelopeParameterNumFields + kFilterParameterNumFields + 2) {
    return false;
  }

  auto raw_start_it = tokens.begin();
  auto raw_end_it = tokens.begin() + (kRawSignalParameterNumFields - 1);
  std::vector<std::string> signal_tokens{raw_start_it, raw_end_it};
  if (!ParseRawSignalParameters(signal_tokens, dest.raw_signal_params)) {
    return false;
  }

  auto env_start_it = tokens.begin() + kRawSignalParameterNumFields;
  auto env_end_it =
      tokens.begin() + (kRawSignalParameterNumFields + kEnvelopeParameterNumFields - 1);
  std::vector<std::string> envelope_tokens{env_start_it, env_end_it};
  if (!ParseEnvelopeParameters(envelope_tokens, dest.envelope_params)) {
    return false;
  }

  auto fil_start_it = tokens.begin() + (kRawSignalParameterNumFields + kEnvelopeParameterNumFields);
  auto fil_end_it = tokens.begin() + (kRawSignalParameterNumFields + kEnvelopeParameterNumFields +
                                      kFilterParameterNumFields - 1);
  std::vector<std::string> filter_tokens{fil_start_it, fil_end_it};
  if (!ParseFilterParameters(filter_tokens, dest.filter_params)) {
    return false;
  }

  dest.duration = atof(tokens[tokens.size() - 2].c_str());
  dest.is_continuous = atoi(tokens.back().c_str()) == 1;

  return true;
}

void SerializeGrainParameters(const GrainParameters &src, std::string &dest, const bool append,
                              const char delimiter) {
  std::string signal_str;
  SerializeRawSignalParameters(src.raw_signal_params, signal_str);
  std::string envelope_str;
  SerializeEnvelopeParameters(src.envelope_params, envelope_str);
  std::string filter_str;
  SerializeFilterParameters(src.filter_params, filter_str);
  if (!append && !dest.empty()) {
    dest.clear();
  }
  dest += signal_str;
  dest += delimiter;
  dest += envelope_str;
  dest += delimiter;
  dest += filter_str;
  dest += delimiter;
  dest += String(src.duration, 2).c_str();
  dest += delimiter;
  dest += src.is_continuous ? "1" : "0";
}

bool ParseMaterial(const std::string &src, Material &dest) {
  std::vector<std::string> tokens;
  if (helper::SplitString(src, tokens)) {
    return ParseMaterial(tokens, dest);
  }
  return false;
}

bool ParseMaterial(const std::vector<std::string> &tokens, Material &dest) {
  //! This implementation is tentative! Should be deleted as soon as the GUI
  //! implements the full protocol. (Delete until dashed line and uncomment the
  //! actual implementation.)
  // TODO: implement
  //  The order of the tokens is: id, is_cont, waveform, freq, amp, duration
  if (tokens.size() != kMaterialMessageLength) {
    return false;
  }
  dest.id = static_cast<uint8_t>(atoi(tokens[0].c_str()));
  dest.grain_params.is_continuous = atoi(tokens[1].c_str()) == 1;
  dest.grain_params.raw_signal_params.waveform =
      static_cast<Waveform>((uint8_t)atoi(tokens[2].c_str()));
  dest.grain_params.raw_signal_params.frequency = (float)atof(tokens[3].c_str());
  dest.grain_params.raw_signal_params.amplitude = (float)atof(tokens[4].c_str());
  dest.grain_params.duration = (float)atof(tokens[5].c_str());

#ifdef SENSINT_DEBUG
  if (debug::kDebugLevel == debug::DebugLevel::verbose) {
    PrintMaterial(dest);
  }
#endif  // SENSINT_DEBUG

  return true;

  //! ---------------------------------------------------------------------
  //! This is the actual implementation.
  // TODO: implement
  //  if (tokens.size() != kMaterialNumFields) {
  //    return false;
  //  }
  //  dest.id = static_cast<uint8_t>(atoi(tokens.front().c_str()));
  //  std::vector<std::string> grain_tokens{tokens.begin() + 1, tokens.end()};
  //  ParseGrainParameters(grain_tokens, dest.grain_params);
  //  return true;
}

void SerializeMaterial(const Material &src, std::string &dest, const bool append,
                       const char delimiter) {
  //! This implementation is tentative! Should be deleted as soon as the GUI
  //! implements the full protocol. (Delete until dashed line and uncomment the
  //! actual implementation.)
  // TODO: implement
  if (!append && !dest.empty()) {
    dest.clear();
  }
  dest += String((int)src.id).c_str();
  dest += delimiter;
  dest += src.grain_params.is_continuous ? "1" : "0";
  dest += delimiter;
  dest += String((uint8_t)src.grain_params.raw_signal_params.waveform).c_str();
  dest += delimiter;
  dest += String(src.grain_params.raw_signal_params.frequency, 2).c_str();
  dest += delimiter;
  dest += String(src.grain_params.raw_signal_params.amplitude, 2).c_str();
  dest += delimiter;
  dest += String(src.grain_params.duration, 2).c_str();

  //! ---------------------------------------------------------------------
  //! This is the actual implementation.
  // TODO: implement
  //  if (!append && !dest.empty()) {
  //    dest.clear();
  //  }
  //  dest += String((int)src.id).c_str();
  //  dest += delimiter;
  //  std::string grain_str;
  //  SerializeGrainParameters(src.grain_params, grain_str);
  //  dest += grain_str;
}

bool ParseMaterialList(const std::string &src, const int length, std::vector<Material> &dest) {
  std::vector<std::string> tokens;
  if (helper::SplitString(src, tokens)) {
    return ParseMaterialList(tokens, length, dest);
  }
  return false;
}

bool ParseMaterialList(const std::vector<std::string> &tokens, const int length,
                       std::vector<Material> &dest) {
  //! This implementation is tentative! Should be deleted as soon as the GUI
  //! implements the full protocol. (Delete until dashed line and uncomment the
  //! actual implementation.)
  // TODO: implement
  if (tokens.size() != length * kMaterialMessageLength) {
    return false;
  }
  if (!dest.empty()) {
    dest.clear();
  }
  for (int i = 0; i < length; i++) {
    std::vector<std::string> material_tokens{
        tokens.begin() + (i * kMaterialMessageLength),
        tokens.begin() + ((i * kMaterialMessageLength) + kMaterialMessageLength)};
    Material material;
    if (!ParseMaterial(material_tokens, material)) {
      return false;
    }
    dest.push_back(material);
  }
  return true;

  //! ---------------------------------------------------------------------
  //! This is the actual implementation.
  // TODO: implement
  //  if (tokens.size() * length != length * kMaterialNumFields) {
  //    return false;
  //  }
  //  if (!dest.empty()) {
  //    dest.clear();
  //  }
  //  for (size_t i = 0; i < length; i++) {
  //    std::vector<std::string> material_tokens{
  //        tokens.begin() + (i * kMaterialNumFields),
  //        tokens.begin() + ((i * kMaterialNumFields) + (kMaterialNumFields -
  //        1))};
  //    Material material;
  //    if (!ParseMaterial(material_tokens, material)) {
  //      return false;
  //    }
  //    dest.push_back(material);
  //  }
  //  return true;
}

void SerializeMaterialList(const std::vector<Material> &src, std::string &dest, const bool append,
                           const char delimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  for (const auto &material : src) {
    std::string material_str;
    SerializeMaterial(material, material_str);
    dest += material_str;
    dest += delimiter;
  }
  dest.pop_back();
}

bool ParseGrain(const std::string &src, Grain &dest) {
  std::vector<std::string> tokens;
  if (helper::SplitString(src, tokens)) {
    return ParseGrain(tokens, dest);
  }
  return false;
}

bool ParseGrain(const std::vector<std::string> &tokens, Grain &dest) {
  if (tokens.size() != kGrainNumFields) {
    return false;
  }
  dest.material_id = static_cast<uint8_t>(atoi(tokens[0].c_str()));
  dest.pos_start = static_cast<analog_sensor_t>(atoi(tokens[1].c_str()));
  dest.pos_end = static_cast<analog_sensor_t>(atoi(tokens[2].c_str()));
  return true;
}

void SerializeGrain(const Grain &src, std::string &dest, const bool append, const char delimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  dest += String((int)src.material_id).c_str();
  dest += delimiter;
  dest += String((int)src.pos_start).c_str();
  dest += delimiter;
  dest += String((int)src.pos_end).c_str();
}

bool ParseGrainSequence(const std::string &src, const int length, GrainSequence &dest) {
  std::vector<std::string> tokens;
  if (helper::SplitString(src, tokens)) {
    return ParseGrainSequence(tokens, length, dest);
  }
  return false;
}

bool ParseGrainSequence(const std::vector<std::string> &tokens, const int length,
                        GrainSequence &dest) {
  if (tokens.size() != (kGrainNumFields * length) + 1) {
    return false;
  }
  dest.id = static_cast<uint8_t>(atoi(tokens.front().c_str()));
  for (int i = 0; i < length; i++) {
    std::vector<std::string> grain_tokens{
        tokens.begin() + 1 + (i * kGrainNumFields),
        tokens.begin() + 1 + ((i * kGrainNumFields) + kGrainNumFields)};
    Grain grain;
    if (!ParseGrain(grain_tokens, grain)) {
      return false;
    }
    dest.grains.push_back(grain);
  }
#ifdef SENSINT_DEBUG
  if (debug::kDebugLevel == debug::DebugLevel::verbose) {
    PrintGrainSequence(dest);
  }
#endif  // SENSINT_DEBUG
  return true;
}

void SerializeGrainSequence(const GrainSequence &src, std::string &dest, const bool append,
                            const char delimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  dest += String((int)src.id).c_str();
  for (const auto &grain : src.grains) {
    std::string grains_str;
    SerializeGrain(grain, grains_str);
    dest += grains_str;
    dest += delimiter;
  }
  dest.pop_back();
}

void SerializeSensorData(const AnalogSensorData &src, std::string &dest, const bool append,
                         const char delimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  dest += String((int)src.device_id).c_str();
  dest += delimiter;
  dest += String((int)src.value).c_str();
}

void SerializeSensorDataList(const std::vector<AnalogSensorData> &src, std::string &dest,
                             const bool append, const char delimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  for (const auto &sensor : src) {
    std::string sensor_str;
    SerializeSensorData(sensor, sensor_str);
    dest += sensor_str;
    dest += delimiter;
  }
  dest.pop_back();
}

bool ParseImuData(const std::vector<std::string> &tokens, ImuData &dest) {
  if (tokens.size() != kImuDataNumFields) {
    return false;
  }
  std::vector<std::string> orientation_tokens{tokens.begin(), tokens.begin() + kVector4DNumFields};
  ParseVector4D<float>(orientation_tokens, dest.orientation_quaternion);
  std::vector<std::string> acceleration_tokens{
      tokens.begin() + (kVector4DNumFields + 1),
      tokens.begin() + (kVector4DNumFields + 1 + kVector3DNumFields)};
  ParseVector3D<float>(acceleration_tokens, dest.acceleration_linear);
  dest.calibration = static_cast<uint32_t>(atoi(tokens[7].c_str()));
  dest.time_offset = static_cast<uint32_t>(atoi(tokens[8].c_str()));
  return true;
}

void SerializeImuData(const ImuData &src, std::string &dest, const bool append,
                      const char delimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  SerializeVector4D<float>(src.orientation_quaternion, dest, true);
  dest += delimiter;
  SerializeVector3D<float>(src.acceleration_linear, dest, true);
  dest += delimiter;
  dest += String((int)src.calibration).c_str();
  dest += delimiter;
  dest += String((int)src.time_offset).c_str();
}

}  // namespace communication
}  // namespace sensint
