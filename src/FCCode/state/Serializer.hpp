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

class SerializerBase {
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
template<typename T, size_t compressed_sz>
class Serializer : SerializerBase {
  protected:
    T _min;
    T _max;
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
    Serializer(T min, T max);

    /**
     * @brief Serializes a given object and stores the compressed object
     * into the provided bitset.
     * 
     * @param src 
     * @param dest 
     */
    void serialize(const T& src, std::bitset<compressed_sz>* dest);

    /**
     * @brief Deserializes a bitset and stores the result in the provided 
     * object pointer.
     * 
     * @param src 
     * @param dest 
     */
    void deserialize(const std::bitset<compressed_sz>& src, T* dest);
};

/**
 * @brief Serializer class for an unsigned integer.
 */
template<size_t compressed_sz> 
class Serializer<unsigned int, compressed_sz> {
  void serialize(const unsigned int& src, std::bitset<compressed_sz>* dest);
  void deserialize(const std::bitset<compressed_sz>& src, unsigned int* dest);
};

/**
 * @brief Serializer class for a float value.
 */
template<size_t compressed_sz>
class Serializer<float, compressed_sz> {
  void serialize(const float& src, std::bitset<compressed_sz>* dest);
  void deserialize(const std::bitset<compressed_sz>& src, float* dest);
};

/**
 * @brief Serializer class for a double value.
 */
template<size_t compressed_sz>
class Serializer<double, compressed_sz> {
  void serialize(const double& src, std::bitset<compressed_sz>* dest);
  void deserialize(const std::bitset<compressed_sz>& src, double* dest);
};

/**
 * @brief Serializer class for a float vector, i.e. std::array<float, 3>.
 */
template<size_t compressed_sz>
class Serializer<f_vector_t, compressed_sz> {
  void serialize(const f_vector_t& src, std::bitset<compressed_sz>* dest);
  void deserialize(const std::bitset<compressed_sz>& src, f_vector_t* dest);
};

/**
 * @brief Serializer class for a double vector, i.e. std::array<double, 3>.
 */
template<size_t compressed_sz>
class Serializer<d_vector_t, compressed_sz> {
  void serialize(const d_vector_t& src, std::bitset<compressed_sz>* dest);
  void deserialize(const std::bitset<compressed_sz>& src, d_vector_t* dest);
};

/**
 * @brief Serializer class for a boolean object.
 */
template<>
class Serializer<bool, SerializerBase::bool_sz> {
  void serialize(const bool& src, std::bitset<SerializerBase::bool_sz>* dest);
  void deserialize(const std::bitset<SerializerBase::bool_sz>& src, bool* dest);
};

/**
 * @brief Serializer class for a float quaternion, i.e. std::array<float, 4>.
 */
template<>
class Serializer<f_quaternion_t, SerializerBase::f_quat_sz> {
  void serialize(const f_quaternion_t& src, std::bitset<SerializerBase::f_quat_sz>* dest);
  void deserialize(const std::bitset<SerializerBase::f_quat_sz>& src, f_quaternion_t* dest);
};

/**
 * @brief Serializer class for a double quaternion, i.e. std::array<double, 4>.
 */
template<>
class Serializer<d_quaternion_t, SerializerBase::d_quat_sz> {
  void serialize(const d_quaternion_t& src, std::bitset<SerializerBase::d_quat_sz>* dest);
  void deserialize(const std::bitset<SerializerBase::d_quat_sz>& src, d_quaternion_t* dest);
};

/**
 * @brief Serializer class for a GPS time object.
 */
template<>
class Serializer<gps_time_t, SerializerBase::gps_time_sz> {
  void serialize(const gps_time_t& src, std::bitset<SerializerBase::gps_time_sz>* dest);
  void deserialize(const std::bitset<SerializerBase::gps_time_sz>& src, gps_time_t* dest);
};

/**
 * @brief Serializer class for a temperature. 
 * This is just a wrapper around a signed integer serializer.
 */
template<>
class Serializer<temperature_t, SerializerBase::temp_sz> {
  void serialize(const temperature_t& src, std::bitset<SerializerBase::temp_sz>* dest);
  void deserialize(const std::bitset<SerializerBase::temp_sz>& src, temperature_t* dest);
};

template<typename T, size_t compressed_sz> 
Serializer<T, compressed_sz>::Serializer(T min, T max) : 
    _min(min),
    _max(max) {}

template<size_t compressed_sz>
void Serializer<unsigned int, compressed_sz>::serialize(
        const unsigned int& src,
        std::bitset<compressed_sz>* dest) {

}

template<size_t compressed_sz> 
void Serializer<unsigned int, compressed_sz>::deserialize(
        const std::bitset<compressed_sz>& src,
        unsigned int* dest) {
    
}

template<size_t compressed_sz>
void Serializer<float, compressed_sz>::serialize(
        const float& src, 
        std::bitset<compressed_sz>* dest) {

}

template<size_t compressed_sz> 
void Serializer<float, compressed_sz>::deserialize(
        const std::bitset<compressed_sz>& src, 
        float* dest) {
    
}

template<size_t compressed_sz>
void Serializer<double, compressed_sz>::serialize(
        const double& src, 
        std::bitset<compressed_sz>* dest) {

}

template<size_t compressed_sz> 
void Serializer<double, compressed_sz>::deserialize(
    const std::bitset<compressed_sz>& src, 
    double* dest) {
    
}

template<size_t compressed_sz>
void Serializer<f_vector_t, compressed_sz>::serialize(
        const f_vector_t& src, 
        std::bitset<compressed_sz>* dest) {
    static_assert(compressed_sz > SerializerBase::f_vec_min_sz, 
        "Attempting to store serialized vector value into a bitset that is too small.");
}

template<size_t compressed_sz> 
void Serializer<f_vector_t, compressed_sz>::deserialize(
        const std::bitset<compressed_sz>& src, 
        f_vector_t* dest) {
    static_assert(compressed_sz > SerializerBase::f_vec_min_sz, 
        "Attempting to store serialized vector value into a bitset that is too small.");
}

template<size_t compressed_sz>
void Serializer<d_vector_t, compressed_sz>::serialize(
        const d_vector_t& src, 
        std::bitset<compressed_sz>* dest) {
    static_assert(compressed_sz > SerializerBase::d_vec_min_sz, 
        "Attempting to store serialized vector value into a bitset that is too small.");

}

template<size_t compressed_sz> 
void Serializer<d_vector_t, compressed_sz>::deserialize(
        const std::bitset<compressed_sz>& src, 
        d_vector_t* dest) {
    static_assert(compressed_sz > SerializerBase::d_vec_min_sz, 
        "Attempting to store serialized vector value into a bitset that is too small.");

    constexpr size_t magnitude_bitsize = compressed_sz - SerializerBase::d_vec_min_sz;
    std::bitset<magnitude_bitsize> magnitude_packed;
    for(int i = 0; i < magnitude_bitsize; i++) magnitude_packed.set(i, src[i]);
    double magnitude = expand_double(magnitude_packed, this->_min, this->_max);

    unsigned int missing_component = (src[magnitude_bitsize] << 1) + src[magnitude_bitsize+1];
    (*dest)[missing_component] = 1;
    int j = 0; // Index of current component being processed
    for(int i = 0; i < 3; i++) {
        if (i != missing_component) {
            std::bitset<SerializerBase::d_vec_component_sz> vec_component_packed;
            for(int k = 0; k < SerializerBase::d_vec_component_sz; k++) 
                vec_component_packed.set(k, 
                    src[compressed_sz-SerializerBase::d_vec_min_sz+2+j*SerializerBase::d_vec_component_sz+k]);
            Serializer<double, SerializerBase::d_vec_component_sz> element_deserializer(-sqrt(2.0), sqrt(2.0));
            element_deserializer.deserialize(vec_component_packed, &((*dest)[i]));
            (*dest)[missing_component] -= pow((*dest)[i],2);
            j++;
        }
    }
    (*dest)[missing_component] = sqrt((*dest)[missing_component]);

    for(int i = 0; i < 3; i++)
        (*dest)[i] *= magnitude;
}

void Serializer<bool, SerializerBase::bool_sz>::serialize(
        const bool& src, 
        std::bitset<SerializerBase::bool_sz>* dest) {
    (*dest)[0] = src;
}

void Serializer<bool, SerializerBase::bool_sz>::deserialize(
        const std::bitset<SerializerBase::bool_sz>& src, 
        bool* dest) {
    *dest = src[0];
}

void Serializer<f_quaternion_t, SerializerBase::f_quat_sz>::serialize(
        const f_quaternion_t& src, 
        std::bitset<SerializerBase::f_quat_sz>* dest) {
    std::bitset<SerializerBase::f_quat_element_sz> quat_element_representations[3];
    std::array<float, 4> q_mags; // Magnitudes of elements in quaternion
    for(int i = 0; i < 4; i++) q_mags[i] = std::abs(src[i]);

    int max_element_idx = 0;
    float max_element_value = std::max(std::max(q_mags[0], q_mags[1]), std::max(q_mags[2], q_mags[3]));
    if (q_mags[0] == max_element_value) max_element_idx = 0;
    if (q_mags[1] == max_element_value) max_element_idx = 1;
    if (q_mags[2] == max_element_value) max_element_idx = 2;
    if (q_mags[3] == max_element_value) max_element_idx = 3;
    int quat_number = 0; // The current compressed vector bitset that we're modifying
    for(int i = 0; i < 4; i++) {
        if (i == max_element_idx) {
            std::bitset<2> largest_element_representation(i);
            dest->set(0, largest_element_representation[0]);
            dest->set(1, largest_element_representation[1]);
        }
        else {
            Serializer<float, SerializerBase::f_quat_element_sz> element_serializer(-sqrtf(2.0f), sqrtf(2.0f));
            element_serializer.serialize(src[quat_number], &quat_element_representations[quat_number]);
            quat_number++;
        }
    }
    for(int i = 0; i < SerializerBase::f_quat_element_sz; i++) {
        dest->set(i+2, quat_element_representations[0][i]);
        dest->set(i+11, quat_element_representations[1][i]);
        dest->set(i+20, quat_element_representations[2][i]);
    }
}

void Serializer<f_quaternion_t, SerializerBase::f_quat_sz>::deserialize(
        const std::bitset<SerializerBase::f_quat_sz>& src, 
        f_quaternion_t* dest) {
    int missing_element = (src[0] << 1) + src[1];
    (*dest)[missing_element] = 1;
    int j = 0; // Currently processed packed quaternion element
    for(int i = 0; i < 4; i++) {
        if (i != missing_element) {
            std::bitset<SerializerBase::f_quat_element_sz> quat_element_packed;
            for(int k = 0; k < SerializerBase::f_quat_element_sz; k++) 
                quat_element_packed.set(k, src[2 + j*SerializerBase::f_quat_element_sz + k]);
            Serializer<float, SerializerBase::f_quat_element_sz> element_deserializer(
                                                                    -sqrtf(2.0f), sqrtf(2.0f));
            element_deserializer.deserialize(quat_element_packed, &(*dest)[i]);
            (*dest)[missing_element] -= pow((*dest)[i], 2);
        }
    }
    (*dest)[missing_element] = sqrt((*dest)[missing_element]);
}

void Serializer<d_quaternion_t, SerializerBase::d_quat_sz>::serialize(
        const d_quaternion_t& src, 
        std::bitset<SerializerBase::d_quat_sz>* dest) {
    std::bitset<SerializerBase::d_quat_element_sz> quat_element_representations[3];
    std::array<double, 4> q_mags; // Magnitudes of elements in quaternion
    for(int i = 0; i < 4; i++) q_mags[i] = std::abs(src[i]);

    int max_element_idx = 0;
    double max_element_value = std::max(std::max(q_mags[0], q_mags[1]), std::max(q_mags[2], q_mags[3]));
    if (q_mags[0] == max_element_value) max_element_idx = 0;
    if (q_mags[1] == max_element_value) max_element_idx = 1;
    if (q_mags[2] == max_element_value) max_element_idx = 2;
    if (q_mags[3] == max_element_value) max_element_idx = 3;
    int quat_number = 0; // The current compressed vector bitset that we're modifying
    for(int i = 0; i < 4; i++) {
        if (i == max_element_idx) {
            std::bitset<2> largest_element_representation(i);
            dest->set(0, largest_element_representation[0]);
            dest->set(1, largest_element_representation[1]);
        }
        else {
            Serializer<double, SerializerBase::d_quat_element_sz> element_serializer(-sqrt(2), sqrt(2));
            element_serializer.serialize(src[quat_number], &quat_element_representations[quat_number]);
            quat_number++;
        }
    }
    for(int i = 0; i < SerializerBase::d_quat_element_sz; i++) {
        dest->set(i+2, quat_element_representations[0][i]);
        dest->set(i+11, quat_element_representations[1][i]);
        dest->set(i+20, quat_element_representations[2][i]);
    }
}

void Serializer<d_quaternion_t, SerializerBase::d_quat_sz>::deserialize(
        const std::bitset<SerializerBase::d_quat_sz>& src, 
        d_quaternion_t* dest) {
    int missing_element = (src[0] << 1) + src[1];
    (*dest)[missing_element] = 1;
    int j = 0; // Currently processed packed quaternion element
    for(int i = 0; i < 4; i++) {
        if (i != missing_element) {
            std::bitset<SerializerBase::d_quat_element_sz> quat_element_packed;
            for(int k = 0; k < SerializerBase::d_quat_element_sz; k++) 
                quat_element_packed.set(k, src[2 + j*SerializerBase::d_quat_element_sz + k]);
            Serializer<double, SerializerBase::d_quat_element_sz> element_deserializer(-sqrt(2), sqrt(2));
            element_deserializer.deserialize(quat_element_packed, &(*dest)[i]);
            (*dest)[missing_element] -= pow((*dest)[i], 2);
        }
    }
    (*dest)[missing_element] = sqrt((*dest)[missing_element]);
}

void Serializer<gps_time_t, SerializerBase::gps_time_sz>::serialize(
        const gps_time_t& src, 
        std::bitset<SerializerBase::gps_time_sz>* dest) {
    if (src.is_not_set) {
        dest->set(0, false);
        return;
    }
    dest->set(0, true);
    std::bitset<16> wn((unsigned short int) src.gpstime.wn);
    std::bitset<32> tow(src.gpstime.tow);
    for(int i = 0; i < 16; i++)
        dest->set(i + 1, wn[i]);
    for(int i = 0; i < 32; i++)
        dest->set(i + 17, tow[i]);
}

void Serializer<gps_time_t, SerializerBase::gps_time_sz>::deserialize(
        const std::bitset<SerializerBase::gps_time_sz>& src,
        gps_time_t* dest) {
    std::bitset<16> wn;
    std::bitset<32> tow;
    for(int i = 0; i < 16; i++)
        wn.set(i + 1, src[i]);
    for(int i = 0; i < 32; i++)
        tow.set(i + 1, src[16+i]);
    dest->gpstime.wn = (unsigned int) wn.to_ulong();
    dest->gpstime.tow = (unsigned int) tow.to_ulong();
}

void Serializer<temperature_t, SerializerBase::temp_sz>::serialize(
        const temperature_t& src, 
        std::bitset<SerializerBase::temp_sz>* dest) {

}

void Serializer<temperature_t, SerializerBase::temp_sz>::deserialize(
        const std::bitset<SerializerBase::temp_sz>& src, 
        temperature_t* dest) {
 
}

#endif