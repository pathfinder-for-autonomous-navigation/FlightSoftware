#pragma once

#include <ControlTask.hpp>
//#include "../../lib/Drivers/QLocate.hpp"
//#include <"../../lib/Drivers/Piksi.hpp">
#include <Piksi.hpp>
//include "Piksi.hpp"
#include <string>

/**
 * QLocate driver states
 * QLocate is initialized to IDLE in setup()
 * Operations are split into command sequences since many operations require
 * multiple commands. Also, we want to account for differences in timing
 * between sending commands and reading expected responses.
 */
static constexpr int SUCCESS = 0;

static constexpr int BAD_BUFFER = 1;
static constexpr int BAD_TIME_OR_NSATS = 2;
static constexpr int BAD_NSATS = 3;
static constexpr int BAD_DATA = 4;

static constexpr int MSG_LEN_WRONG = 5;;

static constexpr int SBDIX = 3;         // SBDIX operation
static constexpr int CONFIG = 4;        // Config operation
static constexpr int IS_FUNCTIONAL = 5; // Is_Functional operation

class PiksiControlTask : public ControlTask<void>
{
public:
// TODO: decide how we want to initialize the Driver
#ifndef DESKTOP
  // PiksiControlTask(StateFieldRegistry &registry) : ControlTask<int>(registry), piksi("piksi"){}

  PiksiControlTask(StateFieldRegistry &registry) : ControlTask<int>(registry),
                                                   piksi("Piksi", Serial4) {}
  //  currentState(IDLE),
  //  fnSeqNum(0),
  //  szMsg(nullptr),
  //  len(0){}
#else
  PiksiControlTask(StateFieldRegistry &registry);
  Devices::Piksi piksi;
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
  void execute();

  // /**
  //  * Request to change the state of the driver.
  //  * This may only be done if the current state is IDLE or if the requested_state
  //  * is CONFIG.
  //  * Returns true if the state is succsfully changed to the requested_state.
  //  * Returns false otherwise.
  //  */
  // bool request_state(int requested_state);

  /** Returns the current state of the Piksi */
  int get_current_state() const;

  // /** Returns the current function number (for testing purposes) */
  // size_t get_current_fn_number() const;

  // /**
  //  * Set the message that Piksi should downlink.  */
  // void set_downlink_msg(const char *, size_t);
protected:
  #ifndef DESKTOP
  Devices::Piksi piksi;
  #endif

  Serializer<d_vector_t> pos_sr;
  ReadableStateField<d_vector_t> pos_f;

  Serializer<d_vector_t> vel_sr;
  ReadableStateField<d_vector_t> vel_f;

  Serializer<d_vector_t> baseline_pos_sr;
  ReadableStateField<d_vector_t> baseline_pos_f;

  Serializer<int> currentState_sr;
  ReadableStateField<int> currentState_f;

  std::array<double, 3> pos;
  std::array<double, 3> vel;
  std::array<double, 3> baseline_pos;
  msg_gps_time_t time;

  unsigned int tow_past = 0;
  unsigned int iar;

  unsigned int pos_past;
  unsigned int pos_tow;

  unsigned int vel_past;
  unsigned int vel_tow;

  unsigned int baseline_past;
  unsigned int baseline_tow;

private:
  // all dispatch_x functions return 0 on success and an error code otherwise
  int get_values();
  // int dispatch_sbdwb();
  // int dispatch_sbdrb();
  // int dispatch_sbdix();
  // int dispatch_config();
  // int dispatch_is_functional();
  //piksi
  //Piksi piksi;

  // Piksi should never be in both IDLE and fnSeqNum != 0
  int currentState; // the state of the Piksi
  // int fnSeqNum;     // the sequence we are on

  // const char *szMsg; // the message to downlink
  // size_t len;        // length of the message to downlink
};