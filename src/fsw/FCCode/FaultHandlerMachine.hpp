#ifndef FAULT_HANDLER_MACHINE_HPP_
#define FAULT_HANDLER_MACHINE_HPP_

#include "TimedControlTask.hpp"
#include "mission_state_t.enum"
#include <common/Fault.hpp>

/**
 * @brief Response structure for fault handler state machines.
 */
struct fault_response_t {
  bool recommend_safehold;
  bool recommend_standby;

  bool operator==(const fault_response_t& other) const {
    return this->recommend_safehold == other.recommend_safehold &&
      this->recommend_standby == other.recommend_standby;
  }
};

// Convenient names for the possible fault response values
constexpr fault_response_t no_fault_response = {false, false};
constexpr fault_response_t standby_fault_response = {false, true};
constexpr fault_response_t safehold_fault_response = {true, false};

/**
 * @brief Abstract class for fault handler state machines, which must
 * return a recommended mission state from their execute function.
 */
class FaultHandlerMachine : public ControlTask<fault_response_t> {
  public:
    FaultHandlerMachine(StateFieldRegistry& r) : ControlTask<fault_response_t>(r) {}
    virtual fault_response_t execute() = 0;
};

#endif
