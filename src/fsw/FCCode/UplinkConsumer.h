#pragma once
#include "TimedControlTask.hpp"
#include "UplinkCommon.h"

/**
 * Uplink Consumer parses uplink packets received in its MT buffer. Expects
 * QuakeManager to set radio_mt_packet_fp to point to its MT buffer and expects
 * QuakeManager to set radio_mt_packet_len_fp whenever a new uplink is received. 
 * 
 * Dependencies: QuakeManager, StateFieldRegistry (must have access to this),
 * Uplink
 * 
 * State Fields Provided: radio_mt_packet_fp, radio_mt_packet_len_fp
 * 
 * !!!IMPORTANT!!!! Most call init_uplink(). I include an if index_size == 0 in 
 * each function just in case
 * 
 * Protocol: A packet is parsed into a set of "requests". A request consists of
 * the index of a field in the registry and a new value to assign to that field. 
 * A request is invalid if the indicated field is not writable. 
 * A packet may consist of several requests. We will assume that new values are
 * padded such that they will be the same length as the field which they update. 
 * 
 * Example: 
 * Suppose we have this packet: [10][2][1][4]
 * 
 * It will have this binary representation: 
 * [_ _ _ _ _][_ _ _][_ _ _ _ _][_ _ _ _ _]
 * [0 1 0 1 0][0 1 0][0 0 0 0 1][0 0 1 0 0]
 * 
 * Uplink Consumer will interpret this packet as two requests:
 *      Update field 10 with the value 2
 *      Update field 1 with the value 4
 * 
 * Fields are indexed starting at 1. There is no field 0.
 * 
 * Notice that indices must be represented with the same number of bits. 
 * New values are assumed to be the same length as the field's value. If this 
 * packet is valid, we may assume that field 10 can be at most 7, and field 1 can
 * be at most 31. 
 * 
 */ 

class UplinkConsumer : public TimedControlTask<void>, public Uplink {
   public:
    
    /**
     * @param registry State field registry
     * @param offset Offset from the start of the cycle (TimedControlTask)
     */
    UplinkConsumer(StateFieldRegistry& registry, unsigned int offset);
    
    /**
     * @brief Checks radio_mt_packet_len_f to see if QuakeManager has received a new
     * uplink. If a new uplink has been received, UplinkConsumer expects the
     * contents of the uplink packet to be in the buffer pointed to by radio_mt_packet_f. 
     * UplinkConsumer will then check the packet, update the fields in the registry,
     * and reset radio_mt_packet_len_f.
     */
    void execute() override;
#ifndef DEBUG
    protected:
#else
    public:
#endif

    /**
     * @brief Applies all the updates specified by mt packet to writable fields
     * @param packet_size the number of bits in the packet excluding padding
     */
    void update_fields();

    /**
     * @brief Checks that all the requests in the packet are valid
     * A packet is valid if 
     * - all requests are for valid indices in registry.writable_fields
     * - the length of the packet contains enough data to update all fields
     * but no more than 7 extra bits of padding
     * - no field is updated more than once
     * @return true if packet is valid and false otherwise
     */
    bool validate_packet();

  /**
   * @brief The length of the newly received mt packet in bytes
   * If it is nonzero, that means a new packet has been read during the current
   * control cycle
   * QuakeManager sets this field whenever it writes a new message 
   * to its mt buffer.
   * UplinkProducer is responsible for checking this field and reseting it to 0. 
   */
   InternalStateField<size_t>* radio_mt_packet_len_fp;

  /**
    * @brief Pointer to the uplink buffer in QuakeManager
    **/ 
  const InternalStateField<char*>* radio_mt_packet_fp;
};
