#pragma once

#include <ControlTask.hpp>
//#include "../../lib/Drivers/QLocate.hpp"
#include "../../lib/Drivers/Piksi.hpp"
#include <string>

/**
 * QLocate driver states
 * QLocate is initialized to IDLE in setup()
 * Operations are split into command sequences since many operations require
 * multiple commands. Also, we want to account for differences in timing
 * between sending commands and reading expected responses.
 */
static constexpr int IDLE = 0;
static constexpr int SBDWB = 1;         // SBDWB operation
static constexpr int SBDRB = 2;         // SBDRB operation
static constexpr int SBDIX = 3;         // SBDIX operation
static constexpr int CONFIG = 4;        // Config operation
static constexpr int IS_FUNCTIONAL = 5; // Is_Functional operation

class PiksiControlTask : public ControlTask<int>
{
public:
  // TODO: decide how we want to initialize the Driver
  #ifndef DESKTOP
  // PiksiControlTask(StateFieldRegistry &registry) : ControlTask<int>(registry), piksi("piksi"){}

  PiksiControlTask(StateFieldRegistry &registry) : ControlTask<int>(registry),
                                                   piksi("Piksi", Serial4){}
                                                  //  currentState(IDLE),
                                                  //  fnSeqNum(0),
                                                  //  szMsg(nullptr),
                                                  //  len(0){}
  #else
  PiksiControlTask(StateFieldRegistry &registry);

  //PiksiControlTask(StateFieldRegistry &registry) : ControlTask<int>(registry),piksi("Piksi"){}
  
                                                  //  currentState(IDLE),
                                                  //  fnSeqNum(0),
                                                  //  szMsg(nullptr),
                                                  //  len(0) {}
                                                  
  #endif
  /** 
   * execute is overriden from ControlTask 
   * Calling execute() when the state is IDLE generates no effects. 
  */
  int execute();

  // /**
  //  * Request to change the state of the driver. 
  //  * This may only be done if the current state is IDLE or if the requested_state
  //  * is CONFIG.
  //  * Returns true if the state is succsfully changed to the requested_state.
  //  * Returns false otherwise. 
  //  */
  // bool request_state(int requested_state);

  // /** Returns the current state of the Piksi */
  // int get_current_state() const;

  // /** Returns the current function number (for testing purposes) */
  // size_t get_current_fn_number() const;

  // /**
  //  * Set the message that Piksi should downlink.  */
  // void set_downlink_msg(const char *, size_t);

private:
  // all dispatch_x functions return 0 on success and an error code otherwise
  int exec_read_buffer();

  // int dispatch_sbdwb();
  // int dispatch_sbdrb();
  // int dispatch_sbdix();
  // int dispatch_config();
  // int dispatch_is_functional();
  //piksi
  Devices::Piksi piksi;
  //Piksi piksi;
  
  // Piksi should never be in both IDLE and fnSeqNum != 0
  // int currentState; // the state of the Piksi
  // int fnSeqNum;     // the sequence we are on

  // const char *szMsg; // the message to downlink
  // size_t len;        // length of the message to downlink
};