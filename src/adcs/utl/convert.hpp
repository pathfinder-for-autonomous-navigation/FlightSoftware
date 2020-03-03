//
// src/adcs/utl/convert.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_UTL_CONVERT_HPP_
#define SRC_ADCS_UTL_CONVERT_HPP_

namespace adcs {
namespace utl {

/** Converts a int8_t to a floating point value. */
inline float fp(int8_t si, float min, float max);

/** Converts an uint8_t to a floating point value. */
inline float fp(uint8_t ui, float min, float max);

/** Converts a int16_t to a floating point value. */
inline float fp(int16_t si, float min, float max);

/** Converts an uint16_t to a floating point value. */
inline float fp(uint16_t ui, float min, float max);

/** Converts a floating point value to an uint8_t. */
inline uint8_t uc(float f, float min, float max);

/** Converts a floating point value to a int8_t */
inline int8_t sc(float f, float min, float max);

/** Converts a floating point value to an uint16_t. */
inline uint16_t us(float f, float min, float max);

/** Convertes a floating point value to a int16_t. */
inline int16_t ss(float f, float min, float max);

}  // namespace utl
}  // namespace adcs

#include "convert.inl"

#endif
