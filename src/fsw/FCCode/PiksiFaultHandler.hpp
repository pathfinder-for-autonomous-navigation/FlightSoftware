#ifndef PIKSI_FAULT_HANDLER_HPP_
#define PIKSI_FAULT_HANDLER_HPP_

#include "FaultHandlerMachine.hpp"
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
     * @brief Check if we have recently gotten any GPS readings 
     * and transition to the standby if: 
     *  - The satellite has entered a close approach state but 
     *    has't recieved CDGPS readings in X time (X is a 
     *    ground-configurable wait initially set to 24 hours).
     *  - The satellite had previously recieved CDGPS 
     *    readings during close approach, but then is still in 
     *    a close approach and hasn't received CDGPS in the 
     *    last Y hours (Y is a ground-configurable wait initially 
     *    set to 3 hours).
     * 
     * @return mission_state_t 
     */
    fault_response_t check_cdgps();

    // Statefields used by the fault handler to determine response.
    ReadableStateField<unsigned int>* piksi_state_fp;
    WritableStateField<unsigned char>* mission_state_fp;
    InternalStateField<unsigned int>* last_fix_time_ccno_fp;
    InternalStateField<unsigned int>* enter_close_appr_time_fp;

    // Statefield for X time
    WritableStateField<unsigned int> no_cdgps_max_wait_f;
    // Statefield for Y time
    WritableStateField<unsigned int> cdgps_delay_max_wait_f;

};

#endif
