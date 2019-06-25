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

    /**
     * @brief Outputs a string representation of the source value into
     * the given destination string.
     */
    void print(const T &src, std::string* dest);
};

/**
 * @brief Specialization of Serializer for booleans.
 */
template <>
class Serializer<bool, bool, static_cast<size_t>(1)>
{
protected:
    bool _min;
    bool _max;

public:
    Serializer(bool min = false, 
               bool max = true) : _min(min), _max(max) {}

    void serialize(const bool &src, std::bitset<static_cast<size_t>(1)> *dest)
    {
        (*dest)[0] = src;
    }

    void deserialize(const std::bitset<static_cast<size_t>(1)> &src, bool *dest)
    {
        *dest = src[0];
    }

    void print(const bool &src, std::string* dest) {
        *dest = src ? "true" : "false";
    }
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

    unsigned int _resolution() const {
        return (unsigned int)lround(ceil((_max - _min) / pow(2.0f, compressed_sz)));
    }

    void serialize(const unsigned int &src, std::bitset<compressed_sz> *dest)
    {   
        unsigned int src_copy = src;
        if (src_copy > _max)
            src_copy = _max;
        if (src_copy < _min)
            src_copy = _min;
        unsigned int result_int = (src_copy - _min) / _resolution();
        std::bitset<compressed_sz> result_copy(result_int);
        *dest = result_copy;
    }

    void deserialize(const std::bitset<compressed_sz> &src, unsigned int *dest)
    {
        *dest = _min + src.to_ulong() * _resolution();
    }

