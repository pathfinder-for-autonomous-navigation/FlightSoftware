#ifndef STATE_MACHINE_STATE_FIELD_HPP_
#define STATE_MACHINE_STATE_FIELD_HPP_

#include <cmath>
#include <string>
#include <utility>
#include "Serializer.hpp"
#include "StateField.hpp"

inline constexpr size_t compressed_state_size(size_t num_states) {

  return static_cast<size_t>(ceil(log(static_cast<float>(num_states)) / logf(2.0f)));
}

/**
 * @brief Specialized serializer for state machine state. Reduces
 * a state number N down to ceil(log2(N)) bits.
 * 
 * @tparam num_states 
 */
template<size_t num_states>
class SMStateSerializer : public Serializer<unsigned int, unsigned int, compressed_state_size(num_states)> {
  public:
    /**
     * @brief Construct a new SMStateSerializer object.
     * 
     */
    SMStateSerializer();
};

template<size_t num_states>
SMStateSerializer<num_states>::SMStateSerializer() : 
  Serializer<unsigned int, unsigned int, compressed_state_size(num_states)>(static_cast<unsigned int>(0), static_cast<unsigned int>(num_states - 1)) {}

/**
 * @brief A specialization of state field for state machine state.
 * 
 * @tparam num_states 
 */
template<size_t num_states>
class SMStateField : public WritableStateField<unsigned int, unsigned int, compressed_state_size(num_states)> {
  public:
    /**
     * @brief Construct a new State Machine State Field object
     * 
     * Parameters inherited from WritableStateField, but there are no fetchers,
     * so the optional parameter values are set to their defaults mandatorily.
     */
    using WritableStateField<unsigned int, unsigned int, compressed_state_size(num_states)>::WritableStateField;
    virtual void init(StateFieldRegistry &reg,
                                    SMStateSerializer<num_states> &s,
                                    typename StateField<unsigned int>::fetch_f fetcher,
                                    typename StateField<unsigned int>::sanity_check_f checker) 
    {
      WritableStateField<unsigned int, unsigned int, compressed_state_size(num_states)>::init(reg, s, fetcher, checker);
    }

    /**
     * @brief Array that maps index number (state) to the name of the state (a string).
     * 
     */
    std::array<std::string, num_states> _state_names;
};

#endif