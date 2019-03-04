template<typename T, unsigned int S>
inline T& circular_stack<T,S>::get() {
    if(static_buffer<T,S>::empty()) return *(new T);
    //Read data and decrement the head (we now have a free space)
    auto val = static_buffer<T,S>::head_;
    static_buffer<T,S>::full_ = false;
    static_buffer<T,S>::decrement_ptr_(static_buffer<T,S>::head_);
    return *val;
}