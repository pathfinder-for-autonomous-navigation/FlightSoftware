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

class SerializerConstants {
  public:
    /**
     * @brief Set of constants defining fixed and minimum bitsizes for compressed objects.
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

template<typename U> class SerializerBase {
  protected:
    bool is_init;
    U _min;
    U _max;

    /**
     * @brief Argumented constructor. Accepts the same arguments as init(), but
     * should NOT be used in a public-facing interface.
     */
    SerializerBase(U min, U max) : is_init(false), _min(min), _max(max) {}
  public:
    /**
     * @brief Default constructor. Empty-initializes the minimum and maximum.
     * This constructor should NOT be used.
     */
    SerializerBase() : is_init(false), _min(), _max() {}

    /**
     * @brief A minimum and a maximum value must be supplied to the serializer, which 
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
    void init(U min, U max) {
        _min = min;
        _max = max;
        is_init = true;
    }
};

/**
 * @brief Provides serialization functionality for use by SerializableStateField.
 * 
 * @tparam T Type to serialize.
 * @tparam compressed_sz Size, in bits, of compressed object.
 */
template <typename T, typename U, size_t compressed_sz>
class Serializer : SerializerBase<U> {
  public:
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
class Serializer<bool, bool, static_cast<size_t>(1)> : public SerializerBase<bool> {
  public:
    Serializer() : SerializerBase<bool>(false, true) {}

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
class Serializer<unsigned int, unsigned int, compressed_sz> : public SerializerBase<unsigned int> {
  public:
    Serializer() : SerializerBase<unsigned int>(0, 0) {}

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
class Serializer<signed int, signed int, compressed_sz> : public SerializerBase<signed int> {
  public:
    Serializer() : SerializerBase<signed int>(0, 0) {}

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
class Serializer<float, float, compressed_sz> : public SerializerBase<float> {
  public:
    Serializer() : SerializerBase<float>(0.0f, 0.0f) {}

    void init(float min, float max) {
        _min = min;
        _max = max;
    }

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
        *dest = _min + resolution * f_bits;
    }

    void print(const float &src, std::string* dest) {
        *dest = std::string(src);
    }
};

/**
 * @brief Specialization of Serializer for doubles.
 */
template <size_t compressed_sz>
class Serializer<double, double, compressed_sz> : public SerializerBase<double> {
  public:
    Serializer() : SerializerBase<double>(0, 0) {}

    void init(double min, double max) {
        _min = min;
        _max = max;
    }

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
class Serializer<f_vector_t, float, compressed_sz> : public SerializerBase<float> {
  public:
    Serializer() : SerializerBase(0.0f, 0.0f) {}

    void serialize(const f_vector_t &src, std::bitset<compressed_sz> *dest)
    {
        float mag = sqrtf(powf(src[0], 2.0f) + pow(src[1], 2.0f) + pow(src[2], 2.0f));
        constexpr unsigned int magnitude_bitsize = compressed_sz - SerializerConstants::f_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_representation;
        
        Serializer<float, float, magnitude_bitsize> magnitude_serializer;
        magnitude_serializer.init(_min, _max);
        magnitude_serializer.serialize(mag, &magnitude_representation);

        // Compress unit vector into two compressed floats
        std::bitset<SerializerConstants::f_vec_min_sz> vec_representation;
        std::bitset<SerializerConstants::f_vec_component_sz> vec_element_representations[2];
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

                Serializer<float, float, SerializerConstants::f_vec_component_sz> vector_element_serializer;
                vector_element_serializer.init(0, sqrtf(2.0f));
                vector_element_serializer.serialize(v_element_scaled, &vec_element_representations[vec_number]);
                vec_number++;
            }
        }
        // Add floats to final bit representation
        for (int i = 0; i < SerializerConstants::f_vec_component_sz; i++)
        {
            vec_representation.set(i + 2, vec_element_representations[0][i]);
            vec_representation.set(i + 11, vec_element_representations[1][i]);
        }
        for (unsigned int i = 0; i < magnitude_bitsize; i++)
            (*dest).set(i, magnitude_representation[i]);
        for (int i = 0; i < SerializerConstants::f_vec_min_sz; i++)
            (*dest).set(i + magnitude_bitsize, vec_representation[i]);
    }

