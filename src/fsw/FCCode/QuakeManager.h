#pragma once
#include "TimedControlTask.hpp"
#include "QuakeControlTask.h"
#include "radio_state_t.enum"
#include <common/constant_tracker.hpp>

/**
 * QuakeManager decides what commands should be sent to the Quake based on the
 * current control cycle and the results of prior control cycles. 
 * 
 * Dependencies: ClockManager, Downlink Provider
 * 
 * States: config, wait, transceive, read, write.
 * 
 * All methods return true on success, false otherwise. 
 */ 

/**
 * Comms Protocol Implementation:
 *  
 * If we have written the entire snapshot, load the next snapshot
 * Otherwise, increment mo_idx to point to the next 70 blocks
 * 
 * Essentially points mo_buffer_copy + mo_idx*packet_size to the next
 * 70 blocks of data that should be downlinked. 
 */
class QuakeManager : public TimedControlTask<void> {
   public:
    QuakeManager(StateFieldRegistry& registry, unsigned int offset);
    ~QuakeManager();
    void execute() override;

   // protected:
   /**
    * @brief attempts to execute a step in the CONFIG command sequence. This command
    * sequence is executed on startup and executed whenever the unexpected_flag 
    * is set
    */
    void dispatch_config();
   /**
    * @brief unconditionally waits the full amount of its allocated cycles
    * If the unexpected_flag is set, transitions to config instead of 
    * transition to wait write
    */
   void dispatch_wait();
   /**
    * @brief attempts an SBD connection with Iridium.
    */
   void dispatch_transceive();

   void dispatch_disabled();
   /**
    * @brief reads an MT message from the MT buffer
    */
   void dispatch_read();
   /**
    * @brief writes a message to the MO buffer
    */
    void dispatch_write();

    /**
     * Handle case where SBDIX reports no comms
     */
    void handle_no_comms();

    /**
     * Handle case where SBDIX reports that we have comms but no msg from the ground
     */
    void handle_comms_no_msg();

    /**
     * Handle case where SBDIX reports that we have comms and we have msg from the ground
     */
    void handle_comms_msg();

    void copy_next_packet();

    void copy_next_snapshot();

   /**
     * @brief Snapshot size in bytes, provided by DownlinkProducer. 
     */
    const InternalStateField<size_t>* snapshot_size_fp;
    
  /**
   * @brief Pointer to downlink packet, provided by DownlinkProducer.
   */
  const InternalStateField<char*>* radio_mo_packet_fp;

  /**
   * @brief State machine constants that control how long the machine may
   * be in the WAIT and TRANSCEIVE states.
   */
  WritableStateField<unsigned int> max_wait_cycles_f;
  WritableStateField<unsigned int> max_transceive_cycles_f;

    /**
    * @brief Quake Error field.
    **/ 
   ReadableStateField<int> radio_err_f;

   /**
    * @brief Uplink buffer.
    **/ 
   InternalStateField<char*> radio_mt_packet_f;

   /**
    * @brief Uplink length.
    **/ 
   InternalStateField<size_t> radio_mt_len_f; 

   /**
    * @brief Current radio state (see radio_state_t.enum).
    **/
   InternalStateField<unsigned char> radio_state_f;

   /**
    * The last cycle for which we had comms
    */
   InternalStateField<unsigned int> last_checkin_cycle_f;

   /**
    * @brief This flag can be used by the sim to dump telemetry over the USB line.
    */
   WritableStateField<bool> dump_telemetry_f;

  #ifdef DEBUG

  QuakeControlTask& dbg_get_qct()
  {
    return qct;
  }
  size_t& dbg_get_max_snap()
  {
    return max_snapshot_size;
  }

  InternalStateField<unsigned int>& dbg_get_last_checkin()
  {
    return last_checkin_cycle_f;
  }

  int dbg_get_cycle_of_entry()
  {
      return cycle_of_entry;
  }

  char* dbg_get_mo_buffer_copy()
  {
    return mo_buffer_copy;
  }

  size_t& dbg_get_mo_idx()
  {
    return mo_idx;
  }

  bool& dbg_get_unexpected_flag()
  {
    return unexpected_flag;
  }

  bool dbg_transition_radio_state(radio_state_t new_state)
  {
     transition_radio_state(new_state);
     return true;
  }

  #endif

  private:
    QuakeControlTask qct;

    /**
     * True if fnSeqNum = 0 and cycle_of_entry = 0 or 1
     * @return true if we just entered this current state
     */
    bool has_just_entered();

    /**
     * True if fnSeqNum = 0 and cycle_of_entry > 1
     * @return
     */
    bool has_finished();

    /**
     * If the current state has no more cycles left, then return true, else false
     */
    bool no_more_cycles(size_t max_cycles);

    /**
     * Transition the radio into the new state
     * update cycle_of_entry to current cycle
     * Precondition: new_state is one of the defined states
     * Postcondition: radio_state_f == new_state
     */ 
    void transition_radio_state(radio_state_t new_state);

    /**
     *
     * Sets the unexpected_flag to true
     * Writes the error code to radio_err_f
     * Transitions the state to radio_state_t::wait
     * @param err_code
     */
    void handle_err(int err_code);

    /**
     * Returns True if err_code is actually an error (i.e. not Devices::OK and not PORT_UNAVAILABLE)
     * @param err_code
     * @return
     */
    bool is_actual_error(int err_code);

    /**
     * The control cycle at which we transitioned to the current state
     */
    unsigned int cycle_of_entry;

    /**
     * Local copy of max_snapshot_size given by DownlinkProducer
     */
    size_t max_snapshot_size;

    /**
     * Temporary buffer of size max_snapshot_size + 1
     * Only SBDWB may write to mo_buffer_copy or mo_idx
     */
    char* mo_buffer_copy;

    /**
     * The index into mo_buffer_copy in multiples of max_packet_size 
     * SBDWB will send the next 70 bytes that start at mo_idx*max_packet_size
     * from the beginning of mo_buffer_copy
     */
    size_t mo_idx;

    /**
     * True if QM encountered an unexpected response from execute()
     * All states transition to wait when this flag is set. 
     * Wait transitions to config after max_wait_cycles passes by
     * Only wait may clear the flag
     */
    bool unexpected_flag;

    /**
     * Max cycles that each radio_state state is allowed to waste before being 
     * transitioned. 
     */ 
    // TODO: these values are temporary. Experiments should be conducted
    // to figure out maximum cycles we are willing to wait. 
  public:
    TRACKED_CONSTANT_SC(unsigned int, max_config_cycles, 5);
    TRACKED_CONSTANT_SC(unsigned int, max_write_cycles, 5);
    TRACKED_CONSTANT_SC(unsigned int, max_read_cycles, 5);

    TRACKED_CONSTANT_SC(size_t, packet_size, 70);
};
