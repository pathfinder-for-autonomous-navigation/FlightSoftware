/** @file comms_utils.inl
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Implementation of utilities for compressing and 
 * decompressing data for downlinking and processing uplinks.
 */
#include <AttitudeMath.hpp>
#include <iostream>

template<size_t max_size>
inline void Comms::trim_float(float f, float min, float max, std::bitset<max_size>* result) {
    if(f > max) f = max;
    if(f < min) f = min;
    float resolution = (max - min) / pow(2, max_size);
    unsigned int result_int = (unsigned int) ((f - min) / resolution);
    std::bitset<max_size> result_copy(result_int);
    *result = result_copy;
}

template<size_t max_size>
inline float Comms::expand_float(const std::bitset<max_size>& f, float min, float max) {
    unsigned long f_bits = f.to_ullong();
    float resolution = (max - min) / pow(2, max_size);
    return min + resolution * f_bits;
}

template<size_t max_size>
inline void Comms::trim_double(double d, double min, double max, std::bitset<max_size>* result) {
    if(d > max) d = max;
    if(d < min) d = min;
    double resolution = (max - min) / pow(2, max_size);
    unsigned int result_int = (unsigned int) ((d - min) / resolution);
    std::bitset<max_size> result_copy(result_int);
    *result = result_copy;
}

template<size_t max_size>
inline float Comms::expand_double(const std::bitset<max_size>& d, double min, double max) {
    unsigned long d_bits = d.to_ullong();
    double resolution = (max - min) / pow(2, max_size);
    return min + resolution * d_bits;
}

template<size_t max_vec_size>
typename std::enable_if<(max_vec_size > Comms::MAX_NORMALIZED_FLOAT_VECTOR_SIZE), void>::type
inline Comms::trim_vector(const std::array<float, 3>& v, float min_magnitude, float max_magnitude, std::bitset<max_vec_size>* result) {
    float mag = vect_mag(v.data());
    constexpr unsigned int magnitude_bitsize = max_vec_size - MAX_NORMALIZED_FLOAT_VECTOR_SIZE;
    std::bitset<magnitude_bitsize> magnitude_representation;
    trim_float(mag, min_magnitude, max_magnitude, &magnitude_representation);

    // Compress unit vector into two compressed floats
    std::bitset<MAX_NORMALIZED_FLOAT_VECTOR_SIZE> vec_representation;
    std::bitset<MAX_FLOAT_VECTOR_COMPONENT_SIZE> vec_element_representations[2];
    std::array<float, 3> v_mags; // Magnitudes of elements in vector
    for(int i = 0; i < 3; i++) v_mags[i] = std::abs(v[i]);
    
    int max_element_idx = 0;
    float max_element_value = std::max(std::max(v_mags[0], v_mags[1]), v_mags[2]);
    if (v_mags[0] == max_element_value) max_element_idx = 0;
    if (v_mags[1] == max_element_value) max_element_idx = 1;
    if (v_mags[2] == max_element_value) max_element_idx = 2;
    int vec_number = 0; // The current compressed vector bitset that we're modifying
    for(int i = 0; i < 3; i++) {
        if (i == max_element_idx) {
            std::bitset<2> largest_element_representation(i);
            vec_representation.set(0, largest_element_representation[0]);
            vec_representation.set(1, largest_element_representation[1]);
        }
        else {
            float v_element_scaled = v[vec_number] / mag;
            trim_float(v_element_scaled, -sqrt(2), sqrt(2), &vec_element_representations[vec_number]);
            vec_number++;
        }
    }
    // Add floats to final bit representation
    for(int i = 0; i < MAX_FLOAT_VECTOR_COMPONENT_SIZE; i++) {
        vec_representation.set(i+2, vec_element_representations[0][i]);
        vec_representation.set(i+11, vec_element_representations[1][i]);
    }
    for(unsigned int i = 0; i < magnitude_bitsize; i++)
        (*result).set(i, magnitude_representation[i]);
    for(int i = 0; i < MAX_NORMALIZED_FLOAT_VECTOR_SIZE; i++)
        (*result).set(i + magnitude_bitsize, vec_representation[i]);
}

