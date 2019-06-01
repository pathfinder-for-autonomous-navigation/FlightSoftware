/**
 * @file StateField.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef STATE_FIELD_HPP_
#define STATE_FIELD_HPP_

#include <Serializer.hpp>
#include <Deserializer.hpp>
#include <vector>
#include <bitset>
#include <ChRt.h>
#include <rwmutex.hpp>

/**
 * @brief A lightweight container around state fields that allows thread-safe access
 * and serialization utilities.
 * 
 * @tparam T Type of the state field.
 * @tparam compressed_size Size of the state field when being sent in a downlink or uplink packet.
 */
template<typename T>
class StateField {
  protected:
    std::vector<thread_t*> _allowed_writers;
    std::vector<thread_t*> _allowed_readers;
    bool _ground_readable;
    bool _ground_writable;
    T _val;
    rwmutex_t* _lock;
  public:
    StateField(mutex_t* l, 
               bool gr, 
               bool gw);

    void add_writer(thread_t* w);
    void add_reader(thread_t* r);

    T get();
    void set(const T& t);
};

template<typename T, unsigned int compressed_size>
class SerializableStateField : public StateField {
  protected:
    Serializer<T, compressed_size>& _serializer;
  public:
    SerializableStateField(mutex_t* l,
               bool gw, 
               Serializer<T, compressed_size>& s);
    
    void serialize(std::bitset<compressed_size>* dest);
    void deserialize(std::bitset<compressed_size>& src);
}

template<typename T, unsigned int compressed_size>
class WritableStateField : public SerializableStateField {
  public:
    WritableStateField(mutex_t* l, Serializer<T, compressed_size>& s);
};

#include <StateField.inl>

#endif