/** @file comms_utils.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb MAX_NORMALIZED_FLOAT_VECTOR_SIZE18
 * @brief Header file defining utilities for compressing and 
 * decompressing data for downlinking and processing uplinks.
 */

#ifndef DOWNLINK_UTILS_HPP_
#define DOWNLINK_UTILS_HPP_

#include <bitset>
#include <array>
#include <FastCRC/FastCRC.h>
#include <cmath>
#include <GPSTime.hpp>
#include <Piksi/Piksi.hpp>
#include <QLocate/QLocate.hpp>

/** 
 * @brief Directive used to produce a compiler warning if a particular
 * integer supplied to a templated function is not large enough. In this case,
 * we use this directive to check if the size of supplied bitsets is large enough
 * for storing a vector into them.
 * */
#define REQUIRES(...) typename std::enable_if<(__VA_ARGS__), int>::type = 0

/** \addtogroup comms_utils Communication Utilities
 * Functions for compressing and decompressing data for downlinking and processing uplinks.
 * **/
/** @{ */
/** \namespace Comms
 * Contains functions and constants related to communication packet serialization and deserialization.
 * */
namespace Comms {
    //! Size of uplink packets, in bytes
    constexpr unsigned int UPLINK_PACKET_SIZE_BYTES = 34; 
    //! Size of uplink packets, in bits
    constexpr unsigned int UPLINK_PACKET_SIZE_BITS = UPLINK_PACKET_SIZE_BYTES * 8; 
    //! Number of bits for squeezing GPS time
    constexpr unsigned int GPSTIME_SIZE = 49;
    //! Number of bits to squeeze a number in the range +/- sqrt(2) for float vector compression
    constexpr unsigned int MAX_FLOAT_VECTOR_COMPONENT_SIZE = 9;
    //! Number of bits for squeezing a quaternion
    constexpr unsigned int QUATERNION_SIZE = MAX_FLOAT_VECTOR_COMPONENT_SIZE * 3 + 2;
    //! Number of bits for squeezing a normalized float vector
    constexpr unsigned int MAX_NORMALIZED_FLOAT_VECTOR_SIZE = MAX_FLOAT_VECTOR_COMPONENT_SIZE * 2 + 2;
    //! Number of bits to squeeze a number in the range +/- sqrt(2) for double vector compression
    constexpr unsigned int MAX_DOUBLE_VECTOR_COMPONENT_SIZE = 18;
    //! Number of bits for squeezing a normalized float vector
    constexpr unsigned int MAX_NORMALIZED_DOUBLE_VECTOR_SIZE = MAX_DOUBLE_VECTOR_COMPONENT_SIZE * 2 + 2;

    /** @brief Create CRC32 checksum at     the end of a packet and copy
     *  it into a destination character buffer.
     *  @param[in] packet Packet to find checksum. This field is modified by the function.
     *  @param[in] packet_ptr Pointer to last bit of data within the packet. This field is modified by the function.
     *  @param[out] dest Destination character buffer into which to write the checksummed packet.
     *  @return The value of the computed CRC32 checksum.
     * **/
    template<unsigned int packet_bit_size>
    unsigned int add_packet_checksum(std::bitset<packet_bit_size>& packet,
                                    unsigned int& packet_ptr,
                                    std::array<char, (unsigned int) ceilf(packet_bit_size / 8)>* dest);

    /** @brief Compress a float into a bitstring.
     *  @param[in] f Float to compress.
     *  @param[in] min Minimum possible value of float.
     *  @param[in] max Maximum possible value of float.
     *  @param[out] result Destination bitstring for storing the compressed float.
     * **/
    template<unsigned int max_size>
    void trim_float(float f, float min, float max, std::bitset<max_size>* result);

    /** @brief Expand a compressed float into its true value.
     *  @param[in] f Bitstring containing the compressed float.
     *  @param[in] min Minimum possible value of float.
     *  @param[in] max Maximum possible value of float.
     *  @return Value of float.
     * **/
    template<unsigned int max_size>
    float expand_float(const std::bitset<max_size>& f, float min, float max);

