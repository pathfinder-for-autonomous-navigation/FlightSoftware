#pragma once
#include <ControlTask.hpp>
#include "QuakeControlTask.h"
#include "radio_mode_t.enum"

/**
 * QuakeManager decides what commands should be sent to the Quake based on the
 * current control cycle and the results of prior control cycles. 
 * 
 * Dependencies: ClockManager, Downlink Provider
 * 
 * States: config, wait, transceive, read, write, manual
 * 
 * All methods return true on success, false otherwise. 
 */ 
class QuakeManager : public TimedControlTask<bool> {
   public:
    QuakeManager(StateFieldRegistry& registry, unsigned int offset);
    ~QuakeManager();
    bool execute() override;

   // protected:
    bool dispatch_config();
    bool dispatch_wait();
    bool dispatch_transceive();
    bool dispatch_read();
    bool dispatch_write();
    bool dispatch_manual();

   /**
     * @brief Control cycle count, provided by ClockManager.
     */
    ReadableStateField<unsigned int>* control_cycle_count_fp;

   /**
     * @brief Snapshot size in bytes, provided by DownlinkProducer. 
     */
    InternalStateField<unsigned int>* snapshot_size_fp;

   /**
    * @brief Pointer to the snapshot t1o be downlinked in pieces of 70 B, provided by DownlinkProducer.
    **/ 
    InternalStateField<char*>* radio_mo_packet_fp;

  /**
    * @brief Pointer to the uplink buffer, provided by DownlinkProducer. 
    **/ 
   InternalStateField<char*>* radio_mt_packet_fp;

     /**
    * @brief Pointer to Quake Error field, provided by DownlinkProducer. 
    **/ 
   InternalStateField<int>* radio_err_fp;

    /**
     * @brief Current radio mode (see radio_mode_t.enum), provided by
     **/
    radio_mode_t radio_mode_f;

  // private:
    QuakeControlTask qct;
    /**
     * Write error to radio_err_fp,
     * print a debug msg, 
     * transition to wait
     */
    bool write_to_error(int err_code);

    /**
     * If the current state has no more cycles left, 
     * then transition the radio to the requested state,
     * printf notice, 
     * Return true if there are no more control cycles, false otherwise
     */
    bool no_more_cycles(size_t max_cycles, radio_mode_t new_state);

    /**
     * Transition the radio into the new state
     * update last_checkin_cycle
     * Precondition: new_state is one of the defined states
     * Postcondition: radio_state_f == new_state, last_checkin_cycle = now
     */ 
    void transition_radio_state(radio_mode_t new_state);

    /**
     * If we have written the entire snapshot, load the next snapshot
     * Otherwise, increment mo_idx to point to the next 70 blocks
     * 
     * Essentially points mo_buffer_copy + mo_idx*packet_size to the next
     * 70 blocks of data that should be downlinked. 
     */
    void update_mo();

    /**
     * The last cycle for which we had comms
     */
    unsigned int last_checkin_cycle;

    size_t max_snapshot_size;
    char* mo_buffer_copy;
    size_t mo_idx;

    /**
     * True if QM encountered an unexpected response
     * All states transition to wait when this flag is set. 
     * Wait transitions to config
     * Only config may unset the flag
     */
    bool unexpectedFlag;

    /**
     * Max cycles that each radio_mode state is allowed to waste before being 
     * transitioned. 
     */ 
    // TODO: these values are temporary. Experiments should be conducted
    // to figure out maximum cycles we are willing to wait. 
    static constexpr unsigned int max_config_cycles = 30;
    static constexpr unsigned int max_wait_cycles = 3000; 
    static constexpr unsigned int max_transceive_cycles = 140;
    static constexpr unsigned int max_write_cycles = 15;
    static constexpr unsigned int max_read_cycles = 15;

    static constexpr size_t packet_size = 70;
};