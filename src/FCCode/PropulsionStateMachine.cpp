/**
 * @file PropulsionStateMachine.cpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-06-28
 */

#include "PropulsionStateMachine.hpp"

static bool pressure_sanity_checker(const float &pressure) {
  return (pressure > 0) &&
         (pressure < PropulsionStateMachine::max_sanity_pressure);
}

static bool tank_inner_temp_sanity_checker(const temperature_t &temp) {
  return (temp > 0) &&
         (temp < PropulsionStateMachine::max_sanity_inner_tank_temperature);
}

static bool tank_outer_temp_sanity_checker(const temperature_t &temp) {
  return (temp > 0) &&
         (temp < PropulsionStateMachine::max_sanity_outer_tank_temperature);
}

static bool firing_time_sanity_checker(const gps_time_t &time) {
  // TODO
  return true;
}

static bool firing_vector_sanity_checker(const f_vector_t &vec) {
  // TODO
  return true;
}

static float pressure_fetcher() {
  // TODO
  return 0.0;
}

static temperature_t tank_inner_temp_fetcher() {
  return static_cast<temperature_t>(0);
}

static temperature_t tank_outer_temp_fetcher() {
  return static_cast<temperature_t>(0);
}

const std::array<std::string, static_cast<unsigned int>(
                                  PropulsionStateMachine::state_t::num_states)>
    PropulsionStateMachine::state_names = {"safed",
                                           "idle",
                                           "waiting_for_pressurize",
                                           "pressurizing",
                                           "waiting_for_inject",
                                           "injecting"};

const std::array<
    std::string,
    static_cast<unsigned int>(
        PropulsionStateMachine::state_transition_t::num_transitions)>
    PropulsionStateMachine::transition_names = {
        "safed_to_idle",

        "idle_to_safed",
        "idle_to_wait_for_pressurize",

        "wait_for_pressurize_to_pressurizing",
        "wait_for_pressurize_to_idle",
        "wait_for_pressurize_to_safed",

        "pressurizing_to_wait_for_inject",
        "pressurizing_to_idle",
        "pressurizing_to_safed",

        "wait_for_inject_to_injecting",
        "wait_for_inject_to_idle",
        "wait_for_inject_to_safed",

        "injecting_to_idle",
};

PropulsionStateMachine::PropulsionStateMachine(StateFieldRegistry &r)
    : StateMachine<num_prop_states>("prop.sm", "prop.sm_state", r),
      temperature_serializer(), pressure_serializer(),
      intertank_valve_serializer(), firing_time_serializer(),
      firing_vector_serializer(),
      tank_inner_temperature("prop.inner_tank.temperature", r),
      tank_outer_temperature("prop.outer_tank.temperature", r),
      tank_pressure("prop.outer_tank.pressure", r),
      intertank_valve("prop.intertank_valve", r),
      firing_time("prop.firing.time", r),
      firing_vector("prop.firing.vector", r) {}

std::unique_ptr<PropulsionStateMachine>
PropulsionStateMachine::create(StateFieldRegistry &r) {
  return std::unique_ptr<PropulsionStateMachine>(new PropulsionStateMachine(r));
}