template<size_t max_vec_size>
typename std::enable_if<(max_vec_size > Comms::MAX_NORMALIZED_DOUBLE_VECTOR_SIZE), void>::type
inline Comms::trim_vector(const std::array<double, 3>& v, double min_magnitude, double max_magnitude, std::bitset<max_vec_size>* result) {
    double mag = sqrt(pow(v[0],2)+pow(v[1],2)+pow(v[2],2));
    constexpr unsigned int magnitude_bitsize = max_vec_size - MAX_NORMALIZED_DOUBLE_VECTOR_SIZE;
    std::bitset<magnitude_bitsize> magnitude_representation;
    trim_double(mag, min_magnitude, max_magnitude, &magnitude_representation);

    // Compress unit vector into two compressed doubles
    std::bitset<MAX_NORMALIZED_DOUBLE_VECTOR_SIZE> vec_representation;
    std::bitset<MAX_DOUBLE_VECTOR_COMPONENT_SIZE> vec_element_representations[2];
    std::array<double, 3> v_mags; // Magnitudes of elements in vector
    for(int i = 0; i < 3; i++) v_mags[i] = std::abs(v[i]);
    
    int max_element_idx = 0;
    double max_element_value = std::max(std::max(v_mags[0], v_mags[1]), v_mags[2]);
    if (v_mags[0] == max_element_value) max_element_idx = 0;
    if (v_mags[1] == max_element_value) max_element_idx = 1;
    if (v_mags[2] == max_element_value) max_element_idx = 2;
    int vec_number = 0; // The current compressed vector bitset that we're modifying
    for(int i = 0; i < 3; i++) {
        if (i == max_element_idx) {
            std::bitset<2> largest_element_representation(i);
            vec_representation.set(0, largest_element_representation[0]);
            vec_representation.set(1, largest_element_representation[1]);
        }
        else {
            double v_element_scaled = v[vec_number] / mag;
            trim_double(v_element_scaled, -sqrt(2), sqrt(2), &vec_element_representations[vec_number]);
            vec_number++;
        }
    }
    // Add doubles to final bit representation
    for(int i = 0; i < MAX_FLOAT_VECTOR_COMPONENT_SIZE; i++) {
        vec_representation.set(i+2, vec_element_representations[0][i]);
        vec_representation.set(i+11, vec_element_representations[1][i]);
    }
    for(unsigned int i = 0; i < magnitude_bitsize; i++)
        (*result).set(i, magnitude_representation[i]);
    for(int i = 0; i < MAX_NORMALIZED_DOUBLE_VECTOR_SIZE; i++)
        (*result).set(i + magnitude_bitsize, vec_representation[i]);
}

template<size_t max_vec_size>
typename std::enable_if<(max_vec_size > Comms::MAX_NORMALIZED_FLOAT_VECTOR_SIZE), void>::type
inline Comms::trim_vector(const std::array<float, 3>& v, float max_magnitude, std::bitset<max_vec_size>* result) {
    trim_vector(v, 0, max_magnitude, result);
}

template<size_t max_vec_size>
typename std::enable_if<(max_vec_size > Comms::MAX_NORMALIZED_DOUBLE_VECTOR_SIZE), void>::type
inline Comms::trim_vector(const std::array<double, 3>& v, double max_magnitude, std::bitset<max_vec_size>* result) {
    trim_vector(v, 0, max_magnitude, result);
}

template<size_t max_vec_size>
typename std::enable_if<(max_vec_size > Comms::MAX_NORMALIZED_FLOAT_VECTOR_SIZE), void>::type
inline Comms::expand_vector(const std::bitset<max_vec_size>& v, float min_magnitude, float max_magnitude, std::array<float, 3>* result) {
    constexpr unsigned int magnitude_bitsize = max_vec_size - MAX_NORMALIZED_FLOAT_VECTOR_SIZE;
    std::bitset<magnitude_bitsize> magnitude_packed;
    for(int i = 0; i < magnitude_bitsize; i++) magnitude_packed.set(i, v[i]);
    float magnitude = expand_float(magnitude_packed, min_magnitude, max_magnitude);

    unsigned int missing_component = (v[magnitude_bitsize] << 1) + v[magnitude_bitsize+1];
    (*result)[missing_component] = 1;
    int j = 0; // Index of current component being processed
    for(int i = 0; i < 3; i++) {
        if (i != missing_component) {
            std::bitset<MAX_FLOAT_VECTOR_COMPONENT_SIZE> vec_component_packed;
            for(int k = 0; k < MAX_FLOAT_VECTOR_COMPONENT_SIZE; k++) vec_component_packed.set(k, v[max_vec_size-MAX_NORMALIZED_FLOAT_VECTOR_SIZE+2+j*MAX_FLOAT_VECTOR_COMPONENT_SIZE+k]);
            (*result)[i] = expand_float(vec_component_packed, -sqrt(2), sqrt(2));
            (*result)[missing_component] -= pow((*result)[i],2);
            j++;
        }
    }
    (*result)[missing_component] = sqrt((*result)[missing_component]);

    for(int i = 0; i < 3; i++)
        (*result)[i] *= magnitude;
}

