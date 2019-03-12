template<typename T, unsigned int S>
inline void static_buffer<T,S>::advance_ptr_(T*& ptr) {
    ptr++;
    if (ptr == buf_ + sizeof(T)*S) ptr = buf_;
}

template<typename T, unsigned int S>
inline void static_buffer<T,S>::decrement_ptr_(T*& ptr) {
    ptr--;
    if (ptr == buf_ - sizeof(T)) ptr = buf_ + sizeof(T)*(S - 1);
}

template<typename T, unsigned int S>
inline static_buffer<T,S>::static_buffer() {}

template<typename T, unsigned int S>
inline void static_buffer<T,S>::put(T item) {
    *head_ = item;
    if(full_) advance_ptr_(tail_);
    advance_ptr_(head_);
    full_ = head_ == tail_;
}

template<typename T, unsigned int S>
inline T static_buffer<T,S>::operator[](unsigned int i) {
    T* ptr = tail_;
    for(int j = 0; j < i; j++) {
        advance_ptr_(ptr);
    }
    return *ptr;
}

template<typename T, unsigned int S>
inline void static_buffer<T,S>::reset() {
    head_ = tail_;
    full_ = false;
}

template<typename T, unsigned int S>
inline bool static_buffer<T,S>::empty() const {
    //if head and tail are equal, we are empty
    return (!full_ && (head_ == tail_));
}

template<typename T, unsigned int S>
inline bool static_buffer<T,S>::full() const {
    //If tail is ahead the head by 1, we are full
    return full_;
}

template<typename T, unsigned int S>
inline constexpr unsigned int static_buffer<T,S>::size() {
    return S;
}