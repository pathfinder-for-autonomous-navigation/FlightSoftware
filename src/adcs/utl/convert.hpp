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

/** Converts a signed char to a floating point value. */
inline float fp(signed char si, float min, float max);

/** Converts an unsigned char to a floating point value. */
inline float fp(unsigned char ui, float min, float max);

/** Converts a signed short to a floating point value. */
inline float fp(signed short si, float min, float max);

/** Converts an unsigned short to a floating point value. */
inline float fp(unsigned short ui, float min, float max);

/** Converts a floating point value to an unsigned char. */
inline unsigned char uc(float f, float min, float max);

/** Converts a floating point value to a signed char */
inline signed char sc(float f, float min, float max);

/** Converts a floating point value to an unsigned short. */
inline unsigned short us(float f, float min, float max);

/** Convertes a floating point value to a signed short. */
inline signed short ss(float f, float min, float max);

}  // namespace utl
}  // namespace adcs

#include "convert.inl"

#endif