bool PropulsionStateMachine::init(unsigned int initial_state) {
  // Initialize serializers
  AbortIfInitFail(temperature_serializer.init(), false);
  AbortIfInitFail(pressure_serializer.init(0, max_serialization_pressure),
                  false);
  AbortIfInitFail(intertank_valve_serializer.init(), false);
  AbortIfInitFail(firing_time_serializer.init(), false);
  AbortIfInitFail(firing_vector_serializer.init(0, max_impulse), false);

  auto temperature_slzr_ptr = std::shared_ptr<
      Serializer<temperature_t, temperature_t, SerializerConstants::temp_sz>>(
      &temperature_serializer);
  auto tank_pressure_slzr_ptr =
      std::shared_ptr<Serializer<float, float, tank_pressure_sz>>(
          &pressure_serializer);
  auto intertank_valve_slzr_ptr =
      std::shared_ptr<Serializer<bool, bool, SerializerConstants::bool_sz>>(
          &intertank_valve_serializer);
  auto firing_time_slzr_ptr = std::shared_ptr<
      Serializer<gps_time_t, bool, SerializerConstants::gps_time_sz>>(
      &firing_time_serializer);
  auto firing_vector_slzr_ptr =
      std::shared_ptr<Serializer<f_vector_t, float, prop_firing_sz>>(
          &firing_vector_serializer);

  // Initialize state variables
  AbortIfInitFail(tank_inner_temperature.init(temperature_slzr_ptr,
                                              tank_inner_temp_fetcher,
                                              tank_inner_temp_sanity_checker),
                  false);
  AbortIfInitFail(tank_outer_temperature.init(temperature_slzr_ptr,
                                              tank_outer_temp_fetcher,
                                              tank_outer_temp_sanity_checker),
                  false);
  AbortIfInitFail(tank_pressure.init(tank_pressure_slzr_ptr, pressure_fetcher,
                                     pressure_sanity_checker),
                  false);
  AbortIfInitFail(
      intertank_valve.init(intertank_valve_slzr_ptr,
                           StateFieldFunctions<bool>::null_fetcher,
                           StateFieldFunctions<bool>::null_sanity_check),
      false);
  AbortIfInitFail(
      firing_time.init(firing_time_slzr_ptr,
                       StateFieldFunctions<gps_time_t>::null_fetcher,
                       firing_time_sanity_checker),
      false);
  AbortIfInitFail(
      firing_vector.init(firing_vector_slzr_ptr,
                         StateFieldFunctions<f_vector_t>::null_fetcher,
                         firing_vector_sanity_checker),
      false);
  AbortIfInitFail(StateMachine<num_prop_states>::init(
                      state_names, static_cast<unsigned int>(state_t::safed)),
                  false);

  // // Allow this state machine access to its own state variables
  _registry.add_writer(std::shared_ptr<Task>(this),
                       std::make_shared<DataField>(tank_inner_temperature));
  _registry.add_writer(std::shared_ptr<Task>(this),
                       std::make_shared<DataField>(tank_outer_temperature));
  _registry.add_writer(std::shared_ptr<Task>(this),
                       std::make_shared<DataField>(tank_pressure));
  _registry.add_reader(std::shared_ptr<Task>(this),
                       std::make_shared<DataField>(intertank_valve));
  _registry.add_reader(std::shared_ptr<Task>(this),
                       std::make_shared<DataField>(firing_time));
  _registry.add_reader(std::shared_ptr<Task>(this),
                       std::make_shared<DataField>(firing_vector));

// Add state handlers. At the end, make sure every state has a registered
// handler, and no handlers exist for undefined states.
#define RegisterStateHandler(state_name, only_once)                            \
  register_state_handler(                                                      \
      static_cast<unsigned int>(PropulsionStateMachine::state_t::state_name),  \
      std::make_shared<PropulsionStateHandler<state_t::state_name>>(           \
          "prop.state_handler." #state_name, _registry, only_once));

  RegisterStateHandler(safed, false);
  RegisterStateHandler(idle, false);
  RegisterStateHandler(wait_for_pressurize, false);
  RegisterStateHandler(pressurizing, true);
  RegisterStateHandler(wait_for_inject, false);
  RegisterStateHandler(injecting, true);

#undef RegisterStateHandler

  AbortIfNot(_state_handlers.size() == state_names.size(), false);
  AbortIfNot(_state._state_names == state_names, false);
  for (unsigned int i = 0; i < static_cast<unsigned int>(state_t::num_states);
       i++) {
    const std::string state_handler_name =
        "prop.state_handler." + _state._state_names[i];
    AbortIfNot(_state_handlers.at(i)->name() != state_handler_name, false);
  }

// Add state transition handlers. At the end, make sure every transition is
// accounted for (i.e. the transition and handler lists are the same size, and
// no handler is associated with an undefined transition.)
#define RegisterTransitionHandler(state1, state2, transition)                  \
  register_transition_handler(                                                 \
      static_cast<unsigned int>(state_t::state1),                              \
      static_cast<unsigned int>(state_t::state2),                              \
      std::make_shared<                                                        \
          PropulsionTransitionHandler<state_transition_t::transition>>(        \
          "prop.transition_handler." #transition, _registry));

  RegisterTransitionHandler(safed, idle, safed_to_idle);
  RegisterTransitionHandler(idle, safed, idle_to_safed);
  RegisterTransitionHandler(idle, wait_for_pressurize,
                            idle_to_wait_for_pressurize);
  RegisterTransitionHandler(wait_for_pressurize, pressurizing,
                            wait_for_pressurize_to_pressurizing);
  RegisterTransitionHandler(wait_for_pressurize, idle,
                            wait_for_pressurize_to_idle);
  RegisterTransitionHandler(wait_for_pressurize, safed,
                            wait_for_pressurize_to_safed);
  RegisterTransitionHandler(pressurizing, wait_for_inject,
                            pressurizing_to_wait_for_inject);
  RegisterTransitionHandler(pressurizing, idle, pressurizing_to_idle);
  RegisterTransitionHandler(pressurizing, safed, pressurizing_to_safed);
  RegisterTransitionHandler(wait_for_inject, injecting,
                            wait_for_inject_to_injecting);
  RegisterTransitionHandler(wait_for_inject, idle, wait_for_inject_to_idle);
  RegisterTransitionHandler(wait_for_inject, safed, wait_for_inject_to_safed);
  RegisterTransitionHandler(injecting, idle, injecting_to_idle);

#undef RegisterTransitionHandler

  AbortIfNot(_transition_handlers.size() == transition_names.size(), false);
  for (std::map<std::pair<unsigned int, unsigned int>,
                std::shared_ptr<TransitionHandler>>::iterator it =
           _transition_handlers.begin();
       it != _transition_handlers.end(); ++it) {
    const std::string &transition_name = (it->second)->name();
    bool found_in_allowed_transitions = false;
    for (int i = 0; i < transition_names.size(); i++) {
      if (transition_name == transition_names[i]) {
        found_in_allowed_transitions = true;
        break;
      }
    }
    AbortIfNot(found_in_allowed_transitions, false);
  }

  // Give the state and transition handlers access to their required variables
  // TODO

  return true;
}
