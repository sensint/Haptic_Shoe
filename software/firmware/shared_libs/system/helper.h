#ifndef __SENSINT_HELPER_H__
#define __SENSINT_HELPER_H__

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "communication.h"

namespace sensint {
namespace helper {

/**
 * @brief converts two bytes to a uint16_t
 *
 * @param bytes two bytes to combine
 * @param little_endian decides whether the bytes are in little endian (default)
 * or big
 * @return uint16_t
 */
static uint16_t BytesToUint16(const uint8_t* bytes, bool little_endian = true) {
  uint16_t value = 0;
  if (little_endian) {
    value = ((uint16_t)bytes[0] << 8) | bytes[1];
  } else {
    value = ((uint16_t)bytes[1] << 8) | bytes[0];
  }
  return value;
}

/**
 * @brief converts four bytes to a uint32_t
 *
 * @param bytes four bytes to combine
 * @param little_endian decides whether the bytes are in little endian (default)
 * or big
 * @return uint16_t
 */
static uint32_t BytesToUint32(const uint8_t* bytes, bool little_endian = true) {
  uint32_t value = 0;
  if (little_endian) {
    value = ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) |
            bytes[3];
  } else {
    value = ((uint32_t)bytes[3] << 24) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[1] << 8) |
            bytes[0];
  }
  return value;
}

/**
 * @brief converts four bytes to a uint32_t
 *
 * @param bytes four bytes to combine
 * @param little_endian decides whether the bytes are in little endian (default)
 * or big
 */
static float BytesToFloat(const uint8_t* bytes, bool little_endian = true) {
  // uint32_t and float are both 4 bytes
  union {
    uint32_t u;
    float f;
  } u;
  // convert bytes to uint32_t
  u.u = BytesToUint32(bytes, little_endian);
  // finally return the float
  return u.f;
}

/**
 * @brief Split a string into a vector of tokens based on a delimiter character
 *
 * @param src reference of the source string
 * @param tokens reference of a vector to store the string tokens
 * @param delimiter character to split at (default is ',')
 * @return false if no tokens available
 */
static bool SplitString(const std::string& src, std::vector<std::string>& tokens,
                        const char delimiter = communication::kMessageDelimiter) {
  if (!tokens.empty()) {
    tokens.clear();
  }
  std::stringstream ss(src);
  std::string token;
  while (std::getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }
  return !tokens.empty();
}

}  // namespace helper
}  // namespace sensint

#endif  // __SENSINT_HELPER_H__