template<size_t max_vec_size>
typename std::enable_if<(max_vec_size > Comms::MAX_NORMALIZED_DOUBLE_VECTOR_SIZE), void>::type
inline Comms::expand_vector(const std::bitset<max_vec_size>& v, double min_magnitude, double max_magnitude, std::array<double, 3>* result) {
    constexpr unsigned int magnitude_bitsize = max_vec_size - MAX_NORMALIZED_DOUBLE_VECTOR_SIZE;
    std::bitset<magnitude_bitsize> magnitude_packed;
    for(int i = 0; i < magnitude_bitsize; i++) magnitude_packed.set(i, v[i]);
    double magnitude = expand_double(magnitude_packed, min_magnitude, max_magnitude);

    unsigned int missing_component = (v[magnitude_bitsize] << 1) + v[magnitude_bitsize+1];
    (*result)[missing_component] = 1;
    int j = 0; // Index of current component being processed
    for(int i = 0; i < 3; i++) {
        if (i != missing_component) {
            std::bitset<MAX_DOUBLE_VECTOR_COMPONENT_SIZE> vec_component_packed;
            for(int k = 0; k < MAX_DOUBLE_VECTOR_COMPONENT_SIZE; k++) vec_component_packed.set(k, v[max_vec_size-MAX_NORMALIZED_DOUBLE_VECTOR_SIZE+2+j*MAX_FLOAT_VECTOR_COMPONENT_SIZE+k]);
            (*result)[i] = expand_double(vec_component_packed, -sqrt(2), sqrt(2));
            (*result)[missing_component] -= pow((*result)[i],2);
            j++;
        }
    }
    (*result)[missing_component] = sqrt((*result)[missing_component]);

    for(int i = 0; i < 3; i++)
        (*result)[i] *= magnitude;
}

template<size_t max_vec_size>
typename std::enable_if<(max_vec_size > Comms::MAX_NORMALIZED_FLOAT_VECTOR_SIZE), void>::type
inline Comms::expand_vector(const std::bitset<max_vec_size>& v, float max_magnitude, std::array<float, 3>* result) {
    expand_vector(v, 0, max_magnitude, result);
}

template<size_t max_vec_size>
typename std::enable_if<(max_vec_size > Comms::MAX_NORMALIZED_DOUBLE_VECTOR_SIZE), void>::type
inline Comms::expand_vector(const std::bitset<max_vec_size>& v, double max_magnitude, std::array<double, 3>* result) {
    expand_vector(v, 0, max_magnitude, result);
}

template<unsigned int max_int_size>
inline void Comms::trim_int(int i, int min, int max, std::bitset<max_int_size>* result) {
    if (i > max) i = max;
    if (i < min) i = min;
    unsigned int resolution = (unsigned int) lround(ceil((max - min) / pow(2.0f, max_int_size)));
    unsigned int result_int = (i - min) / ((int) resolution);
    std::bitset<max_int_size> result_copy(result_int);
    *result = result_copy;
}

template<unsigned int max_int_size>
inline int Comms::expand_int(const std::bitset<max_int_size>& result, int min, int max) {
    unsigned int resolution = (unsigned int) lround(ceil((max - min) / pow(2.0f, max_int_size)));
    return min + result.to_ulong() * resolution;
}

template<unsigned int bitset_size>
inline void Comms::expand_message(const std::bitset<bitset_size>& bitset, QuakeMessage* message) {
    unsigned int byte_length = (unsigned int) ceilf((0.0f + bitset.size()) / 8);
    message->length = byte_length;
    char* mes = message->mes;
    for(int i = 0; i < byte_length; i++) {
        std::bitset<8> byte_repr;
        for(int j = 0; j < 8; j++) byte_repr.set(j, bitset[i*8 + j]);
        unsigned char byte_char = (unsigned char) byte_repr.to_ulong();
        mes[i] = byte_char;
    }
}

template<unsigned int bitset_size>
inline void Comms::trim_message(const QuakeMessage& message, std::bitset<bitset_size>* bitset) {
    unsigned int byte_length = (unsigned int) ceilf((0.0f + bitset->size()) / 8);
    for(int i = 0; i < byte_length; i++) {
        std::bitset<8> byte_repr(message.mes[i]);
        for(int j = 0; j < 8; j++) bitset->set(i*8+j, bitset[j]);
    }
}