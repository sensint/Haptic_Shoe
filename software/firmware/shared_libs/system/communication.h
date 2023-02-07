#ifndef __SENSINT_COMMUNICATION_H__
#define __SENSINT_COMMUNICATION_H__

#include <types.h>

#include <string>
#include <vector>

namespace sensint {
namespace communication {

enum class Devices : uint8_t {
  kAll = 0x00,       // Broadcast
  kGUI = 0x01,       // PC
  kMST = 0x02,       // Master Controller
  kPD1 = 0x03,       // Peripheral Device 1
  kPD2 = 0x04,       // Peripheral Device 2
  kPD3 = 0x05,       // Peripheral Device 3
  kLeftShoe = 0x06,  // left shoe's proxy
  kRightShoe = 0x07  // right shoe's proxy
};

enum class MessageTypes : uint8_t {
  /******** status range 0x00 - 0x0F ********/
  // !reserved for generic things!
  kUndefined = 0x00,
  kSuccess = 0x01,
  kError = 0x02,

  /******** recording range 0x10 - 0x1F ********/
  kStartRecording = 0x10,
  kStopRecording = 0x11,
  kChangeRecordingInterval = 0x12,

  /******** augmentation range 0x20 - 0x2F ********/
  kStartAugmentation = 0x20,
  kStopAugmentation = 0x21,
  kSelectGrainSequence = 0x22,

  /******** material range 0x30 - 0x3F ********/
  kAddMaterial = 0x30,
  kUpdateMaterial = 0x31,
  kDeleteMaterial = 0x32,

  kAddMaterialList = 0x33,
  kUpdateMaterialList = 0x34,
  kDeleteMaterialList = 0x35,

  kAddGrainSequence = 0x36,
  kUpdateGrainSequence = 0x37,
  kDeleteGrainSequence = 0x38,

  kAddGrainSequenceList = 0x39,
  kUpdateGrainSequenceList = 0x3A,
  kDeleteGrainSequenceList = 0x3B,

  kDeleteAllMaterials = 0x3C,
  kDeleteAllGrainSequences = 0x3D,

