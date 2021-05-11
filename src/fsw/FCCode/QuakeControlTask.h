#pragma once
#include "Drivers/QLocate.hpp"
#include "radio_state_t.enum"

/**
 * QLocate driver states
 * QLocate is initialized to IDLE in setup()
 * Operations are split into command sequences since many operations require
 * multiple commands. Also, we want to account for differences in timing
 * between sending commands and reading expected responses.
 */

class QuakeControlTask 
{
public:
  #ifndef DESKTOP
  QuakeControlTask() :
      quake("Quake", &Serial3, Devices::QLocate::DEFAULT_TIMEOUT),
      fnSeqNum(0),
      MO_msg_p(nullptr),
      MO_msg_len(0){
        quake.setup();
      }
  #else
  QuakeControlTask() :
      quake("Quake"),
      fnSeqNum(0),
      MO_msg_p(nullptr),
      MO_msg_len(0) {}
  #endif
  /** 
   * The radio state is set by the Quake Manager. There are 4 relevant
   * states: write, read, transcieve, and config. The control task executes
   * the appropriate command based on the Quake state.
   * Calling execute() when the state is wait generates no effects.
  */
  int execute(radio_state_t state);

  /** Returns the current function number*/
  size_t get_fn_num() const;

  /**
   * Set the message that Quake should downlink.  */
  void set_downlink_msg(char *, size_t);

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

  /**
   * Short burst data: Write binary data
   * This command is used to transfer a binary SBD message from 
   * the DTE (Data Terminal Equipment) to the single mobile 
   * originated buffer (downlink, mobile being the satellite)
   */
  int dispatch_sbdwb();

  /**
   * Short burst data: Read binary data
   * This command is used to transfer a binary SBD message from the 
   * single mobile terminated buffer (uplink)
   */
  int dispatch_sbdrb();

  /**
   * This command initiates an SBD session between the Quake and the GSS (Gateway SBD Subsystem).
   * If there is a message in the mobile originated buffer (downlink) it will be 
   * transferred to the GSS. Similarly if there is one or more MT messages (uplinks) 
   * queued at the GSS the oldest will be transferred to the Quake 
   * and placed into the mobile terminated buffer
   */
  int dispatch_sbdix();

  /**
   * Handles errors in communication
   */
  int dispatch_config();

private:
  // Quake should never be in both IDLE and fnSeqNum != 0
  Devices::QLocate quake;

  /**
   * Whenever we execute a command (sbdix, sbdrb, sbdwb), we have to call
   * some functions in a specific order: query_[command]_[X]() (there may be 
   * more than 1 query command, hence the X) and then get_[command](). The 
   * fnSeqNum signifies which step in this sequence we are on.
   */
  int fnSeqNum;

  char *MO_msg_p;    // pointer to MO buffer (downlink)
  size_t MO_msg_len; // length of the message to downlink (should always be packet_size)

  friend class QuakeManager;
};