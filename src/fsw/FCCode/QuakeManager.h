#pragma once
#include "TimedControlTask.hpp"
#include "QuakeControlTask.h"
#include "radio_state_t.enum"

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
class QuakeManager : public TimedControlTask<bool> {
   public:
    QuakeManager(StateFieldRegistry& registry, unsigned int offset);
    ~QuakeManager();
    bool execute() override;

   // protected:
   /**
    * @brief attempts to execute a step in the CONFIG command sequence. This command
    * sequence is executed on startup and executed whenever the unexpected_flag 
    * is set
    */
    bool dispatch_config();
   /**
    * @brief unconditionally waits the full amount of its allocated cycles
    * If the unexpected_flag is set, transitions to config instead of 
    * transition to wait write
    */
    bool dispatch_wait();
   /**
    * @brief attempts an SBD connection with Iridium.
    */
    bool dispatch_transceive();
   /**
    * @brief reads an MT message from the MT buffer
    */
    bool dispatch_read();
   /**
    * @brief writes a message to the MO buffer
    */
    bool dispatch_write();

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
    return transition_radio_state(new_state);
  }

  #endif

  private:
    QuakeControlTask qct;
    /**
     * Returns true if err_code either OK or PORT_UNAVAILABLE.
     * Otherwise: 
     *  Write error to radio_err_fp,
     *  print a debug msg, 
     *  sets the unexpected_flag
     *  transition to wait
     */
    bool write_to_error(int err_code);

    /**
     * If the current state has no more cycles left, 
     * then transition the radio to the requested state,
     * printf a notice about the transition 
     * Return true if there are no more control cycles, false otherwise
     */
    bool no_more_cycles(size_t max_cycles, radio_state_t new_state);

    /**
     * Transition the radio into the new state
     * update last_checkin_cycle to current cycle
     * Precondition: new_state is one of the defined states
     * Postcondition: radio_state_f == new_state, last_checkin_cycle = now
     */ 
    bool transition_radio_state(radio_state_t new_state);

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
    static constexpr unsigned int max_config_cycles = 5;
    static constexpr unsigned int max_write_cycles = 5;
    static constexpr unsigned int max_read_cycles = 5;

    static constexpr size_t packet_size = 70;
};