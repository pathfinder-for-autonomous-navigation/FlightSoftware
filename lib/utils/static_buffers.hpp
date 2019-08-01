#ifndef STATIC_BUFFER_HPP_
#define STATIC_BUFFER_HPP_

#include <cstddef>

/** @brief An statically-allocated circular buffer data structure. Can either be
 *configured to be a stack or a queue.
 * Old values are overwritten regardless of whether structure is a stack or
 *queue.
 * Useful for downlink data storage since it prevents heap fragmentation whilst
 *providing buffer functionality.
 * Functionality credit for most of this is to
 *https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc#c-
**/
template <typename T, size_t S>
class static_buffer {
   protected:
    T buf_[S];  // buffer
    T *head_ = buf_;
    T *tail_ = buf_;
    bool full_ = false;
    // Advances a pointer ahead by one element, wrapping around if necessary.
    void advance_ptr_(T *&ptr);
    // Decrements a pointer by one element, wrapping around if necessary.
    void decrement_ptr_(T *&ptr);

   public:
    /**
     * @brief Construct a new circular buffer object.
     */
    static_buffer();

    /**
     * @brief Puts item into the circular buffer.
     *
     * The item is copied into the buffer, rather than a pointer being stored.
     * This is since
     * mostly everything in the flight code is statically allocated, and the
     * object being provided
     * likely has a short function duration.
     *
     * @param item Item to be _copied_ into the buffer.
     */
    void put(T item);

    /**
     * @brief Pops off the first item in the circular buffer.
     * The size of the buffer is automatically reduced, enabling the insertion of
     * more elements.
     * @return A reference to the first object in the circular buffer.
     */
    virtual T &get() = 0;

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
    size_t size() const;
};

/** @brief An statically-allocated circular buffer data structure.
 * Useful for downlink data storage since it prevents heap fragmentation whilst
 *providing queue functionality.
**/
template <typename T, size_t S>
class circular_buffer : public static_buffer<T, S> {
   public:
    /**
     * @brief Pops off the first item in the circular buffer.
     * The size of the buffer is automatically reduced, enabling the insertion of
     * more elements.
     * @return T& A reference to the first object in the circular buffer.
     */
    T &get() override;
};

/** @brief An statically-allocated circular stack data structure.
 * Useful for downlink data storage since it prevents heap fragmentation whilst
 *providing stack functionality.
**/
template <typename T, size_t S>
class circular_stack : public static_buffer<T, S> {
   public:
    /**
     * @brief Pops off the first item in the circular stack.
     * The size of the stack is automatically reduced, enabling the insertion of
     * more elements.
     * @return T& A reference to the first object in the circular stack.
     */
    T &get() override;
};

template <typename T, size_t S>
void static_buffer<T, S>::advance_ptr_(T *&ptr) {
    ptr++;
    if (ptr == buf_ + sizeof(T) * S) ptr = buf_;
}

template <typename T, size_t S>
void static_buffer<T, S>::decrement_ptr_(T *&ptr) {
    if (ptr == buf_) ptr = buf_ + sizeof(T) * S;
    ptr--;
}

template <typename T, size_t S>
static_buffer<T, S>::static_buffer() {}

template <typename T, size_t S>
void static_buffer<T, S>::put(T item) {
    *head_ = item;
    if (full_) advance_ptr_(tail_);
    advance_ptr_(head_);
    full_ = head_ == tail_;
}

template <typename T, size_t S>
void static_buffer<T, S>::reset() {
    head_ = tail_;
    full_ = false;
}

template <typename T, size_t S>
bool static_buffer<T, S>::empty() const {
    // if head and tail are equal, we are empty
    return (!full_ && (head_ == tail_));
}

template <typename T, size_t S>
bool static_buffer<T, S>::full() const {
    // If tail is ahead the head by 1, we are full
    return full_;
}

template <typename T, size_t S>
size_t static_buffer<T, S>::size() const {
    return S;
}

template <typename T, size_t S>
T &circular_buffer<T, S>::get() {
    if (static_buffer<T, S>::empty()) return *(new T);
    // Read data and advance the tail (we now have a free space)
    auto val = static_buffer<T, S>::tail_;
    static_buffer<T, S>::full_ = false;
    static_buffer<T, S>::advance_ptr_(static_buffer<T, S>::tail_);
    return *val;
}

template <typename T, size_t S>
T &circular_stack<T, S>::get() {
    if (static_buffer<T, S>::empty()) return *(new T);
    // Read data and decrement the head (we now have a free space)
    static_buffer<T, S>::decrement_ptr_(static_buffer<T, S>::head_);
    auto val = static_buffer<T, S>::head_;
    static_buffer<T, S>::full_ = false;
    return *val;
}

#endif