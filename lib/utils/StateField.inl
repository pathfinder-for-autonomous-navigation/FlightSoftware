/**
 * @file StateField.inl
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#include <StateField.hpp>

template<typename T>
inline StateField<T>::StateField(mutex_t* l, bool gr, bool gw) :
    _lock(l),
    _ground_readable(gr),
    _ground_writable(gw) {}

template<typename T>
inline void StateField<T>::add_reader(thread_t* reader) { _allowed_readers.add(reader); }

template<typename T>
inline void StateField<T>::add_writer(thread_t* writer) { _allowed_writers.add(writer); }

template<typename T>
inline T StateField<T>::get() { 
    rwMtxRLock(_lock);
    T retval = _val;
    rwMtxRUnlock(_lock);
    return retval;
}

template<typename T>
inline void StateField<T>::set(const T& t) {
    rwMtxWLock(_lock);
    _val = t;
    rwMtxWUnlock(_lock);
}

template<typename T, unsigned int compressed_size>
inline SerializableStateField<T, compressed_size>::SerializableStateField(
        mutex_t* l, 
        bool gw,
        Serializer<T, compressed_size>& s) : StateField(l, true, gw), _serializer(s) {}

template<typename T, unsigned int compressed_size>
inline void SerializableStateField<T, compressed_size>::serialize(std::bitset<compressed_size>* dest) {
    rwMtxRLock(_lock);
    _serializer.serialize(_val, dest);
    rwMtxRUnlock(_lock);
}

template<typename T, unsigned int compressed_size>
inline void SerializableStateField<T, compressed_size>::deserialize(std::bitset<compressed_size>& src) {
    rwMtxWLock(_lock);
    _serializer.deserialize(src, &_val);
    rwMtxWUnlock(_lock);
}

template<typename T, unsigned int compressed_size>
inline WritableStateField::WritableStateField(mutex_t* l,
                        Serializer<T, compressed_size>& s) :
    SerializableStateField(l, true, s) {}