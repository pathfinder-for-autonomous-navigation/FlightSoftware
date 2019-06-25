/**
 * @file StateField.inl
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#include "StateField.hpp"
#include "debug_console.hpp"

template<typename T>
inline T StateField<T>::null_fetcher() {
    set(_val);
}

template<typename T>
inline bool StateField<T>::null_sanity_check(const T& val) const {
    return true;
}

template<typename T>
inline StateField<T>::StateField(const std::string& name,
                                 debug_console& dbg_console,
                                 bool gr, 
                                 bool gw, 
                                 StateFieldRegistry& reg,
                                 typename StateField<T>::fetch_f fetcher,
                                 typename StateField<T>::sanity_check_f checker) :
    DataField(name),
    Debuggable(dbg_console),
    _ground_readable(gr),
    _ground_writable(gw),
    _registry(reg),
    _fetcher(fetcher),
    _checker(checker) {}

template<typename T>
inline std::string& StateField<T>::name() const { return _name; }

template<typename T>
inline void StateField<T>::add_reader(Task& reader) { _registry.add_reader(&reader, this); }

template<typename T>
inline void StateField<T>::add_writer(Task& writer) { _registry.add_writer(&writer, this); }

template<typename T>
inline DataField* StateField<T>::ptr() { return this; }

template<typename T>
inline bool StateField<T>::can_read(Task& getter) {
    return _registry.can_read(getter, *this);
}

template<typename T>
inline bool StateField<T>::can_write(Task& setter) {
    return _registry.can_write(setter, *this);
}

template<typename T>
inline T StateField<T>::get(Task* getter) {
    if (!can_read(getter))
        _dbg_console.printf(debug_severity::ALERT, 
            "Task %s illegally tried to read state field %s.", getter->name().c_str(), this->name().c_str());

    return _val;
}

template<typename T>
inline void StateField<T>::set(Task* setter, const T& t) {
    if (!can_write(setter))
        _dbg_console.printf(debug_severity::ALERT, 
            "Task %s illegally tried to read state field %s.", setter->name().c_str(), this->name().c_str());

    _val = t;
}

template<typename T>
inline void StateField<T>::fetch(Task* setter) {
    set(setter, _fetcher());
}

template<typename T>
inline bool StateField<T>::sanity_check(Task* getter) const {
    return _checker(get(getter));
}

template<typename T>
inline InternalStateField<T>::InternalStateField(const std::string& name, 
                                                 debug_console& dbg_console,
                                                 StateFieldRegistry& reg,
                                                 typename StateField<T>::fetch_f fetcher) : 
                            StateField<T>(name, dbg_console, false, false, reg, fetcher) {}

template<typename T, typename U, unsigned int compressed_sz>
inline SerializableStateField<T, U, compressed_sz>::SerializableStateField(
                                                    const std::string& name,
                                                    debug_console& dbg_console,
                                                    bool gw,
                                                    StateFieldRegistry& reg,
                                                    Serializer<T, U, compressed_sz>& s,
                                                    typename StateField<T>::fetch_f fetcher) : 
        StateField<T>(name, dbg_console, true, gw, reg, fetcher), _serializer(s) {}

template<typename T, typename U, unsigned int compressed_sz>
inline void SerializableStateField<T, U, compressed_sz>::serialize(std::bitset<compressed_sz>* dest) {
    rwMtxRLock((this->_lock));
    (this->_serializer).serialize(this->_val, dest);
    rwMtxRUnlock((this->_lock));
}

template<typename T, typename U, unsigned int compressed_sz>
inline void SerializableStateField<T, U, compressed_sz>::deserialize(const std::bitset<compressed_sz>& src) {
    rwMtxWLock((this->_lock));
    (this->_serializer).deserialize(src, &(this->_val));
    rwMtxWUnlock((this->_lock));
}

template<typename T, typename U, unsigned int compressed_sz>
inline void SerializableStateField<T, U, compressed_sz>::print(std::string* dest) {
    rwMtxWLock((this->_lock));
    (this->_serializer).print(this->_val, dest);
    rwMtxWUnlock((this->_lock));
}

template <typename T, typename U, unsigned int compressed_sz>
inline ReadableStateField<T, U, compressed_sz>::ReadableStateField(const std::string &name,
                                                                   debug_console &dbg_console,
                                                                   StateFieldRegistry &reg,
                                                                   Serializer<T, U, compressed_sz> &s,
                                                                   typename StateField<T>::fetch_f fetcher) : 
    SerializableStateField<T, U, compressed_sz>(name, dbg_console, false, reg, s, fetcher) {}

template<typename T, typename U, unsigned int compressed_sz>
inline WritableStateField<T, U, compressed_sz>::WritableStateField(const std::string& name,
                                                                   debug_console& dbg_console,
                                                                   StateFieldRegistry& reg,
                                                                   Serializer<T, U, compressed_sz>& s,
                                                                   typename StateField<T>::fetch_f fetcher) :
    SerializableStateField<T, U, compressed_sz>(name, dbg_console, true, reg, s, fetcher) {}