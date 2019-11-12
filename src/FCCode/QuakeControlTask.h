#pragma once

#include <TimedControlTask.hpp>
#include "../../lib/Drivers/QLocate.hpp"

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

class QuakeControlTask : public TimedControlTask<int>
{
public:
  #ifndef DESKTOP
  QuakeControlTask(StateFieldRegistry &registry, unsigned int offset) :
      TimedControlTask<int>(registry, offset),
      quake("Quake", &Serial3,Devices::QLocate::DEFAULT_NR_PIN, Devices::QLocate::DEFAULT_TIMEOUT),
      currentState(IDLE),
      fnSeqNum(0),
      szMsg(nullptr),
      len(0){}
  #else
  QuakeControlTask(StateFieldRegistry &registry, unsigned int offset) : 
      TimedControlTask<int>(registry, offset),
      quake(),
      currentState(IDLE),
      fnSeqNum(0),
      szMsg(nullptr),
      len(0) {}
  #endif
  /** 
   * execute is overriden from ControlTask 
   * Calling execute() when the state is IDLE generates no effects. 
  */
  int execute();

  /**
   * Request to change the state of the driver. 
   * This may only be done if the current state is IDLE or if the requested_state
   * is CONFIG.
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

private:
  // all dispatch_x functions return 0 on success and an error code otherwise
  int dispatch_sbdwb();
  int dispatch_sbdrb();
  int dispatch_sbdix();
  int dispatch_config();
  int dispatch_is_functional();
  Devices::QLocate quake;
  // Quake should never be in both IDLE and fnSeqNum != 0
  int currentState; // the state of the Quake
  int fnSeqNum;     // the sequence we are on

  const char *szMsg; // the message to downlink
  size_t len;        // length of the message to downlink
};