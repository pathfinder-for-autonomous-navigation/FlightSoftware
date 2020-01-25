#pragma once

#include "TimedControlTask.hpp"
#include "Drivers/QLocate.hpp"

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

class QuakeControlTask : public ControlTask<int>
{
public:
  #ifndef DESKTOP
  QuakeControlTask(StateFieldRegistry &registry) :
      ControlTask<int>(registry),
      quake("Quake", &Serial3,Devices::QLocate::DEFAULT_NR_PIN, Devices::QLocate::DEFAULT_TIMEOUT),
      currentState(IDLE),
      fnSeqNum(0),
      MO_msg_p(nullptr),
      MO_msg_len(0){
        quake.setup();
      }
  #else
  QuakeControlTask(StateFieldRegistry &registry) : 
      ControlTask<int>(registry),
      quake(),
      currentState(IDLE),
      fnSeqNum(0),
      MO_msg_p(nullptr),
      MO_msg_len(0) {}
  #endif
  /** 
   * execute is overriden from ControlTask 
   * Calling execute() when the state is IDLE generates no effects. 
  */
  int execute();

  /**
   * Request to change the state of the driver. 
   * This may only be done if the current state is IDLE or if the requested_state
   * is CONFIG (or IDLE). 
   * Returns true if the state is succsfully changed to the requested_state.
   * Returns false otherwise. 
   */
  bool request_state(int requested_state);

  /** Returns the current state of the Quake */
  int get_current_state() const;

  /** Returns the current function number (for testing purposes) */
  size_t get_current_fn_number() const;

  /**
   * Set the message that Quake should downlink.  */
  void set_downlink_msg(const char *, size_t);

  char* const get_MT_msg()
  {
    return quake.mt_message;
  }

  int get_MO_status()
  {
    return quake.sbdix_r[0];
  }

  int get_MT_status()
  {
    return quake.sbdix_r[2];
  }

  int get_MT_length()
  {
    return quake.sbdix_r[4];
  }


#ifdef DEBUG
  void dbg_set_state(int state) 
  {
    currentState = state;
  }

  void dbg_set_fnSeqNum(int num)
  {
    fnSeqNum = num;
  }

  Devices::QLocate& dbg_get_quake()
  {
    return quake;
  }

  const char* dbg_get_MO_msg()
  {
    return MO_msg_p;
  }

  size_t& dbg_get_MO_len()
  {
    return MO_msg_len;
  }
#endif

protected:
  // all dispatch_x functions return 0 on success and an error code otherwise
  int dispatch_sbdwb();
  int dispatch_sbdrb();
  int dispatch_sbdix();
  int dispatch_config();
  int dispatch_is_functional();

private:
  // Quake should never be in both IDLE and fnSeqNum != 0
  Devices::QLocate quake;

  int currentState; // the state of the Quake
  int fnSeqNum;     // the sequence we are on

  const char *MO_msg_p;   // the message to downlink
  size_t MO_msg_len;      // length of the message to downlink
};