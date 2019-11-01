#pragma once

#include <ControlTask.hpp>
#include <QLocate.hpp>
using namespace Devices;

class QuakeControlTask : public ControlTask<int>
{
public:
  // TODO: decide how we want to initialize the Driver
  QuakeControlTask(StateFieldRegistry &registry) : ControlTask<int>(registry),
                                                   currentState(IDLE),
                                                   fnSeqNum(0),
                                                   szMsg(nullptr),
                                                   len(0),
                                                   quake(
                                                       QLocate("QUAKE NAME: change this",
                                                               &Serial3,
                                                               QLocate::DEFAULT_NR_PIN,
                                                               QLocate::DEFAULT_TIMEOUT)) {}
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
  int get_current_state();

  /** Returns the current function number (for testing purposes) */
  size_t get_current_fn_number();

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
  QLocate quake;
  // Quake should never be in both IDLE and fnSeqNum != 0
  int currentState; // the state of the Quake
  int fnSeqNum;     // the sequence we are on

  const char *szMsg; // the message to downlink
  size_t len;        // length of the message to downlink
};