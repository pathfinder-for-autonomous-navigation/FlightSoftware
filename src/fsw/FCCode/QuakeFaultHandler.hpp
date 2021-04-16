#ifndef quake_fault_handler_HPP_
#define quake_fault_handler_HPP_

#include "FaultHandlerMachine.hpp"
#include "qfh_state_t.enum"

/**
 * @brief This state machine class implements a tiered fault system
 * for addressing comms failures. The fault system is as follows:
 * 
 * If Quake radio fails to initiate a successful SBDIX in 24 hours:
 * - Force the mission state to standby, wait 24 hours in that state.
 *   This puts the satellite in an comms-maximizing attitude.
 * - If we still don't have comms by then, power cycle the radio every 8 hours three times.
 * - If we still don't have comms by then, go into safe hold, which after 24 hours
 *   will reset the entire satellite.
 * 
 * NOTE: It's important that this machine does not override conditions
 * that have forced the satellite into safe hold, i.e. it should not
 * cause an autonomous transition from safe hold to standby. This important
 * caveat is managed by the MissionManager in its execute() function by
 * prioritizing transitions to safehold due to power, prop, and ADCS faults
 * over transitions to standby.
 * 
 * TODO unit test the shit out of this mission manager behavior.
 */
class QuakeFaultHandler : public FaultHandlerMachine
{
public:
#ifdef UNIT_TEST
   friend class TestFixtureQFH;
#endif

   /**
     * @brief Construct a new Quake Fault Handler.
     * 
     * @param r State field registry. Needed so that the Quake fault
     * handler can check its indicator fields.
     */
   QuakeFaultHandler(StateFieldRegistry &r);

   /**
     * @brief Runs the fault state machine and returns the suggested
     * mission state.
     * 
     * @return mission_state_t Can either be standby, safe hold, or
     * manual. The mission manager should switch to standby or
     * safe hold if this state machine requires it, but should take
     * no action when manual is returned.
     */
   fault_response_t execute();

protected:
   /**
     * @brief Dispatch functions for the fault handler state machine.
     * 
     * @return mission_state_t 
     */
   fault_response_t dispatch_unfaulted();
   fault_response_t dispatch_forced_standby();
   fault_response_t dispatch_powercycle(qfh_state_t next);
   fault_response_t dispatch_powercycle_1();
   fault_response_t dispatch_powercycle_2();
   fault_response_t dispatch_powercycle_3();
   fault_response_t dispatch_safehold();

   /**
     * @brief Executes a clean transition between fault states.
     * 
     * In the future, this should also trigger the recording of an event.
     * 
     * @param next_state Next state for transition.
     */
   void transition_to(qfh_state_t next_state);

   // Current state of fault checker DFA, and the control cycle
   // count at which it entered this state.
   WritableStateField<unsigned char> cur_state;
   unsigned int cur_state_entry_ccno = 0;

   /**
     * @brief Indicator fields used by the fault handler.
     */
   ReadableStateField<unsigned char> *radio_state_fp;
   const ReadableStateField<unsigned int> *last_checkin_cycle_fp;
   WritableStateField<bool> *radio_power_cycle_fp;
   // Flag that can be used by HOOTL/HITL to disable/enable fault handling
   WritableStateField<bool> fault_handler_enabled_f;

   /**
     * @brief Helper functions for if-statements, to make them look cleaner.
     */
   bool less_than_one_day_since_successful_comms() const;
   bool in_state_for_more_than_time(const unsigned int time) const;
   bool in_state_for_exact_time(const unsigned int time) const;
   bool radio_is_disabled() const;
   bool radio_is_wait() const;
};

#endif
