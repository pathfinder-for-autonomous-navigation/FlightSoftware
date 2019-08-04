/**
 * @file Serializer.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_

#include <stdio.h>
#include <bitset>
#include <cmath>
#include "GPSTime.hpp"
#include "InitializationRequired.hpp"
#include "types.hpp"

class SerializerConstants {
   public:
    /**
     * @brief Set of constants defining fixed and minimum bitsizes for compressed
     * objects.
     */
    constexpr static size_t bool_sz = 1;
    constexpr static size_t f_quat_sz = 30;
    constexpr static size_t d_quat_sz = 30;
    constexpr static size_t gps_time_sz = 30;
    constexpr static size_t temp_sz = 30;

    constexpr static size_t f_vec_min_sz = 30;
    constexpr static size_t f_vec_component_sz = 9;
    constexpr static size_t f_quat_component_sz = 9;
    constexpr static size_t d_vec_min_sz = 30;
    constexpr static size_t d_vec_component_sz = 9;
    constexpr static size_t d_quat_component_sz = 9;
};

template <typename U>
class SerializerBase : public InitializationRequired, public SerializerConstants {
   protected:
    U _min;
    U _max;

    /**
     * @brief Argumented constructor. Accepts the same arguments as init(), but
     * should NOT be used in a public-facing interface.
     */
    SerializerBase(U min, U max)
        : InitializationRequired(), SerializerConstants(), _min(min), _max(max) {}

   public:
    /**
     * @brief Default constructor. Empty-initializes the minimum and maximum.
     * This constructor should NOT be used.
     */
    SerializerBase() : InitializationRequired(), _min(), _max() {}

    /**
     * @brief A minimum and a maximum value must be supplied to the serializer,
     * which will use them in some way to construct a fixed-point scheme for
     * compressing numerical data.
     *
     * For example, some integer state field might have a minimum value of 0
     * and a maximum value of 100. So all allowable values can be represented
     * in 7 bits (since 2^7 = 128), which is significantly less than the
     * 32-bit allocation of a standard integer.
     *
     * @param min The minimum value of the object that can be compressed.
     * @param max The maximum value of the object that can be compressed.
     * @return True if initialization was successful, false if maximum was less
     * than minimum.
     */
    bool init(U min, U max) {
        if (min > max) return false;
        _min = min;
        _max = max;
        return InitializationRequired::init();
    }
};

/**
 * @brief Provides serialization functionality for use by
 * SerializableStateField.
 *
 * @tparam T Type to serialize.
 * @tparam csz Size, in bits, of compressed object.
 */
template <typename T, typename U, size_t csz>
class Serializer : SerializerBase<U> {
   public:
    using SerializerBase<U>::SerializerBase;
    /**
     * @brief Serializes a given object and stores the compressed object
     * into the provided bitset.
     *
     * @param src
     * @param dest
     *
     * @return True if serialization succeeded, false if serializer was
     *         uninitialized.
     */
    bool serialize(const T &src, std::bitset<csz> *dest);

    /**
     * @brief Deserializes a bitset and stores the result in the provided
     * object pointer.
     *
     * @param src
     * @param dest
     *
     * @return True if serialization succeeded, false if serializer was
     *         uninitialized.
     */
    bool deserialize(const std::bitset<csz> &src, T *dest);

    /**
     * @brief Outputs a string representation of the source value into
     * the given destination string.
     *
     * @return True if serialization succeeded, false if serializer was
     *         uninitialized.
     */
    bool print(const T &src, std::string *dest);
};

#include "SerializerTypes.inl"

#endif