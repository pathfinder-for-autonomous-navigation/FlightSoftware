/**
 * @file Serializer.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_

#include <bitset>
#include <cmath>
#include "GPSTime.hpp"
#include "types.hpp"

class SerializerBase
{
public:
    /**
     * @brief Set of macros defining fixed and minimum bitsizes for compressed objects.
     */
    constexpr static size_t bool_sz = 1;
    constexpr static size_t f_quat_sz = 30;
    constexpr static size_t d_quat_sz = 30;
    constexpr static size_t gps_time_sz = 30;
    constexpr static size_t temp_sz = 30;

    constexpr static size_t f_vec_min_sz = 30;
    constexpr static size_t f_vec_component_sz = 9;
    constexpr static size_t f_quat_element_sz = 9;
    constexpr static size_t d_vec_min_sz = 30;
    constexpr static size_t d_vec_component_sz = 9;
    constexpr static size_t d_quat_element_sz = 9;
};

/**
 * @brief Provides serialization functionality for use by SerializableStateField.
 * 
 * @tparam T Type to serialize.
 * @tparam compressed_sz Size, in bits, of compressed object.
 */
template <typename T, typename U, size_t compressed_sz>
class Serializer : SerializerBase
{
protected:
    U _min;
    U _max;

public:
    /**
     * @brief Construct a new Serializer object
     * 
     * A minimum and a maximum value must be supplied to the serializer, which 
     * will use them in some way to construct a fixed-point scheme for compressing
     * numerical data. 
     * 
     * For example, some integer state field might have a minimum value of 0 
     * and a maximum value of 100. So all allowable values can be represented 
     * in 7 bits (since 2^7 = 128), which is significantly less than the 
     * 32-bit allocation of a standard integer.
     * 
     * @param min The minimum value of the object that can be compressed.
     * @param max The maximum value of the object that can be compressed.
     */
    Serializer(U min, U max) : _min(min), _max(max) {}

    /**
     * @brief Serializes a given object and stores the compressed object
     * into the provided bitset.
     * 
     * @param src 
     * @param dest 
     */
    void serialize(const T &src, std::bitset<compressed_sz> *dest);

    /**
     * @brief Deserializes a bitset and stores the result in the provided 
     * object pointer.
     * 
     * @param src 
     * @param dest 
     */
    void deserialize(const std::bitset<compressed_sz> &src, T *dest);
};

/**
 * @brief Specialization of Serializer for unsigned ints.
 */
template <size_t compressed_sz>
class Serializer<unsigned int, unsigned int, compressed_sz>
{
protected:
    unsigned int _min;
    unsigned int _max;

public:
    Serializer(unsigned int min, unsigned int max) : _min(min), _max(max) {}

    void serialize(const unsigned int &src, std::bitset<compressed_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<compressed_sz> &src, unsigned int *dest)
    {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for signed integers.
 */
template <size_t compressed_sz>
class Serializer<signed int, signed int, compressed_sz>
{
protected:
    signed int _min;
    signed int _max;

public:
    Serializer(signed int min, signed int max) : _min(min), _max(max) {}

    void serialize(const signed int &src, std::bitset<compressed_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<compressed_sz> &src, signed int *dest)
    {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for floats.
 */
template <size_t compressed_sz>
class Serializer<float, float, compressed_sz>
{
protected:
    float _min;
    float _max;

public:
    Serializer(float min, float max) : _min(min), _max(max) {}

    void serialize(const float &src, std::bitset<compressed_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<compressed_sz> &src, float *dest)
    {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for doubles.
 */
template <size_t compressed_sz>
class Serializer<double, double, compressed_sz>
{
protected:
    double _min;
    double _max;

public:
    Serializer(double min, float max) : _min(min), _max(max) {}

    void serialize(const double &src, std::bitset<compressed_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<compressed_sz> &src, double *dest)
    {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for float vector.
 */
template <size_t compressed_sz>
class Serializer<f_vector_t, float, compressed_sz>
{
protected:
    float _min;
    float _max;

public:
    Serializer(float min, float max) : _min(min), _max(max) {}

    void serialize(const f_vector_t &src, std::bitset<compressed_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<compressed_sz> &src, f_vector_t *dest)
    {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for double vector.
 */
template <size_t compressed_sz>
class Serializer<d_vector_t, double, compressed_sz>
{
protected:
    double _min;
    double _max;

public:
    Serializer(double min, double max) : _min(min), _max(max) {}

    void serialize(const d_vector_t &src, std::bitset<compressed_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<compressed_sz> &src, d_vector_t *dest)
    {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for float quaternion.
 */
template <size_t compressed_sz>
class Serializer<f_quaternion_t, float, compressed_sz>
{
protected:
    float _min;
    float _max;

public:
    Serializer(float min, float max) : _min(min), _max(max) {}

    void serialize(const f_quaternion_t &src, std::bitset<compressed_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<compressed_sz> &src, f_quaternion_t *dest)
    {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for double quaternion.
 */
template <size_t compressed_sz>
class Serializer<d_quaternion_t, double, compressed_sz>
{
protected:
    double _min;
    double _max;

public:
    Serializer(double min, double max) : _min(min), _max(max) {}

    void serialize(const d_quaternion_t &src, std::bitset<compressed_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<compressed_sz> &src, d_quaternion_t *dest)
    {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for GPS time.
 */
template <size_t compressed_sz>
class Serializer<gps_time_t, bool, compressed_sz>
{
protected:
    bool _min;
    bool _max;

public:
    Serializer(bool min, bool max) : _min(min), _max(max) {}

    void serialize(const gps_time_t &src, std::bitset<compressed_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<compressed_sz> &src, gps_time_t *dest)
    {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for temperature values.
 */
template <size_t compressed_sz>
class Serializer<temperature_t, signed int, compressed_sz>
{
protected:
    signed int _min;
    signed int _max;

public:
    static constexpr int TEMPERATURE_MIN = -40;
    static constexpr int TEMPERATURE_MIN = -125;

    Serializer(signed int min, signed int max) : _min(TEMPERATURE_MIN), _max(TEMPERATURE_MAX)
    {
        // Note: argument values are completely ignored because they don't matter.
    }

    void serialize(const gps_time_t &src, std::bitset<compressed_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<compressed_sz> &src, gps_time_t *dest)
    {
        // TODO
    }
};
#endif