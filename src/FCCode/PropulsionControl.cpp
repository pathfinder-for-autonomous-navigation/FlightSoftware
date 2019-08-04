/**
 * @file PropulsionStateMachine.cpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-06-28
 */

#include "PropulsionControl.hpp"

PropulsionControl::PropulsionControl(const std::shared_ptr<StateFieldRegistry> &_registry_ptr,
                      const std::shared_ptr<ReadableStateFieldBase> &_tank_inner_temperature_ptr,
                      const std::shared_ptr<ReadableStateFieldBase> &_tank_outer_temperature_ptr,
                      const std::shared_ptr<ReadableStateFieldBase> &_tank_pressure_ptr,
                      const std::shared_ptr<WritableStateFieldBase> &_firing_time_ptr,
                      const std::shared_ptr<WritableStateFieldBase> &_firing_vector_ptr) : 
      StateMachine<num_prop_states>("prop.sm", "prop.sm_state", _registry),
      tank_inner_temperature_ptr(_tank_inner_temperature_ptr),
      tank_outer_temperature_ptr(_tank_outer_temperature_ptr),
      tank_pressure_ptr(_tank_pressure_ptr),
      firing_time_ptr(_firing_time_ptr),
      firing_vector_ptr(_firing_vector_ptr),
      intertank_valve("prop.intertank_valve") {}

std::unique_ptr<PropulsionControl> PropulsionControl::create(
    const std::shared_ptr<StateFieldRegistry> &_registry_ptr,
    const std::shared_ptr<ReadableStateFieldBase> &_tank_inner_temperature_ptr,
    const std::shared_ptr<ReadableStateFieldBase> &_tank_outer_temperature_ptr,
    const std::shared_ptr<ReadableStateFieldBase> &_tank_pressure_ptr,
    const std::shared_ptr<WritableStateFieldBase> &_firing_time_ptr,
    const std::shared_ptr<WritableStateFieldBase> &_firing_vector_ptr) {
    auto ptr = std::unique_ptr<PropulsionControl>(new PropulsionControl(_registry_ptr, _tank_inner_temperature_ptr,
                                                   _tank_outer_temperature_ptr, _tank_pressure_ptr,
                                                   _firing_time_ptr, _firing_vector_ptr));
    ReturnIfNot(ptr, std::unique_ptr<PropulsionControl>(nullptr));
    ReturnIfInitFail(ptr->init(0), std::unique_ptr<PropulsionControl>(nullptr));

    return ptr;
}

bool PropulsionControl::init(unsigned int initial_state) {
    auto intertank_valve_slzr_ptr =
        std::make_shared<Serializer<bool, bool, SerializerConstants::bool_sz>>();
    auto firing_time_slzr_ptr =
        std::make_shared<Serializer<gps_time_t, bool, SerializerConstants::gps_time_sz>>();
    auto firing_vector_slzr_ptr = std::make_shared<Serializer<f_vector_t, float, prop_firing_sz>>();

    // Initialize serializers
    ReturnIfInitFail(intertank_valve_slzr_ptr->init(), false);
    ReturnIfInitFail(firing_time_slzr_ptr->init(), false);
    ReturnIfInitFail(firing_vector_slzr_ptr->init(0, max_impulse), false);

    // Initialize state variables
    ReturnIfInitFail(intertank_valve.init(intertank_valve_slzr_ptr, StateField<bool>::null_fetcher,
                                          StateField<bool>::null_sanity_check),
                     false);

    // Initialize state machine
    std::array<std::string, num_prop_states> state_names;
    ReturnIfInitFail(init(0), false);

    // // Allow this state machine access to its own state variables
    _registry->add_reader(std::shared_ptr<ControlTaskBase>(this),
                          std::make_shared<StateFieldBase>(intertank_valve));

    // Add state handlers.
    // TODO

    // Add state transition handlers.
    // TODO

    // Give the state and transition handlers access to their required variables
    // TODO

    return true;
}