    void print(const unsigned int &src, std::string* dest) {
        *dest = std::string(src);
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

    unsigned int _resolution() const
    {
        return (unsigned int)lround(ceil((_max - _min) / pow(2.0f, compressed_sz)));
    }

    void serialize(const signed int &src, std::bitset<compressed_sz> *dest)
    {
        unsigned int src_copy = src;
        if (src_copy > _max)
            src_copy = _max;
        if (src_copy < _min)
            src_copy = _min;
        unsigned int result_int = (src_copy - _min) / _resolution();
        std::bitset<compressed_sz> result_copy(result_int);
        *dest = result_copy;
    }

    void deserialize(const std::bitset<compressed_sz> &src, signed int *dest)
    {
        *dest = _min + src.to_ulong() * _resolution();
    }

    void print(const signed int &src, std::string* dest) {
        *dest = std::string(src);
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
        float src_copy = src;
        if (src_copy > _max)
            src_copy = _max;
        if (src_copy < _min)
            src_copy = _min;
        float resolution = (_max - _min) / pow(2, compressed_sz);
        unsigned int result_int = (unsigned int)((src_copy - _min) / resolution);
        std::bitset<compressed_sz> result_copy(result_int);
        *dest = result_copy;
    }

    void deserialize(const std::bitset<compressed_sz> &src, float *dest)
    {
        unsigned long f_bits = src.to_ullong();
        float resolution = (_max - _min) / pow(2, compressed_sz);
        return _min + resolution * f_bits;
    }

    void print(const float &src, std::string* dest) {
        *dest = std::string(src);
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
        double src_copy = src;
        if (src_copy > _max)
            src_copy = _max;
        if (src_copy < _min)
            src_copy = _min;
        double resolution = (_max - _min) / pow(2, compressed_sz);
        unsigned int result_int = (unsigned int)((src_copy - _min) / resolution);
        std::bitset<compressed_sz> result_copy(result_int);
        *dest = result_copy;
    }

    void deserialize(const std::bitset<compressed_sz> &src, double *dest)
    {
        unsigned long f_bits = src.to_ullong();
        double resolution = (_max - _min) / pow(2, compressed_sz);
        return _min + resolution * f_bits;
    }

    void print(const double &src, std::string* dest) {
        *dest = std::string(src);
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
        float mag = vect_mag(src.data());
        constexpr unsigned int magnitude_bitsize = compressed_sz - SerializerBase::f_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_representation;
        
        // TODO replace
        trim_float(mag, _min, _max, &magnitude_representation);

        // Compress unit vector into two compressed floats
        std::bitset<SerializerBase::f_vec_min_sz> vec_representation;
        std::bitset<SerializerBase::f_vec_component_sz> vec_element_representations[2];
        std::array<float, 3> v_mags; // Magnitudes of elements in vector
        for (int i = 0; i < 3; i++)
            v_mags[i] = std::abs(src[i]);

        int max_element_idx = 0;
        float max_element_value = std::max(std::max(v_mags[0], v_mags[1]), v_mags[2]);
        if (v_mags[0] == max_element_value)
            max_element_idx = 0;
        if (v_mags[1] == max_element_value)
            max_element_idx = 1;
        if (v_mags[2] == max_element_value)
            max_element_idx = 2;
        int vec_number = 0; // The current compressed vector bitset that we're modifying
        for (int i = 0; i < 3; i++)
        {
            if (i == max_element_idx)
            {
                std::bitset<2> largest_element_representation(i);
                vec_representation.set(0, largest_element_representation[0]);
                vec_representation.set(1, largest_element_representation[1]);
            }
            else
            {
                float v_element_scaled = src[vec_number] / mag;

                // TODO replace
                trim_float(v_element_scaled, -sqrt(2), sqrt(2), &vec_element_representations[vec_number]);
                vec_number++;
            }
        }
        // Add floats to final bit representation
        for (int i = 0; i < SerializerBase::f_vec_component_sz; i++)
        {
            vec_representation.set(i + 2, vec_element_representations[0][i]);
            vec_representation.set(i + 11, vec_element_representations[1][i]);
        }
        for (unsigned int i = 0; i < magnitude_bitsize; i++)
            (*dest).set(i, magnitude_representation[i]);
        for (int i = 0; i < SerializerBase::f_vec_min_sz; i++)
            (*dest).set(i + magnitude_bitsize, vec_representation[i]);
    }

    void deserialize(const std::bitset<compressed_sz> &src, f_vector_t *dest)
    {
        constexpr unsigned int magnitude_bitsize = compressed_sz - SerializerBase::f_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_packed;
        for (int i = 0; i < magnitude_bitsize; i++)
            magnitude_packed.set(i, src[i]);

        // TODO replace
        float magnitude = expand_float(magnitude_packed, _min, _max);

        unsigned int missing_component = (src[magnitude_bitsize] << 1) + src[magnitude_bitsize + 1];
        (*dest)[missing_component] = 1;
        int j = 0; // Index of current component being processed
        for (int i = 0; i < 3; i++)
        {
            if (i != missing_component)
            {
                std::bitset<SerializerBase::f_vec_component_sz> vec_component_packed;
                for (int k = 0; k < SerializerBase::f_vec_component_sz; k++)
                    vec_component_packed.set(k, src[compressed_sz - SerializerBase::f_vec_min_sz + 2 + j * SerializerBase::f_vec_component_sz + k]);

                // TODO replace
                (*dest)[i] = expand_float(vec_component_packed, -sqrt(2), sqrt(2));
                (*dest)[missing_component] -= pow((*dest)[i], 2);
                j++;
            }
        }
        (*dest)[missing_component] = sqrt((*dest)[missing_component]);

        for (int i = 0; i < 3; i++)
            (*dest)[i] *= magnitude;
    }

    void print(const f_vector_t &src, std::string* dest) {
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
        double mag = sqrt(pow(src[0], 2) + pow(src[1], 2) + pow(src[2], 2));
        constexpr unsigned int magnitude_bitsize = compressed_sz - SerializerBase::d_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_representation;
        trim_double(mag, _min, _max, &magnitude_representation);

        // Compress unit vector into two compressed doubles
        std::bitset<SerializerBase::d_vec_min_sz> vec_representation;
        std::bitset<SerializerBase::d_vec_component_sz> vec_element_representations[2];
        std::array<double, 3> v_mags; // Magnitudes of elements in vector
        for (int i = 0; i < 3; i++)
            v_mags[i] = std::abs(src[i]);

        int max_element_idx = 0;
        double max_element_value = std::max(std::max(v_mags[0], v_mags[1]), v_mags[2]);
        if (v_mags[0] == max_element_value)
            max_element_idx = 0;
        if (v_mags[1] == max_element_value)
            max_element_idx = 1;
        if (v_mags[2] == max_element_value)
            max_element_idx = 2;
        int vec_number = 0; // The current compressed vector bitset that we're modifying
        for (int i = 0; i < 3; i++)
        {
            if (i == max_element_idx)
            {
                std::bitset<2> largest_element_representation(i);
                vec_representation.set(0, largest_element_representation[0]);
                vec_representation.set(1, largest_element_representation[1]);
            }
            else
            {
                double v_element_scaled = src[vec_number] / mag;
                trim_double(v_element_scaled, -sqrt(2), sqrt(2), &vec_element_representations[vec_number]);
                vec_number++;
            }
        }
        // Add doubles to final bit representation
        for (int i = 0; i < SerializerBase::d_vec_component_sz; i++)
        {
            vec_representation.set(i + 2, vec_element_representations[0][i]);
            vec_representation.set(i + 11, vec_element_representations[1][i]);
        }
        for (unsigned int i = 0; i < magnitude_bitsize; i++)
            (*dest).set(i, magnitude_representation[i]);
        for (int i = 0; i < SerializerBase::d_vec_min_sz; i++)
            (*dest).set(i + magnitude_bitsize, vec_representation[i]);
    }

    void deserialize(const std::bitset<compressed_sz> &src, d_vector_t *dest)
    {
        constexpr unsigned int magnitude_bitsize = compressed_sz - SerializerBase::d_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_packed;
        for (int i = 0; i < magnitude_bitsize; i++)
            magnitude_packed.set(i, src[i]);

        // TODO replace
        double magnitude = expand_double(magnitude_packed, _min, _max);

        unsigned int missing_component = (src[magnitude_bitsize] << 1) + src[magnitude_bitsize + 1];
        (*dest)[missing_component] = 1;
        int j = 0; // Index of current component being processed
        for (int i = 0; i < 3; i++)
        {
            if (i != missing_component)
            {
                std::bitset<SerializerBase::d_vec_component_sz> vec_component_packed;
                for (int k = 0; k < SerializerBase::d_vec_component_sz; k++)
                    vec_component_packed.set(k, src[compressed_sz - SerializerBase::d_vec_component_sz + 2 + j * SerializerBase::d_vec_component_sz + k]);

                // TODO replace
                (*dest)[i] = expand_double(vec_component_packed, -sqrt(2), sqrt(2));
                (*dest)[missing_component] -= pow((*dest)[i], 2);
                j++;
            }
        }
        (*dest)[missing_component] = sqrt((*dest)[missing_component]);

        for (int i = 0; i < 3; i++)
            (*dest)[i] *= magnitude;
    }

    void print(const d_vector_t &src, std::string* dest) {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for float quaternion.
 */
template <>
class Serializer<f_quat_t, float, SerializerBase::f_quat_sz>
{
protected:
    float _min;
    float _max;
public:
    Serializer(float min, float max) : _min(min), _max(max) {}

    void serialize(const f_quat_t &src, std::bitset<SerializerBase::f_quat_sz> *dest)
    {
        std::bitset<9> quat_element_representations[3];
        std::array<float, 4> q_mags; // Magnitudes of elements in quaternion
        for (int i = 0; i < 4; i++)
            q_mags[i] = std::abs(src[i]);

        int max_element_idx = 0;
        float max_element_value = std::max(std::max(q_mags[0], q_mags[1]), std::max(q_mags[2], q_mags[3]));
        if (q_mags[0] == max_element_value)
            max_element_idx = 0;
        if (q_mags[1] == max_element_value)
            max_element_idx = 1;
        if (q_mags[2] == max_element_value)
            max_element_idx = 2;
        if (q_mags[3] == max_element_value)
            max_element_idx = 3;
        int quat_number = 0; // The current compressed vector bitset that we're modifying
        for (int i = 0; i < 4; i++)
        {
            if (i == max_element_idx)
            {
                std::bitset<2> largest_element_representation(i);
                dest->set(0, largest_element_representation[0]);
                dest->set(1, largest_element_representation[1]);
            }
            else
            {
                // TODO replace
                trim_float(src[quat_number], -sqrt(2), sqrt(2), &quat_element_representations[quat_number]);
                quat_number++;
            }
        }
        for (int i = 0; i < 9; i++)
        {
            dest->set(i + 2, quat_element_representations[0][i]);
            dest->set(i + 11, quat_element_representations[1][i]);
            dest->set(i + 20, quat_element_representations[2][i]);
        }
    }

    void deserialize(const std::bitset<SerializerBase::f_quat_sz> &src, f_quat_t *dest)
    {
        int missing_element = (src[0] << 1) + src[1];
        (*dest)[missing_element] = 1;
        int j = 0; // Currently processed packed quaternion element
        for (int i = 0; i < 4; i++)
        {
            if (i != missing_element)
            {
                std::bitset<9> quat_element_packed;
                for (int k = 0; k < 9; k++)
                    quat_element_packed.set(k, src[2 + j * 9 + k]);

                // TODO replace
                (*dest)[i] = expand_float(quat_element_packed, -sqrt(2), sqrt(2));
                (*dest)[missing_element] -= pow((*dest)[i], 2);
            }
        }
        (*dest)[missing_element] = sqrt((*dest)[missing_element]);
    }

    void print(const f_quat_t &src, std::string* dest) {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for double quaternion.
 */
template <>
class Serializer<d_quat_t, double, SerializerBase::d_quat_sz>
{
protected:
    double _min;
    double _max;
public:
    Serializer(double min, double max) : _min(min), _max(max) {}

    void serialize(const d_quat_t &src, std::bitset<SerializerBase::d_quat_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<SerializerBase::d_quat_sz> &src, d_quat_t *dest)
    {
        // TODO
    }

    void print(const d_quat_t &src, std::string* dest) {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for GPS time.
 */
template <>
class Serializer<gps_time_t, bool, SerializerBase::gps_time_sz>
{
protected:
    bool _min;
    bool _max;
public:
    Serializer(bool min, bool max) : _min(min), _max(max) {}

    void serialize(const gps_time_t &src, std::bitset<SerializerBase::gps_time_sz> *dest)
    {
        if (src.is_not_set)
        {
            dest->set(0, false);
            return;
        }
        dest->set(0, true);
        std::bitset<16> wn((unsigned short int)src.gpstime.wn);
        std::bitset<32> tow(src.gpstime.tow);
        for (int i = 0; i < 16; i++)
            dest->set(i + 1, wn[i]);
        for (int i = 0; i < 32; i++)
            dest->set(i + 17, tow[i]);
    }

    void deserialize(const std::bitset<SerializerBase::gps_time_sz> &src, gps_time_t *dest)
    {
        std::bitset<16> wn;
        std::bitset<32> tow;
        for (int i = 0; i < 16; i++)
            wn.set(i + 1, src[i]);
        for (int i = 0; i < 32; i++)
            tow.set(i + 1, src[16 + i]);
        dest->gpstime.wn = (unsigned int)wn.to_ulong();
        dest->gpstime.tow = (unsigned int)tow.to_ulong();
    }

    void print(const gps_time_t &src, std::string* dest) {
        // TODO
    }
};

/**
 * @brief Specialization of Serializer for temperature values.
 */
template <>
class Serializer<temperature_t, temperature_t, SerializerBase::temp_sz>
{
protected:
    temperature_t _min;
    temperature_t _max;
public:
    static constexpr temperature_t TEMPERATURE_MIN = static_cast<temperature_t>(-40);
    static constexpr temperature_t TEMPERATURE_MAX = static_cast<temperature_t>(125);

    Serializer(temperature_t min = TEMPERATURE_MIN,
               temperature_t max = TEMPERATURE_MAX) : _min(min), _max(max)
    {
        // Note: argument values are completely ignored because they don't matter.
    }

    void serialize(const temperature_t &src, std::bitset<SerializerBase::temp_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<SerializerBase::temp_sz> &src, temperature_t *dest)
    {
        // TODO
    }

    void print(const temperature_t &src, std::string* dest) {
        // TODO
    }
};
#endif