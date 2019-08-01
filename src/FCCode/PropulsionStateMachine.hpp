/**
 * @file PropulsionStateMachine.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-06-28
 */

#ifndef PROPULSION_STATE_MACHINE_HPP_
#define PROPULSION_STATE_MACHINE_HPP_

#include "StateMachine.hpp"

constexpr size_t num_prop_states = 6;   // TODO move to an autogenerated file
constexpr size_t tank_pressure_sz = 2;  // TODO move to an autogenerated file
constexpr size_t prop_firing_sz = 2;    // TODO move to an autogenerated file

/**
 * @brief Propulsion state machine.
 *
 */
class PropulsionStateMachine : public StateMachine<num_prop_states> {
   private:
    // Prevent multiple copies of this class, and provide a constructor for
    // internal use.
    SINGLETON(PropulsionStateMachine);
    explicit PropulsionStateMachine(StateFieldRegistry &r);

    // TODO add propulsion devices (SpikeAndHold).
    Serializer<temperature_t, temperature_t, SerializerConstants::temp_sz> temperature_serializer;
    Serializer<float, float, tank_pressure_sz> pressure_serializer;
    Serializer<bool, bool, SerializerConstants::bool_sz> intertank_valve_serializer;
    Serializer<gps_time_t, bool, SerializerConstants::gps_time_sz> firing_time_serializer;
    Serializer<f_vector_t, float, prop_firing_sz> firing_vector_serializer;

   public:
    /**
     * @brief Enumeration of available states.
     */
    enum class state_t {
        safed,
        idle,
        wait_for_pressurize,
        pressurizing,
        wait_for_inject,
        injecting,
        num_states
    };

    /**
     * @brief Enumeration of available state transitions.
     */
    enum class state_transition_t {
        safed_to_idle,

        idle_to_safed,
        idle_to_wait_for_pressurize,

        wait_for_pressurize_to_pressurizing,
        wait_for_pressurize_to_idle,
        wait_for_pressurize_to_safed,

        pressurizing_to_wait_for_inject,
        pressurizing_to_idle,
        pressurizing_to_safed,

        wait_for_inject_to_injecting,
        wait_for_inject_to_idle,
        wait_for_inject_to_safed,

        injecting_to_idle,
        num_transitions,
    };

    /**
     * @brief Names of available states for this state machine. These
     * are passed to the state variable upon initialization.
     */
    static const std::array<std::string, static_cast<unsigned int>(state_t::num_states)>
        state_names;
    static const std::array<std::string,
                            static_cast<unsigned int>(state_transition_t::num_transitions)>
        transition_names;

    /**
     * @brief Constants for use within the sanity check functions.
     */
    static constexpr temperature_t max_sanity_inner_tank_temperature = 20;
    static constexpr temperature_t max_sanity_outer_tank_temperature = 20;
    static constexpr float max_serialization_pressure = 20.0f;
    static constexpr float max_sanity_pressure = 20.0f;
    static constexpr float max_impulse = 20.0f;

    bool init(unsigned int initial_state);

    static std::unique_ptr<PropulsionStateMachine> create(StateFieldRegistry &r);

   protected:
    // Base classes for state machine handlers. Specialized versions defined in
    // PropulsionStateMachine.inl
    template <state_t state>
    class PropulsionStateHandler : public StateHandler {};
    template <state_transition_t transition>
    class PropulsionTransitionHandler : public TransitionHandler {};

    ReadableStateField<temperature_t, temperature_t, SerializerConstants::temp_sz>
        tank_inner_temperature;
    ReadableStateField<temperature_t, temperature_t, SerializerConstants::temp_sz>
        tank_outer_temperature;
    ReadableStateField<float, float, tank_pressure_sz> tank_pressure;
    WritableStateField<bool, bool, SerializerConstants::bool_sz> intertank_valve;
    WritableStateField<gps_time_t, bool, SerializerConstants::gps_time_sz> firing_time;
    WritableStateField<f_vector_t, float, prop_firing_sz> firing_vector;
};

#include "PropulsionStateMachine.inl"

#endif
