template<typename T, unsigned int S>
inline T& circular_buffer<T,S>::get() {
    if(static_buffer<T,S>::empty()) return *(new T);
    //Read data and advance the tail (we now have a free space)
    auto val = static_buffer<T,S>::tail_;
    static_buffer<T,S>::full_ = false;
    static_buffer<T,S>::advance_ptr_(static_buffer<T,S>::tail_);
    return *val;
}