#ifndef STATIC_BUFFER_HPP_
#define STATIC_BUFFER_HPP_

/** @brief An statically-allocated circular buffer data structure. Can either be configured to be a stack or a queue.
 * Old values are overwritten regardless of whether structure is a stack or queue.
 * Useful for downlink data storage since it prevents heap fragmentation whilst providing buffer functionality. 
 * Functionality credit for most of this is to https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc#c- 
**/
template<typename T, unsigned int S>
class static_buffer {
  protected:
    T buf_[S]; // buffer
    T* head_ = buf_;
    T* tail_ = buf_;
    bool full_ = false;
		// Advances a pointer ahead by one element, wrapping around if necessary.
		void advance_ptr_(T*& ptr);
    // Decrements a pointer by one element, wrapping around if necessary.
    void decrement_ptr_(T*& ptr);
  public:
    /**
     * @brief Construct a new circular buffer object.
     */
    static_buffer();

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
     * The size of the buffer is automatically reduced, enabling the insertion of more elements.
     * @return A reference to the first object in the circular buffer.
     */
  	virtual T& get() = 0;

    /**
     * @brief Array-like read access to the stack. Most recent element is the last element in the list.
     * Wraps around if necessary. Returns a _copy_ of an array element.
     */
  	T operator[](unsigned int i);

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
     * @return Maximum size of buffer.
     */
    constexpr unsigned int size();
};

#include "static_buffer.inl"

#endif