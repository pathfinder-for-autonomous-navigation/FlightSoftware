/** @file helpers.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Defines helper functions for data history collection.
 */

#ifndef DATA_COLLECTION_HELPERS_HPP_
#define DATA_COLLECTION_HELPERS_HPP_

#include <rwmutex.hpp>
#include <array>
#include <static_buffers.hpp>

namespace DataCollection {
    /**
     * @brief Adds a value to a buffer.
     * 
     * @tparam T Type of element being added to the buffer.
     * @tparam S Size of the buffer the element is being added to.
     * @param buf Buffer object to add the element to.
     * @param buf_lock Readers-writers lock that needs to be obtained in order to write to the buffer.
     * @param val_src_lock Readers-writers lock that needs to be obtained in order to read the element's value.
     * @param val The element
     */
    template<typename T, unsigned int S>
    void add_to_buffer(circular_buffer<T,S>& buf, const T& val);

    /**
     * @brief Add an element to a time-averaged value.
     * 
     * @param avg Current average.
     * @param val Value to add to average.
     * @param val_src_lock Readers-writers lock that needs to be obtained in order to read the element's value.
     * @param num_measurements Number of measurements that have been used in the average.
     */
    void add_to_avg(std::array<float, 3>& avg, 
                    const std::array<float, 3>& val,
                    unsigned int* num_measurements);

    /**
     * @brief Sets an average to zero.
     * 
     * @tparam size Size of the array that contains the average.
     * @param avg Pointer to the current average. All of the elements of the average will be set to zero.
     * @param num_measurements Pointer to the integer containing the number of measurements in the average. This field will be
     * set to zero.
     */
    template<unsigned int size>
    void reset_avg(std::array<float, size>* avg, unsigned int* num_measurements);
}

#include "helpers.inl"

#endif