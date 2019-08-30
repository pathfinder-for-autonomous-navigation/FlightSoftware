#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <array>

/**
 * @brief Float-based vector.
 *
 */
typedef std::array<float, 3> f_vector_t;

/**
 * @brief Double-based vector.
 *
 */
typedef std::array<double, 3> d_vector_t;

/**
 * @brief Float-based quaternion.
 *
 */
typedef std::array<float, 4> f_quat_t;

/**
 * @brief Double-based quaternion.
 *
 */
typedef std::array<double, 4> d_quat_t;

/**
 * @brief Wrapper around integer for a temperature.
 *
 */
typedef signed int temperature_t;

#endif