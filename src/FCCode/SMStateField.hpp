#ifndef STATE_MACHINE_STATE_FIELD_HPP_
#define STATE_MACHINE_STATE_FIELD_HPP_

#include <cmath>
#include <string>
#include <utility>
#include "Serializer.hpp"
#include "StateField.hpp"

// Number of bits required to store a state number that has num_states possible states
inline constexpr size_t csz_state(const size_t num_states) {
    for (int i = 0; i < 32; i++) {
        if (pow(2, i) > num_states) return i;
    }
    return 32;
}

/**
 * @brief Specialized serializer for state machine state. Reduces
 * a state number N down to ceil(log2(N)) bits.
 *
 * @tparam num_states
 */
class SMStateSerializer : public Serializer<unsigned int> {
   public:
    SMStateSerializer(size_t num_states)
        : Serializer<unsigned int>(0, num_states - 1, csz_state(num_states)) {}
};

/**
 * @brief A specialization of state field for state machine state.
 *
 * @tparam num_states
 */
class SMStateField : public WritableStateField<unsigned int> {
   public:
    /**
     * @brief Construct a new State Machine State Field object
     */
    SMStateField(const std::string& name, const SMStateSerializer& s)
        : WritableStateField<unsigned int>(name, s), _state_names() {}

    /**
     * @brief Array that maps index number (state) to the name of the state (a
     * string).
     */
    std::vector<std::string> _state_names;
};

#endif