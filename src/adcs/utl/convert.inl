//
// src/adcs/utl/convert.inl
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "convert.hpp"

namespace adcs {
namespace utl {

inline float fp(int8_t si, float min, float max) {
  return min + (((float) si) + 128.0f) * (max - min) / 255.0f;
}

inline float fp(uint8_t ui, float min, float max) {
  return min + ((float) ui) * (max - min) / 255.0f;
}

inline float fp(int16_t si, float min, float max) {
  return min + (((float) si) + 32768.0f) * (max - min) / 65535.0f;
}

inline float fp(uint16_t ui, float min, float max) {
  return min + ((float) ui) * (max - min) / 65535.0f;
}

inline uint8_t uc(float f, float min, float max) {
  return (uint8_t)(255.0f * (f - min) / (max - min));
}

inline int8_t sc(float f, float min, float max) {
  return (int8_t)(255.0f * (f - min) / (max - min) - 128.0f);
}

inline uint16_t us(float f, float min, float max) {
  return (uint16_t)(65535.0f * (f - min) / (max - min));
}

inline int16_t ss(float f, float min, float max) {
  return (int16_t)(65535.0f * (f - min) / (max - min) - 32768.0f);
}
}  // namespace utl
}  // namespace adcs
