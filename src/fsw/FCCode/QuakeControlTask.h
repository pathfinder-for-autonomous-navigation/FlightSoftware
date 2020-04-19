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
   *
   * Calling execute() when the state is wait generates no effects.
  */
  int execute(radio_state_t state);

  /** Returns the current function number*/
  size_t get_fn_num() const;

  /**
   * Set the message that Quake should downlink.  */
  void set_downlink_msg(char *, size_t);

  char* get_MT_msg()
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
  int dispatch_sbdwb();
  int dispatch_sbdrb();
  int dispatch_sbdix();
  int dispatch_config();

private:
  // Quake should never be in both IDLE and fnSeqNum != 0
  Devices::QLocate quake;

  int fnSeqNum;     // the sequence we are on

  char *MO_msg_p;   // data to which MO_msg_p points is not constant
  size_t MO_msg_len; // length of the message to downlink (should always be packet_size)

  friend class QuakeManager;
};