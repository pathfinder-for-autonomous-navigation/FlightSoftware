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
inline StateField<T>::StateField(std::string name, rwmutex_t* l, bool gr, bool gw, StateFieldRegistry& reg,
               debug_console& dbg_console, fetch_f fetcher, mutex_t* f_l) :
    Debuggable(dbg_console),
    _name(name),
    _lock(l),
    _ground_readable(gr),
    _ground_writable(gw),
    _registry(reg),
    _fetcher(fetcher),
    _fetch_lock(f_l) {
    // TODO check if both locks are initialized
}

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

    rwMtxRLock(_lock);
    T retval = _val;
    rwMtxRUnlock(_lock);
    return retval;
}

template<typename T>
inline void StateField<T>::set(Task* setter, const T& t) {
    if (!can_write(setter))
        dbg.printf(debug_severity::ALERT, 
            "Task %s illegally tried to read state field %s.", setter->name().c_str(), this->name().c_str());

    rwMtxWLock(_lock);
    _val = t;
    rwMtxWUnlock(_lock);
}

template<typename T>
inline void StateField<T>::fetch(Task* setter) {
    if (_fetch_lock != NULL) chMtxLock(_fetch_lock);
        T new_val = _fetcher();
    if (_fetch_lock != NULL) chMtxUnlock(_fetch_lock);
    set(setter, new_val);
}

template<typename T>
inline InternalStateField<T>::InternalStateField(std::string name, 
               rwmutex_t* l,
               StateFieldRegistry& reg,
               debug_console& dbg_console,
               fetch_f fetcher,
               mutex_t* f_l) : StateField(name, l, false, false, reg, dbg_console, fetcher, f_l) {}

template<typename T, unsigned int compressed_size>
inline SerializableStateField<T, compressed_size>::SerializableStateField(
        std::string name,
        rwmutex_t* l, 
        bool gw,
        StateFieldRegistry& reg,
        Serializer<T, compressed_size>& s,
        debug_console& dbg_console,
        fetch_f fetcher,
        mutex_t* f_l) : StateField(name, l, true, gw, reg, dbg_console, fetcher, f_l), _serializer(s) {}

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
inline WritableStateField::WritableStateField(std::string name,
                        rwmutex_t* l,
                        StateFieldRegistry& reg,
                        Serializer<T, compressed_size>& s,
                        debug_console& dbg_console,
                        fetch_f fetcher,
                        mutex_t* f_l) :
    SerializableStateField(name, l, true, reg, s, dbg_console, fetcher, f_l) {}