#ifndef CIRCULAR_BUFFER_HPP_
#define CIRCULAR_BUFFER_HPP_

#include <static_buffer.hpp>

// TODO test

/** @brief An statically-allocated circular buffer data structure. 
 * Useful for downlink data storage since it prevents heap fragmentation whilst providing queue functionality. 
**/
template<typename T, unsigned int S>
class circular_buffer : public static_buffer<T,S> {
  public:
    /**
     * @brief Pops off the first item in the circular buffer.
     * The size of the buffer is automatically reduced, enabling the insertion of more elements.
     * @return T& A reference to the first object in the circular buffer.
     */
  	T& get() override;
};

#include "circular_buffer.inl"

#endif