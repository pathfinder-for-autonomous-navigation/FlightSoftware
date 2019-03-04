#ifndef CIRCULAR_STACK_HPP_
#define CIRCULAR_STACK_HPP_

#include "static_buffer.hpp"

/** @brief An statically-allocated circular stack data structure. 
 * Useful for downlink data storage since it prevents heap fragmentation whilst providing stack functionality. 
**/
template<typename T, unsigned int S>
class circular_stack : public static_buffer<T,S> {
  public:
    /**
     * @brief Pops off the first item in the circular stack.
     * The size of the stack is automatically reduced, enabling the insertion of more elements.
     * @return T& A reference to the first object in the circular stack.
     */
  	T& get() override;
};

#include "circular_stack.inl"

#endif