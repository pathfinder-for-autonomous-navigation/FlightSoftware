/**
 * @file StateField.inl
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#include "StateField.hpp"
#include "debug_console.hpp"

template<typename T>
inline T StateFieldFunctions<T>::null_fetcher() {
    static T val;
    return val;
}

template<typename T>
inline bool StateFieldFunctions<T>::null_sanity_check(const T& val) {
    return true;
}

template<typename T>
inline StateField<T>::StateField(const std::string& name,
                                 StateFieldRegistry& reg) : DataField(name), 
                                                            Debuggable(),
                                                            InitializationRequired(),
                                                            _ground_readable(false),
                                                            _ground_writable(false),
                                                            _val(),
                                                            _registry(reg),
                                                            _fetcher(nullptr),
                                                            _checker(nullptr) {}

template<typename T>
inline bool StateField<T>::init(bool gr, 
                                bool gw, 
                                typename StateFieldFunctions<T>::fetch_f fetcher,
                                typename StateFieldFunctions<T>::sanity_check_f checker) {
    _ground_readable = gr;
    _ground_writable = gw;
    _fetcher = fetcher;
    _checker = checker;
    return InitializationRequired::init();
}

template<typename T>
inline bool StateField<T>::add_reader(Task& reader) {
    if (!_is_initialized) return false;
    return _registry.add_reader(&reader, this);
}

template<typename T>
inline bool StateField<T>::add_writer(Task& writer) {
    if (!_is_initialized) return false;
    return _registry.add_writer(&writer, this);
}

template<typename T>
inline DataField* StateField<T>::ptr() {
    if (!_is_initialized) return nullptr;
    return this;
}

template<typename T>
inline bool StateField<T>::can_read(Task& getter) {
    if (!_is_initialized) return false;
    return _registry.can_read(getter, *this);
}

template<typename T>
inline bool StateField<T>::can_write(Task& setter) {
    if (!_is_initialized) return false;
    return _registry.can_write(setter, *this);
}

template<typename T>
inline T StateField<T>::get(Task* getter) {
    if (!_is_initialized) return _val;
    if (!can_read(*getter))
        _dbg_console->printf(debug_severity::ALERT, 
            "Task %s illegally tried to read state field %s.", getter->name().c_str(), this->name().c_str());

    return _val;
}

template<typename T>
inline bool StateField<T>::set(Task* setter, const T& t) {
    if (!_is_initialized) return false;
    if (!can_write(setter))
        _dbg_console->printf(debug_severity::ALERT, 
            "Task %s illegally tried to read state field %s.", setter->name().c_str(), this->name().c_str());
    _val = t;
    return true;
}

template<typename T>
inline bool StateField<T>::fetch(Task* setter) {
    return set(setter, _fetcher());
}

template<typename T>
inline bool StateField<T>::sanity_check(Task* getter) const {
    return _checker(get(getter));
}

template<typename T>
inline bool StateField<T>::is_readable() const {
    return _ground_readable;
}

template<typename T>
inline bool StateField<T>::is_writable() const {
    return _ground_writable;
}

template<typename T>
bool InternalStateField<T>::init(typename StateField<T>::fetch_f fetcher,
                                 typename StateField<T>::sanity_check_f checker) {
    return StateField<T>::init(false, false, fetcher, checker);
}

template<typename T, typename U, unsigned int compressed_sz>
SerializableStateField<T, U, compressed_sz>::SerializableStateField(const std::string& name,
                                                                    StateFieldRegistry& reg) : 
                                                            StateField<T>(name), 
                                                            _serializer(nullptr) { }

template<typename T, typename U, unsigned int compressed_sz>
inline bool SerializableStateField<T, U, compressed_sz>::init(bool gw,
                                                              Serializer<T, U, compressed_sz>* s,
                                                              typename StateFieldFunctions<T>::fetch_f fetcher,
                                                              typename StateFieldFunctions<T>::sanity_check_f checker)
{
    if (!s._is_initialized()) return false;
    _serializer = s;
    return StateField<T>::init(true, gw, fetcher, checker);
}

template<typename T, typename U, unsigned int compressed_sz>
inline bool SerializableStateField<T, U, compressed_sz>::serialize(std::bitset<compressed_sz>* dest) {
    return this->_is_initialized && (this->_serializer).serialize(this->_val, dest);
}

template<typename T, typename U, unsigned int compressed_sz>
inline bool SerializableStateField<T, U, compressed_sz>::deserialize(const std::bitset<compressed_sz>& src) {
    return this->_is_initialized && (this->_serializer).deserialize(src, &(this->_val));
}

template<typename T, typename U, unsigned int compressed_sz>
inline bool SerializableStateField<T, U, compressed_sz>::print(std::string* dest) {
    return this->_is_initialized && (this->_serializer).print(this->_val, dest);
}

template <typename T, typename U, unsigned int compressed_sz>
inline bool ReadableStateField<T, U, compressed_sz>::init(Serializer<T, U, compressed_sz>* s,
                                                          typename StateFieldFunctions<T>::fetch_f fetcher,
                                                          typename StateFieldFunctions<T>::sanity_check_f checker)
{
    return SerializableStateField<T,U,compressed_sz>::init(false, s, fetcher, checker);
}

template<typename T, typename U, unsigned int compressed_sz>
inline bool WritableStateField<T, U, compressed_sz>::init(Serializer<T, U, compressed_sz>* s,
                                                          typename StateFieldFunctions<T>::fetch_f fetcher,
                                                          typename StateFieldFunctions<T>::sanity_check_f checker)
{
    return SerializableStateField<T,U,compressed_sz>::init(true, s, fetcher, checker);                                                  
}