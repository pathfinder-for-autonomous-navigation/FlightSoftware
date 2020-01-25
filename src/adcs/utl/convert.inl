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

inline float fp(signed char si, float min, float max) {
  return min + (((float) si) + 128.0f) * (max - min) / 255.0f;
}

inline float fp(unsigned char ui, float min, float max) {
  return min + ((float) ui) * (max - min) / 255.0f;
}

inline float fp(signed short si, float min, float max) {
  return min + (((float) si) + 32768.0f) * (max - min) / 65535.0f;
}

inline float fp(unsigned short ui, float min, float max) {
  return min + ((float) ui) * (max - min) / 65535.0f;
}

inline unsigned char uc(float f, float min, float max) {
  return (unsigned char)(255.0f * (f - min) / (max - min));
}

inline signed char sc(float f, float min, float max) {
  return (signed char)(255.0f * (f - min) / (max - min) - 128.0f);
}

inline unsigned short us(float f, float min, float max) {
  return (unsigned short)(65535.0f * (f - min) / (max - min));
}

inline signed short ss(float f, float min, float max) {
  return (signed short)(65535.0f * (f - min) / (max - min) - 32768.0f);
}
}  // namespace utl
}  // namespace adcs
