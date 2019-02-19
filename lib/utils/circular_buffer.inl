#include "circular_buffer.hpp"

template<typename T, unsigned int S>
void circular_buffer<T,S>::advance_ptr_(T*& ptr) {
    ptr++;
    if (ptr == buf_ + sizeof(T)*S) ptr = buf_;
}

template<typename T, unsigned int S>
circular_buffer<T,S>::circular_buffer() {}

template<typename T, unsigned int S>
void circular_buffer<T,S>::put(T item) {
    *head_ = item;
    if(full_) advance_ptr_(tail_);
    advance_ptr_(head_);
    full_ = head_ == tail_;
}

template<typename T, unsigned int S>
T& circular_buffer<T,S>::get() {
    if(empty()) return *(new T);
    //Read data and advance the tail (we now have a free space)
    auto val = tail_;
    full_ = false;
    advance_ptr_(tail_);
    return *val;
}

template<typename T, unsigned int S>
void circular_buffer<T,S>::reset() {
    head_ = tail_;
    full_ = false;
}

template<typename T, unsigned int S>
bool circular_buffer<T,S>::empty() const {
    //if head and tail are equal, we are empty
    return (!full_ && (head_ == tail_));
}

template<typename T, unsigned int S>
bool circular_buffer<T,S>::full() const {
    //If tail is ahead the head by 1, we are full
    return full_;
}

template<typename T, unsigned int S>
constexpr unsigned int circular_buffer<T,S>::size() {
    return S;
}