    void deserialize(const std::bitset<compressed_sz> &src, f_vector_t *dest)
    {
        constexpr size_t magnitude_bitsize = compressed_sz - SerializerConstants::f_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_packed;
        for (int i = 0; i < magnitude_bitsize; i++)
            magnitude_packed.set(i, src[i]);

        float magnitude = 0.0f;
        Serializer<float, float, magnitude_bitsize> magnitude_serializer;
        magnitude_serializer.init(_min, _max);
        magnitude_serializer.deserialize(magnitude_packed, &magnitude);

        unsigned int missing_component = (src[magnitude_bitsize] << 1) + src[magnitude_bitsize + 1];
        (*dest)[missing_component] = 1;
        int j = 0; // Index of current component being processed
        for (int i = 0; i < 3; i++)
        {
            if (i != missing_component)
            {
                std::bitset<SerializerConstants::f_vec_component_sz> vec_component_packed;
                for (int k = 0; k < SerializerConstants::f_vec_component_sz; k++)
                    vec_component_packed.set(k, src[compressed_sz - SerializerConstants::f_vec_min_sz + 2 + j * SerializerConstants::f_vec_component_sz + k]);

                Serializer<float, float, SerializerConstants::f_vec_component_sz> vec_component_serializer;
                vec_component_serializer.init(0.0f, sqrtf(2.0f));
                vec_component_serializer.deserialize(vec_component_packed, &((*dest)[i]));
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
class Serializer<d_vector_t, double, compressed_sz> : public SerializerBase<double>
{
  public:
    Serializer() : SerializerBase<double>(0.0) {}

    void serialize(const d_vector_t &src, std::bitset<compressed_sz> *dest)
    {
        double mag = sqrt(pow(src[0], 2) + pow(src[1], 2) + pow(src[2], 2));
        constexpr unsigned int magnitude_bitsize = compressed_sz - SerializerConstants::d_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_representation;

        Serializer<double, double, magnitude_bitsize> magnitude_serializer;
        magnitude_serializer.init(_min, _max);
        magnitude_serializer.serialize(mag, &magnitude_representation);

        // Compress unit vector into two compressed doubles
        std::bitset<SerializerConstants::d_vec_min_sz> vec_representation;
        std::bitset<SerializerConstants::d_vec_component_sz> vec_element_representations[2];
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
                Serializer<double, double, SerializerConstants::d_vec_component_sz> vector_element_serializer;
                vector_element_serializer.init(0, sqrt(2.0));
                vector_element_serializer.serialize(v_element_scaled, &vec_element_representations[vec_number]);
                vec_number++;
            }
        }
        // Add doubles to final bit representation
        for (int i = 0; i < SerializerConstants::d_vec_component_sz; i++)
        {
            vec_representation.set(i + 2, vec_element_representations[0][i]);
            vec_representation.set(i + 11, vec_element_representations[1][i]);
        }
        for (unsigned int i = 0; i < magnitude_bitsize; i++)
            (*dest).set(i, magnitude_representation[i]);
        for (int i = 0; i < SerializerConstants::d_vec_min_sz; i++)
            (*dest).set(i + magnitude_bitsize, vec_representation[i]);
    }

    void deserialize(const std::bitset<compressed_sz> &src, d_vector_t *dest)
    {
        constexpr size_t magnitude_bitsize = compressed_sz - SerializerConstants::d_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_packed;
        for (int i = 0; i < magnitude_bitsize; i++)
            magnitude_packed.set(i, src[i]);

        double magnitude = 0.0;
        Serializer<double, double, magnitude_bitsize> magnitude_serializer;
        magnitude_serializer.init(_min, _max);
        magnitude_serializer.deserialize(magnitude_packed, &magnitude);

        unsigned int missing_component = (src[magnitude_bitsize] << 1) + src[magnitude_bitsize + 1];
        (*dest)[missing_component] = 1;
        int j = 0; // Index of current component being processed
        for (int i = 0; i < 3; i++)
        {
            if (i != missing_component)
            {
                std::bitset<SerializerConstants::d_vec_component_sz> vec_component_packed;
                for (int k = 0; k < SerializerConstants::d_vec_component_sz; k++)
                    vec_component_packed.set(k, src[compressed_sz - SerializerConstants::d_vec_component_sz + 2 + j * SerializerConstants::d_vec_component_sz + k]);

                Serializer<double, double, magnitude_bitsize> vec_component_serializer;
                vec_component_serializer.init(0.0, sqrt(2.0));
                vec_component_serializer.deserialize(vec_component_packed, &((*dest)[i]));
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
class Serializer<f_quat_t, float, SerializerConstants::f_quat_sz> : public SerializerBase<float>
{
  public:
    Serializer() : SerializerBase<float>(0.0f, 0.0f) {}

    void serialize(const f_quat_t &src, std::bitset<SerializerConstants::f_quat_sz> *dest)
    {
        std::bitset<SerializerConstants::f_quat_component_sz> quat_element_representations[3];
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
                Serializer<float, float, SerializerConstants::f_quat_component_sz> quat_element_serializer;
                quat_element_serializer.init(0.0f, sqrtf(2.0f));
                quat_element_serializer.serialize(src[quat_number], &quat_element_representations[quat_number]);
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

    void deserialize(const std::bitset<SerializerConstants::f_quat_sz> &src, f_quat_t *dest)
    {
        int missing_element = (src[0] << 1) + src[1];
        (*dest)[missing_element] = 1;
        int j = 0; // Currently processed packed quaternion element
        for (int i = 0; i < 4; i++)
        {
            if (i != missing_element)
            {
                std::bitset<SerializerConstants::f_quat_component_sz> quat_element_packed;
                for (int k = 0; k < 9; k++)
                    quat_element_packed.set(k, src[2 + j * 9 + k]);

                // TODO replace
                Serializer<float, float, SerializerConstants::f_quat_component_sz> quat_element_serializer;
                quat_element_serializer.init(0.0f, sqrtf(2.0f));
                quat_element_serializer.deserialize(quat_element_packed, &((*dest)[i]));
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
class Serializer<d_quat_t, double, SerializerConstants::d_quat_sz> : public SerializerBase<double>
{
  public:
    Serializer() : SerializerBase<double>(0.0, 0.0) {}

    void serialize(const d_quat_t &src, std::bitset<SerializerConstants::d_quat_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<SerializerConstants::d_quat_sz> &src, d_quat_t *dest)
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
class Serializer<gps_time_t, bool, SerializerConstants::gps_time_sz> : public SerializerBase<bool> {
  public:
    Serializer() : SerializerBase<bool>(false, false) {}

    void serialize(const gps_time_t &src, std::bitset<SerializerConstants::gps_time_sz> *dest)
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

    void deserialize(const std::bitset<SerializerConstants::gps_time_sz> &src, gps_time_t *dest)
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
class Serializer<temperature_t, temperature_t, SerializerConstants::temp_sz> : public SerializerBase<temperature_t>
{
  public:
    static constexpr temperature_t TEMPERATURE_MIN = static_cast<temperature_t>(-40);
    static constexpr temperature_t TEMPERATURE_MAX = static_cast<temperature_t>(125);

    Serializer() : SerializerBase<temperature_t>(TEMPERATURE_MIN, TEMPERATURE_MAX) { }

    void serialize(const temperature_t &src, std::bitset<SerializerConstants::temp_sz> *dest)
    {
        // TODO
    }

    void deserialize(const std::bitset<SerializerConstants::temp_sz> &src, temperature_t *dest)
    {
        // TODO
    }

    void print(const temperature_t &src, std::string* dest) {
        // TODO
    }
};
#endif