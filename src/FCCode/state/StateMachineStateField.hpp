#ifndef STATE_MACHINE_STATE_FIELD_HPP_
#define STATE_MACHINE_STATE_FIELD_HPP_

#include <cmath>
#include <string>
#include <utility>
#include "Serializer.hpp"
#include "StateField.hpp"

inline constexpr unsigned int compressed_state_size(unsigned int num_states) {
  return (unsigned int) ceil(log(static_cast<float>(num_states)) / log(2.0f));
}

template<unsigned int num_states>
class SMStateSerializer : public Serializer<unsigned int, compressed_state_size(num_states)> {
  public:
    SMStateSerializer();
};

template<unsigned int num_states>
SMStateSerializer<num_states>::SMStateSerializer() : Serializer(0, num_states - 1) {}

class SMStateFieldBase {};

template<unsigned int num_states>
class SMStateField : public WritableStateField<unsigned int, compressed_state_size(num_states)>, SMStateFieldBase {
  public:
    enum class state_t;

    /**
     * @brief Construct a new State Machine State Field object
     * 
     * Parameters inherited from WritableStateField, but there are no fetchers,
     * so the optional parameter values are set to their defaults mandatorily.
     */
    SMStateField(std::string& name,
      rwmutex_t* l,
      StateFieldRegistry& reg,
      SMStateSerializer<num_states>& s,
      debug_console& dbg_console);
    
    std::array<std::string, num_states> _state_names;
}

template<unsigned int num_states>
SMStateField<num_states>::SMStateField(
        std::string& name,
        rwmutex_t* l,
        StateFieldRegistry& reg,
        SMStateSerializer<num_states>& s,
        debug_console& dbg_console) :
    WritableStateField(name, l, reg, s, dbg_console),
    SMStateFieldBase() {}

#endif