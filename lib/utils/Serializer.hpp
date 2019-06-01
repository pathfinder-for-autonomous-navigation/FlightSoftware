/**
 * @file Serializer.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_

#include "GPSTime.hpp"
#include "types.hpp"

template<typename T, unsigned int compressed_size>
class Serializer {
  protected:
    double _min;
    double _max;
  public:
    Serializer(double min, double max);
    void serialize(T& src, std::bitset<compressed_size>* dest);
    void deserialize(std::bitset<compressed_size>& src, T* dest);
};

template<unsigned int compressed_size> class Serializer<int, compressed_size>;
template<unsigned int compressed_size> class Serializer<unsigned int, compressed_size>;
template<unsigned int compressed_size> class Serializer<float, compressed_size>;
template<unsigned int compressed_size> class Serializer<double, compressed_size>;
template<unsigned int compressed_size> class Serializer<f_vector_t, compressed_size>;
template<unsigned int compressed_size> class Serializer<d_vector_t, compressed_size>;

#define GPS_TIME_COMPRESSED_SIZE 29 // TODO
#define F_QUATERNION_COMPRESSED_SIZE 29 // TODO
#define D_QUATERNION_COMPRESSED_SIZE 38 // TODO
#define TEMPERATURE_COMPRESSED_SIZE 38 // TODO
class Serializer<f_quaternion_t, F_QUATERNION_COMPRESSED_SIZE>;
class Serializer<d_quaternion_t, D_QUATERNION_COMPRESSED_SIZE>;
class Serializer<gps_time_t, GPS_TIME_COMPRESSED_SIZE>;
class Serializer<temperature_t, TEMPERATURE_COMPRESSED_SIZE>;

#endif