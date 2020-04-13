#ifndef PIKSI_FAULT_HANDLER_HPP_
#define PIKSI_FAULT_HANDLER_HPP_

#include "FaultHandlerMachine.hpp"
#include "piksi_fh_state_t.enum"
#include "piksi_mode_t.enum"

class PiksiFaultHandler : public FaultHandlerMachine {
  public:

    /**
     * @brief Construct a new Piksi Fault Handler.
     * 
     * @param r State field registry.
     */
    PiksiFaultHandler(StateFieldRegistry& r);

    /**
     * @brief Runs the fault state machine and recommends 
     * that the mission manager go to standby if required.
     */
    fault_response_t execute();

  protected:

    /**
     * @brief Dispatch functions for the fault handler state machine.
     * 
     * @return mission_state_t 
     */
    fault_response_t dispatch_unfaulted();
    fault_response_t dispatch_check_cdgps();
    fault_response_t no_cdgps();
    fault_response_t cdgps_delay();

    /**
     * @brief Executes a transition between fault states.
     * @param next_state Next state for transition.
     */
    void transition_to(qfh_state_t next_state);

    // Current state of fault checker DFA, and the control cycle
    // count at which it entered this state.
    piksi_fh_state_t current_state = piksi_fh_state_t::unfaulted;
    unsigned int current_state_entry_ccno = 0;


    // Statefields used by the fault handler to determine response.
    ReadableStateField<unsigned int>* piksi_state_fp;
    WritableStateField<unsigned char>* mission_state_fp;
    // Statefield for X time
    // Statefield for Y time
    InternalStateField<sys_time_t>* last_fix_time_fp;

    // Last fix time since entering close approach
    InternalStateField<sys_time_t>* enter_close_appr_time_fp;

};

#endif