    /** @brief Compress a double into a bitstring.
     *  @param[in] d Double to compress.
     *  @param[in] min Minimum possible value of double.
     *  @param[in] max Maximum possible value of double.
     *  @param[out] result Destination bitstring for storing the compressed double.
     * **/
    template<unsigned int max_size>
    void trim_double(double d, double min, double max, std::bitset<max_size>* result);

    /** @brief Expand a compressed double into its true value.
     *  @param[in] d Bitstring containing the compressed double.
     *  @param[in] min Minimum possible value of double.
     *  @param[in] max Maximum possible value of double.
     *  @return Value of double.
     * **/
    template<unsigned int max_size>
    float expand_double(const std::bitset<max_size>& d, double min, double max);

    /** @brief Compress a 3-component vector of floats into a bitstring.
     *  @param[in] v Vector to compress.
     *  @param[in] min_magnitude Minimum possible magnitude of vector. May be a negative number.
     *  @param[in] max_magnitude Maximum possible magnitude of vector. May be a negative number.
     *  @param[out] result Destination bitstring for storing the compressed vector.
     * **/
    template<unsigned int max_vec_size, REQUIRES(max_vec_size > MAX_NORMALIZED_FLOAT_VECTOR_SIZE)>
    void trim_vector(const std::array<float, 3>& v, float min_magnitude, float max_magnitude, std::bitset<max_vec_size>* result);

    /** @brief Compress a 3-component vector of doubles into a bitstring.
     *  @param[in] v Vector to compress.
     *  @param[in] min_magnitude Minimum possible magnitude of vector. May be a negative number.
     *  @param[in] max_magnitude Maximum possible magnitude of vector. May be a negative number.
     *  @param[out] result Destination bitstring for storing the compressed vector.
     * **/
    template<unsigned int max_vec_size, REQUIRES(max_vec_size > MAX_NORMALIZED_DOUBLE_VECTOR_SIZE)>
    void trim_vector(const std::array<double, 3>& v, double min_magnitude, double max_magnitude, std::bitset<max_vec_size>* result);

    /** @brief Compress a 3-component vector of floats into a bitstring.
     *  @param[in] v Vector to compress.
     *  @param[in] max_magnitude Maximum possible magnitude of vector. May be a negative number. (The minimum is assumed to be zero).
     *  @param[out] result Destination bitstring for storing the compressed vector.
     * **/
    template<unsigned int max_vec_size, REQUIRES(max_vec_size > MAX_NORMALIZED_FLOAT_VECTOR_SIZE)>
    void trim_vector(const std::array<float, 3>& v, float max_magnitude, std::bitset<max_vec_size>* result);

    /** @brief Compress a 3-component vector of doubles into a bitstring.
     *  @param[in] v Vector to compress.
     *  @param[in] max_magnitude Maximum possible magnitude of vector. May be a negative number. (The minimum is assumed to be zero).
     *  @param[out] result Destination bitstring for storing the compressed vector.
     * **/
    template<unsigned int max_vec_size, REQUIRES(max_vec_size > MAX_NORMALIZED_DOUBLE_VECTOR_SIZE)>
    void trim_vector(const std::array<double, 3>& v, double max_magnitude, std::bitset<max_vec_size>* result);

    /** @brief Expand a compressed vector of floats into its true value.
     *  @param[in] v Bitstring containing the compressed vector.
     *  @param[in] min_magnitude Minimum possible magnitude of vector.
     *  @param[in] max_magnitude Maximum possible magnitude of vector.
     *  @param[out] result Float array into which the uncompressed vector should be stored.
     * **/
    template<unsigned int max_vec_size, REQUIRES(max_vec_size > MAX_NORMALIZED_FLOAT_VECTOR_SIZE)>
    void expand_vector(const std::bitset<max_vec_size>& v, float min_magnitude, float max_magnitude, std::array<float, 3>* result);

    /** @brief Expand a compressed vector of doubles into its true value.
     *  @param[in] v Bitstring containing the compressed vector.
     *  @param[in] min_magnitude Minimum possible magnitude of vector.
     *  @param[in] max_magnitude Maximum possible magnitude of vector.
     *  @param[out] result Double array into which the uncompressed vector should be stored.
     * **/
    template<unsigned int max_vec_size, REQUIRES(max_vec_size > MAX_NORMALIZED_DOUBLE_VECTOR_SIZE)>
    void expand_vector(const std::bitset<max_vec_size>& v, double min_magnitude, double max_magnitude, std::array<double, 3>* result);
    
