/**
 * @file Serializers.inl
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#include "Serializer.hpp"

template<typename T, unsigned int compressed_size> 
inline Serializer<T, compressed_size>::Serializer(double min, double max) : 
    _min(min),
    _max(max) {}

template<unsigned int compressed_size> 
inline void Serializer<int, compressed_size>::serialize(int& src, std::bitset<compressed_size>* dest) {
    
}

template<unsigned int compressed_size> 
inline void Serializer<int, compressed_size>::deserialize(std::bitset<compressed_size>& src, int* dest) {
    
}

template<unsigned int compressed_size>
inline void Serializer<unsigned int, compressed_size>::serialize(unsigned int& src, std::bitset<compressed_size>* dest) {

}

template<unsigned int compressed_size> 
inline void Serializer<unsigned int, compressed_size>::deserialize(std::bitset<compressed_size>& src, unsigned int* dest) {
    
}

template<unsigned int compressed_size>
inline void Serializer<float, compressed_size>::serialize(float& src, std::bitset<compressed_size>* dest) {

}

template<unsigned int compressed_size> 
inline void Serializer<float, compressed_size>::deserialize(std::bitset<compressed_size>& src, float* dest) {
    
}

template<unsigned int compressed_size>
inline void Serializer<double, compressed_size>::serialize(double& src, std::bitset<compressed_size>* dest) {

}

template<unsigned int compressed_size> 
inline void Serializer<double, compressed_size>::deserialize(std::bitset<compressed_size>& src, double* dest) {
    
}

template<unsigned int compressed_size>
inline void Serializer<f_vector_t, compressed_size>::serialize(f_vector_t& src, std::bitset<compressed_size>* dest) {

}

template<unsigned int compressed_size> 
inline void Serializer<f_vector_t, compressed_size>::deserialize(std::bitset<compressed_size>& src, f_vector_t* dest) {
    
}

template<unsigned int compressed_size>
inline void Serializer<d_vector_t, compressed_size>::serialize(d_vector_t& src, std::bitset<compressed_size>* dest) {

}

template<unsigned int compressed_size> 
inline void Serializer<d_vector_t, compressed_size>::deserialize(std::bitset<compressed_size>& src, d_vector_t* dest) {
    
}

inline void Serializer<f_quaternion_t, F_QUATERNION_COMPRESSED_SIZE>::serialize(f_quaternion_t& src, std::bitset<compressed_size>* dest) {
    std::bitset<9> quat_element_representations[3];
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
            trim_float(src[quat_number], -sqrt(2), sqrt(2),  &quat_element_representations[quat_number]);
            quat_number++;
        }
    }
    for(int i = 0; i < 9; i++) {
        dest->set(i+2, quat_element_representations[0][i]);
        dest->set(i+11, quat_element_representations[1][i]);
        dest->set(i+20, quat_element_representations[2][i]);
    }
}

inline void Serializer<f_quaternion_t, F_QUATERNION_COMPRESSED_SIZE>::deserialize(std::bitset<compressed_size>& src, T* dest) {
    int missing_element = (src[0] << 1) + src[1];
    (*dest)[missing_element] = 1;
    int j = 0; // Currently processed packed quaternion element
    for(int i = 0; i < 4; i++) {
        if (i != missing_element) {
            std::bitset<9> quat_element_packed;
            for(int k = 0; k < 9; k++) quat_element_packed.set(k, src[2 + j*9 + k]);
            (*dest)[i] = expand_float(quat_element_packed, -sqrt(2), sqrt(2));
            (*dest)[missing_element] -= pow((*dest)[i], 2);
        }
    }
    (*dest)[missing_element] = sqrt((*dest)[missing_element]);
}

inline void Serializer<d_quaternion_t, D_QUATERNION_COMPRESSED_SIZE>::serialize(d_quaternion_t& src, std::bitset<compressed_size>* dest) {

}

inline void Serializer<d_quaternion_t, D_QUATERNION_COMPRESSED_SIZE>::deserialize(std::bitset<compressed_size>& src, T* dest) {
    
}

inline void Serializer<gps_time_t, GPS_TIME_COMPRESSED_SIZE>::serialize(gps_time_t& src, std::bitset<compressed_size>* dest) {
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

inline void Serializer<gps_time_t, GPS_TIME_COMPRESSED_SIZE>::deserialize(std::bitset<compressed_size>& src, gps_time_t* dest) {
    
}

inline void Serializer<temperature_t, TEMPERATURE_COMPRESSED_SIZE>::serialize(temperature_t& src, std::bitset<compressed_size>* dest) {

}

inline void Serializer<temperature_t, TEMPERATURE_COMPRESSED_SIZE>::deserialize(std::bitset<compressed_size>& src, temperature_t* dest) {
    
}