  /******** peripheral data range 0x40 - 0x4F ********/
  kSingleAnalogSensorData = 0x40,
  kAnalogSensorDataList = 0x41,
  kSingleIMUData = 0x42,
  kIMUDataList = 0x43,
  kShoeData = 0x44,
  kReinitializeIMU = 0x45,
};

// TODO: Remove this as soon as the GUI implements the full protocol!
static constexpr uint32_t kMaterialMessageLength = 6;

// TODO: the suitable amount should be defined at some point
static constexpr int kMaxPayload = 4096;

static constexpr char kMessageDelimiter = ',';

/**
 * @brief A data structure that represents the messages send between the GUI
 * client and the peripheral controllers in form of a comma separated list of
 * values.
 *
 *  ┌────────────────────────────────────────┬─────────┬──────┐
 *  │                  METADATA              │  DATA   │ META │
 *  ├────────┬──────────┬─────────┬──────────┼─────────┼──────┤
 *  │ start  │ dest     │ type    │ length   │ payload │ end  │
 *  ├────────┼──────────┼─────────┼──────────┼─────────┼──────┤
 *  │ char   │ uint8_t  │ uint8_t │ uint32_t │ string  │ char │
 *  └────────┴──────────┴─────────┴──────────┴─────────┴──────┘
 *
 *  - start: "<" to indicate the start of a data frame
 *  - dest: ID of the controller to send data to, where "0" means broadcast (to
 *    all peripherals)
 *  - type: ID of the message type (see MessageType)
 *  - length: number of values of the payload
 *  - payload: the actual data as a string
 *  - end: ">" to indicate the end of a data frame
 */
struct DataFrame {
  static const char start = '<';
  uint8_t destination;
  MessageTypes type;
  uint32_t length;
  std::string payload;
  static const char end = '>';
};

/**
 * @brief A data structure that represents the messages send between the GUI
 * client and the peripheral controllers in a binary form.
 *
 *  ┌────────────────────────────────────────┬───────────┐
 *  │                   HEADER               │   DATA    │
 *  ├────────┬──────────┬─────────┬──────────┼───────────┤
 *  │ start  │ dest     │ type    │ length   │ payload   │
 *  ├────────┼──────────┼─────────┼──────────┼───────────┤
 *  │ string │ uint8_t  │ uint8_t │ uint32_t │ uint8_t[] │
 *  └────────┴──────────┴─────────┴──────────┴───────────┘
 *
 *  - start: "<<" to indicate the start of a data frame
 *  - dest: ID of the controller, where "0" means broadcast (to all peripherals)
 *  - type: ID of the message type
 *  - length: number of bytes of the payload
 *  - payload: the actual data as a vector of bytes
 */
struct __attribute__((__packed__)) BinaryDataFrame {
  static const char start = '<';
  uint8_t destination;
  MessageTypes type;
  uint32_t length;
  uint8_t payload[kMaxPayload] = {0};
};

/**
 * @brief Clear the serial port if there is still data available.
 */
void ClearSerialPort();

bool GetSerializedDataFrameFromSerial(std::string &str);

bool GetSerializedDataFrameFromI2C(std::string &str, bool &start_found);

void SerializeDataFrame(const DataFrame &src, std::string &dest, const bool append = false,
                        const char delimiter = kMessageDelimiter);

/**
 * @brief Parse the parameters of the raw signal from a string. The parameters
 * are separated by a specified delimiter (default is comma). The string should
 * be structured as follows: waveform, frequency, amplitude
 * example: 0,200,0.8
 *
 * @param src parameters combined in a string
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseRawSignalParameters(const std::string &src, RawSignalParameters &dest);

/**
 * @brief Parse the parameters of the raw signal from a list of strings. The
 * strings should be ordered as follows: waveform, frequency, amplitude
 *
 * @param tokens parameters as strings
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseRawSignalParameters(const std::vector<std::string> &tokens, RawSignalParameters &dest);

/**
 * @brief Serialize the parameters of a raw signal as string.
 *
 * @param src data structure of the parameters
 * @param dest the parameters as string
 * @param delimiter character to separate single parameters
 */
void SerializeRawSignalParameters(const RawSignalParameters src, std::string &dest,
                                  const bool append = false,
                                  const char delimiter = kMessageDelimiter);

/**
 * @brief Parse the parameters of an ADSR envelope from a string. The parameters
 * are separated by a specified delimiter (default is comma). The string should
 * be structured as follows: attack, decay, sustain, release
 * example: 10,0,1,20
 *
 * @param src parameters combined in a string
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseEnvelopeParameters(const std::string &src, EnvelopeParameters &dest);

/**
 * @brief Parse the parameters of an ADSR envelope a list of strings. The
 * strings should be ordered as follows: attack, decay, sustain, release
 *
 * @param tokens parameters as strings
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseEnvelopeParameters(const std::vector<std::string> &tokens, EnvelopeParameters &dest);

/**
 * @brief Serialize the parameters of an ADSR envelope as string.
 *
 * @param src data structure of the parameters
 * @param dest the parameters as string
 * @param delimiter character to separate single parameters
 */
void SerializeEnvelopeParameters(const EnvelopeParameters &src, std::string &dest,
                                 const bool append = false,
                                 const char delimiter = kMessageDelimiter);

/**
 * @brief Parse the parameters of a filter from a string. The parameters
 * are separated by a specified delimiter (default is comma). The string should
 * be structured as follows: highCutFreq, highCutRes, lowCutFreq, lowCutRes
 * example: 600,0.5,10,0.5
 *
 * @param src parameters combined in a string
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseFilterParameters(const std::string &src, FilterParameters &dest);

/**
 * @brief Parse the parameters of an ADSR envelope a list of strings. The
 * strings should be ordered as follows: highCutFreq, highCutRes, lowCutFreq,
 * lowCutRes
 *
 * @param tokens parameters as strings
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseFilterParameters(const std::vector<std::string> &tokens, FilterParameters &dest);

/**
 * @brief Serialize the parameters of a filter as string.
 *
 * @param src data structure of the parameters
 * @param dest the parameters as string
 * @param delimiter character to separate single parameters
 */
void SerializeFilterParameters(const FilterParameters &src, std::string &dest,
                               const bool append = false, const char delimiter = kMessageDelimiter);

/**
 * @brief Parse the parameters of a grain from a string. The parameters
 * are separated by a specified delimiter (default is comma). The string should
 * be structured as follows: raw_signal_params, adsr_params, filter_params,
 * duration, is_continuous
 *
 * @param src parameters combined in a string
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseGrainParameters(const std::string &src, GrainParameters &dest);

/**
 * @brief Parse the parameters of a grain from a list of strings. The strings
 * should be ordered as follows: raw_signal_params, adsr_params, filter_params,
 * duration, is_continuous
 *
 * @param src parameters as strings
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseGrainParameters(const std::vector<std::string> &tokens, GrainParameters &dest);

/**
 * @brief Serialize the parameters of a grain as string.
 *
 * @param src data structure of the parameters
 * @param dest the parameters as string
 * @param delimiter character to separate single parameters
 */
void SerializeGrainParameters(const GrainParameters &src, std::string &dest,
                              const bool append = false, const char delimiter = kMessageDelimiter);

/**
 * @brief Parse the parameters of a material from a string. The parameters
 * are separated by a specified delimiter (default is comma). The string should
 * be structured as follows: mat_id, grain_params
 *
 * @param src parameters combined in a string
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseMaterial(const std::string &src, Material &dest);

/**
 * @brief Parse the parameters of a material from a list of strings. The strings
 * should be ordered as follows: mat_id, grain_params
 *
 * @param src parameters as strings
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseMaterial(const std::vector<std::string> &tokens, Material &dest);

/**
 * @brief Serialize the parameters of a material as string.
 *
 * @param src data structure of the parameters
 * @param dest the parameters as string
 * @param delimiter character to separate single parameters
 */
void SerializeMaterial(const Material &src, std::string &dest, const bool append = false,
                       const char delimiter = kMessageDelimiter);

/**
 * @brief Parse the parameters of a list of material from a string. The
 * parameters are separated by a specified delimiter (default is comma). The
 * string should be structured as follows: num_mat, materials
 *
 * @param src parameters combined in a string
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseMaterialList(const std::string &src, const int length, std::vector<Material> &dest);

/**
 * @brief Parse the parameters of a list of materials from a list of strings.
 * The strings should be ordered as follows: num_mat, materials
 *
 * @param src parameters as strings
 * @param dest data structure for the parameters
 *
 * @return true if parsing was successful
 */
bool ParseMaterialList(const std::vector<std::string> &tokens, const int length,
                       std::vector<Material> &dest);

void SerializeMaterialList(const std::vector<Material> &src, std::string &dest,
                           const bool append = false, const char delimiter = kMessageDelimiter);

bool ParseGrain(const std::string &src, Grain &dest);

bool ParseGrain(const std::vector<std::string> &tokens, Grain &dest);

void SerializeGrain(const Grain &src, std::string &dest, const bool append = false,
                    const char delimiter = kMessageDelimiter);

bool ParseGrainSequence(const std::string &src, const int length, GrainSequence &dest);

bool ParseGrainSequence(const std::vector<std::string> &tokens, const int length,
                        GrainSequence &dest);

void SerializeGrainSequence(const GrainSequence &src, std::string &dest, const bool append = false,
                            const char delimiter = kMessageDelimiter);

void SerializeSensorData(const AnalogSensorData &src, std::string &dest, const bool append = false,
                         const char delimiter = kMessageDelimiter);

void SerializeSensorDataList(const std::vector<AnalogSensorData> &src, std::string &dest,
                             const bool append = false, const char delimiter = kMessageDelimiter);

template <typename T>
bool ParseVector2D(const std::vector<std::string> &tokens, Vector2D<T> &dest,
                   const bool is_float = true) {
  if (tokens.size() != kVector2DNumFields) {
    return false;
  }
  if (is_float) {
    dest.x = static_cast<T>(atof(tokens[0].c_str()));
    dest.y = static_cast<T>(atof(tokens[1].c_str()));
  } else {
    dest.x = static_cast<T>(atoi(tokens[0].c_str()));
    dest.y = static_cast<T>(atoi(tokens[1].c_str()));
  }
  return true;
}

template <typename T>
void SerializeVector2D(const Vector2D<T> &vector, std::string &dest, const bool append = false,
                       const bool is_float = true, const char delimiter = kMessageDelimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  if (is_float) {
    dest += String(vector.x, 6).c_str();
    dest += delimiter;
    dest += String(vector.y, 6).c_str();
  } else {
    dest += String((int)vector.x).c_str();
    dest += delimiter;
    dest += String((int)vector.y).c_str();
  }
}

template <typename T>
bool ParseVector3D(const std::vector<std::string> &tokens, Vector3D<T> &dest,
                   const bool is_float = true) {
  if (tokens.size() != kVector3DNumFields) {
    return false;
  }
  if (is_float) {
    dest.x = static_cast<T>(atof(tokens[0].c_str()));
    dest.y = static_cast<T>(atof(tokens[1].c_str()));
    dest.z = static_cast<T>(atof(tokens[2].c_str()));
  } else {
    dest.x = static_cast<T>(atoi(tokens[0].c_str()));
    dest.y = static_cast<T>(atoi(tokens[1].c_str()));
    dest.z = static_cast<T>(atoi(tokens[2].c_str()));
  }
  return true;
}

template <typename T>
void SerializeVector3D(const Vector3D<T> &vector, std::string &dest, const bool append = false,
                       const bool is_float = true, const char delimiter = kMessageDelimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  if (is_float) {
    dest += String(vector.x, 6).c_str();
    dest += delimiter;
    dest += String(vector.y, 6).c_str();
    dest += delimiter;
    dest += String(vector.z, 6).c_str();
  } else {
    dest += String((int)vector.x).c_str();
    dest += delimiter;
    dest += String((int)vector.y).c_str();
    dest += delimiter;
    dest += String((int)vector.z).c_str();
  }
}

template <typename T>
bool ParseVector4D(const std::vector<std::string> &tokens, Vector4D<T> &dest,
                   const bool is_float = true) {
  if (tokens.size() != kVector4DNumFields) {
    return false;
  }
  if (is_float) {
    dest.w = static_cast<T>(atof(tokens[0].c_str()));
    dest.x = static_cast<T>(atof(tokens[1].c_str()));
    dest.y = static_cast<T>(atof(tokens[2].c_str()));
    dest.z = static_cast<T>(atof(tokens[3].c_str()));
  } else {
    dest.w = static_cast<T>(atoi(tokens[0].c_str()));
    dest.x = static_cast<T>(atoi(tokens[1].c_str()));
    dest.y = static_cast<T>(atoi(tokens[2].c_str()));
    dest.z = static_cast<T>(atoi(tokens[3].c_str()));
  }
  return true;
}

template <typename T>
void SerializeVector4D(const Vector4D<T> &vector, std::string &dest, const bool append = false,
                       const bool is_float = true, const char delimiter = kMessageDelimiter) {
  if (!append && !dest.empty()) {
    dest.clear();
  }
  if (is_float) {
    dest += String(vector.w, 6).c_str();
    dest += delimiter;
    dest += String(vector.x, 6).c_str();
    dest += delimiter;
    dest += String(vector.y, 6).c_str();
    dest += delimiter;
    dest += String(vector.z, 6).c_str();
  } else {
    dest += String((int)vector.w).c_str();
    dest += delimiter;
    dest += String((int)vector.x).c_str();
    dest += delimiter;
    dest += String((int)vector.y).c_str();
    dest += delimiter;
    dest += String((int)vector.z).c_str();
  }
}

bool ParseImuData(const std::vector<std::string> &tokens, ImuData &dest);

void SerializeImuData(const ImuData &src, std::string &dest, const bool append = false,
                      const char delimiter = kMessageDelimiter);

}  // namespace communication
}  // namespace sensint

#endif  // __SENSINT_COMMUNICATION_H__