    /** @brief Expand a compressed vector of floats into its true value.
     *  @param[in] v Bitstring containing the compressed vector.
     *  @param[in] max_magnitude Maximum possible magnitude of vector.
     *  @param[out] result Float array into which the uncompressed vector should be stored.
     * **/
    template<unsigned int max_vec_size, REQUIRES(max_vec_size > MAX_NORMALIZED_FLOAT_VECTOR_SIZE)>
    void expand_vector(const std::bitset<max_vec_size>& v, float max_magnitude, std::array<float, 3>* result);

    /** @brief Expand a compressed vector of doubles into its true value.
     *  @param[in] v Bitstring containing the compressed vector.
     *  @param[in] max_magnitude Maximum possible magnitude of vector.
     *  @param[out] result Double array into which the uncompressed vector should be stored.
     * **/
    template<unsigned int max_vec_size, REQUIRES(max_vec_size > MAX_NORMALIZED_DOUBLE_VECTOR_SIZE)>
    void expand_vector(const std::bitset<max_vec_size>& v, double max_magnitude, std::array<double, 3>* result);

    /** @brief Compress an integer into a smaller representation.
     *  @param[in] i Integer to compress.
     *  @param[in] min Minimum possible value of integer.
     *  @param[in] max Maximum possible value of integer.
     *  @param[out] result Destination bitstring for storing the compressed integer.
     * **/
    template<unsigned int max_int_size>
    void trim_int(int i, int min, int max, std::bitset<max_int_size>* result);

    /** @brief Expand a compressed integer into its true value.
     *  @param[in] result Bitstring containing the compressed integer.
     *  @param[in] min Minimum possible value of float.
     *  @param[in] max Maximum possible value of float.
     *  @return Value of integer.
     * **/
    template<unsigned int max_int_size>
    int expand_int(const std::bitset<max_int_size>& result, int min, int max);

    /** @brief Compress a temperature into a bitstring.
     *  @param[in] t Temperature to compress.
     *  @param[out] result Destination bitstring for storing the compressed temperature.
     * **/
    void trim_temperature(int t, std::bitset<9>* result);
    
    /** @brief Expand a compressed temperature into its true value.
     *  @param[in] t Compressed bitstring containing the temperature.
     *  @return Temperature value, as an integer in Celsius.
     * **/
    int expand_temperature(const std::bitset<9>& t);

    /** @brief Compress a rotation quaternion into a bitstring.
     *  @param[in] q Quaternion to compress.
     *  @param[out] result Destination bitstring for storing the compressed quaternion.
     * **/
    void trim_quaternion(const std::array<float, 4>& q, std::bitset<QUATERNION_SIZE>* result);

    /** @brief Expand a compressed rotation quaternion into its true value.
     *  @param[in] q Bitstring containing the compressed quaternion.
     *  @param[out] result Float array into which the quaternion vector should be stored.
     * **/
    void expand_quaternion(const std::bitset<QUATERNION_SIZE>& q, std::array<float, 4>* result);

    /** @brief Compress a GPS time into a bitstring.
     *  @param[in] gpstime GPS time struct to compress.
     *  @param[out] result Destination bitstring for storing the compressed GPS time.
     * **/
    void trim_gps_time(const gps_time_t& gpstime, std::bitset<GPSTIME_SIZE>* result);

    /** @brief Expand a compressed GPS time into its true value.
     *  @param[in] gpstime Bitstring containing the compressed GPS time.
     *  @param[out] result GPS time struct containing the uncompressed GPS time.
     * **/
    void expand_gps_time(const std::bitset<GPSTIME_SIZE>& gpstime, gps_time_t* result);

    /** @brief Expands bitset into a Quake message
     * */
    template<unsigned int bitset_size>
    void expand_message(const std::bitset<bitset_size>& bitset, Devices::QLocate::Message* message);

    /** @brief Converts Quake message into a bitset
     * */
    template<unsigned int bitset_size>
    void trim_bitset(const Devices::QLocate::Message& message, std::bitset<bitset_size>* bitset);
}
/** @} */
#include "comms_utils.inl"

#endif