#ifndef CIRCULAR_BUFFER_HPP_
#define CIRCULAR_BUFFER_HPP_

#include <cstdio>

// TODO test

/** @brief An statically-allocated circular buffer data structure. 
 * 
 * Useful for downlink data storage since it prevents heap fragmentation whilst providing queue functionality. 
 * Functionality credit to https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc#c- 
**/
template<typename T, unsigned int S>
class circular_buffer {
  private:
    T buf_[S]; // Buffer
    T* head_ = buf_;
    T* tail_ = buf_;
    bool full_ = false;
		// Advances a pointer ahead by one element, wrapping around if necessary.
		void advance_ptr_(T*& ptr);
  public:
    /**
     * @brief Construct a new circular buffer object.
     */
    circular_buffer();

    /**
     * @brief Puts item into the circular buffer.
     * 
     * The item is copied into the buffer, rather than a pointer being stored. This is since
     * mostly everything in the flight code is statically allocated, and the object being provided
     * likely has a short function duration.
     * 
     * @param item Item to be _copied_ into the buffer.
     */
    void put(T item);

    /**
     * @brief Pops off the first item in the circular buffer.
     * 
     * The size of the buffer is automatically reduced, enabling the insertion of more elements.
     * 
     * @return T& A reference to the first object in the circular buffer.
     */
  	T& get();

    /**
     * @brief Flushes the circular buffer.
     */
    void reset();

    /**
     * @brief Returns whether or not the circular buffer is empty.
     * 
     * @return true If the circular buffer is empty.
     * @return false If the circular buffer is not empty.
     */
    bool empty() const;
    /**
     * @brief Returns whether or not the circular buffer is full.
     * 
     * @return true If the circular buffer is full.
     * @return false If the circular buffer is not full.
     */
		bool full() const;

    /**
     * @brief Returns maximum size of buffer.
     * 
     * @return constexpr unsigned int Maximum size of buffer.
     */
    constexpr unsigned int size();
};

#include "circular_buffer.inl"

#endif