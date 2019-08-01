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
template <size_t num_states>
class SMStateSerializer
    : public Serializer<unsigned int, unsigned int, compressed_state_size(num_states)> {
   public:
    using Serializer<unsigned int, unsigned int, compressed_state_size(num_states)>::Serializer;
    bool init() {
        return Serializer<unsigned int, unsigned int, compressed_state_size(num_states)>::init(
            0, num_states - 1);
    }
};

/**
 * @brief A specialization of state field for state machine state.
 *
 * @tparam num_states
 */
template <size_t num_states>
class SMStateField
    : public WritableStateField<unsigned int, unsigned int, compressed_state_size(num_states)> {
   public:
    /**
     * @brief Construct a new State Machine State Field object
     */
    SMStateField(const std::string &name, StateFieldRegistry &reg)
        : WritableStateField<unsigned int, unsigned int, compressed_state_size(num_states)>(name,
                                                                                            reg),
          _state_names() {}

    void init(const std::array<std::string, num_states> &state_names,
              const std::shared_ptr<SMStateSerializer<num_states>> &s,
              typename StateFieldFunctions<unsigned int>::sanity_check_f checker =
                  StateFieldFunctions<unsigned int>::null_sanity_check) {
        _state_names = state_names;
        WritableStateField<unsigned int, unsigned int, compressed_state_size(num_states)>::init(
            s, nullptr, checker);
    }

    /**
     * @brief Array that maps index number (state) to the name of the state (a
     * string).
     */
    std::array<std::string, num_states> _state_names;
};

#endif