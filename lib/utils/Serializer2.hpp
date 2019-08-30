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
#include <memory>
#include <string>
#include "GPSTime.hpp"
#include "InitializationRequired.hpp"
#include "fixed_array.hpp"
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
    constexpr static size_t f_vec_quat_component_sz = 9;
    constexpr static size_t d_vec_min_sz = 30;
    constexpr static size_t d_vec_quat_component_sz = 9;

    static const std::vector<std::string>> serializable_types;
};

const std::vector<std::string>> SerializerConstants::serializable_types = {
    "bool", "f_quat", "f_vec", "d_quat", "d_vec", "gpstime",
    "int",  "uint",   "temp",  "float",  "double"};

template <typename T>
class SerializerBase : protected SerializerConstants {
   protected:
    T _min;
    T _max;
    bit_array serialized_val;

    /**
     * @brief Argumented constructor. This is protected to prevent construction of this
     * implementation-less base class.
     *
     * If the provided size is less than 0 (which could happen if you're constructing the magnitude
     * serializer for a vector serializer), return from the constructor and do not resize the
     * serialized bit array.
     */
    SerializerBase(T min, T max, size_t compressed_size) : _min(min), _max(max) {
        if (static_cast<int>(compressed_size) < 0) return;
        serialized_val.resize(compressed_size);
    }

   public:
    /**
     * @brief Get the stored bit array containing the serialized value.
     *
     * @return const bit_array&
     */
    const bit_array &get_bit_array() const { return serialized_val; }

    /**
     * @brief Return size of bit array held by this serializer.
     *
     * @return size_t
     */
    size_t bitsize() const { return serialized_val.size(); }

    /**
     * @brief Get string length that is necessary to print the value
     * of the stored contents.
     *
     * @return size_t
     */
    virtual size_t strlen() const;

    /**
     * @brief Set the internally stored serialized value. Do nothing if the source bit arary does
     * not have the same size as the internally stored bit array.
     */
    void set_bit_array(const bit_array &src) {
        if (src.size() != serialized_val.size()) return;
        serialized_val = src;
    }
};

template <typename T>
class Serializer : public SerializerBase {
   public:
    using SerializerBase<T>::SerializerBase;

    /**
     * @brief Serializes a given object and stores the compressed object
     * into the member bitset.
     *
     * @param src
     *
     * @return True if serialization succeeded, false if serializer was
     *         uninitialized.
     */
    void serialize(const T &src);

    /**
     * @brief Deserializes the bit array and stores the result in the provided
     * object pointer.
     *
     * @param dest
     *
     * @return True if serialization succeeded, false if serializer was
     *         uninitialized.
     */
    void deserialize(std::shared_ptr<T> &dest);

    /**
     * @brief Outputs a string representation of the source value into
     * the given destination string.
     *
     * This function does not perform length-checking on the string, so
     * it is up to the user to ensure that the requisite space is available.
     * The strlen function provided in SerializerBase can be used
     * to find the string space required by this function.
     *
     * @param src  Source value
     * @param dest Destination string
     *
     * @return True if serialization succeeded, false if serializer was
     *         uninitialized.
     */
    static void print(const T &src, char *dest);
};

#include "SerializerTypes2.inl"

#endif