#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <array>

/**
 * @brief Float-based vector.
 *
 */
using f_vector_t = std::array<float, 3>;

/**
 * @brief Double-based vector.
 *
 */
using d_vector_t = std::array<double, 3>;

/**
 * @brief Float-based quaternion.
 *
 */
using f_quat_t = std::array<float, 4>;

/**
 * @brief Double-based quaternion.
 *
 */
using d_quat_t = std::array<double, 4>;

/**
 * @brief Wrapper around integer for a temperature.
 *
 */
using temperature_t = int32_t;

#endif