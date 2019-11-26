#pragma once
#include <TimedControlTask.hpp>

/**
 * Uplink Consumer parses uplink packets received in its MT buffer. Expects
 * QuakeManager to set radio_mt_packet_fp to point to its MT buffer and expects
 * QuakeManager to set radio_mt_ready_fp whenever a new uplink is received. 
 * 
 * Dependencies: QuakeManager, StateFieldRegistry (must have access to this)
 * 
 * State Fields Provided: radio_mt_packet_fp, radio_mt_ready_fp
 * 
 * Notes: Since radio_mt_packet_f is a pointer to a buffer in QuakeManager,
 * if QuakeManager is destroyed/delete, this pointer will become invalid. 
 * QuakeManager must set radio_mt_ready_f to false before it dies. 
 * 
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
 * Notice that indices must be represented with the same number of bits. 
 * New values are assumed to be the same length as the field's value. If this 
 * packet is valid, we may assume that field 10 can be at most 7, and field 1 can
 * be at most 31. 
 * 
 */ 

class UplinkConsumer : public TimedControlTask<void> {
   public:

    /**
     * @param registry State field registry
     * @param offset Offset from the start of the cycle (TimedControlTask)
     */
    UplinkConsumer(StateFieldRegistry& registry, unsigned int offset);
    /**
     * @brief Checks radio_mt_ready_f to see if QuakeManager has received a new
     * uplink. If a new uplink has been received, UplinkConsumer expects the
     * contents of the uplink packet to be in the buffer pointed to by radio_mt_packet_f. 
     * UplinkConsumer will then clear the rardio_mt_ready_f flag, parse the packet,
     * and update the relevant fields in the registry. 
     */
    void execute() override;
#ifdef DEBUG
  // Methods to be used in debug only. 
InternalStateField<bool>* get_mt_ready()
{
    return &radio_mt_ready_f;
}
InternalStateField<char*>* get_mt_ptr()
{
    return &radio_mt_packet_f;
}

#endif
    private:

    /**
     * @brief Parses the uplink packet
     */
    void parse_packet();

    /**
     * @brief Checks that the field is writable
     * @return returns the length of the field
     */
    size_t check_field(size_t field_index);

    /**
     * @brief Updates the field_index with the new_value 
     */
    void update_field(size_t field_index, unsigned char new_value); // TODO: what should this type be? 

    /**
     * Checks that all the requests in the packet are valid
     * A packet is valid if all requests are for writable fields and if the length
     * of the packet matches indices and corresponding fields.
     * 
     * @return true if all requests are valid and packet is valid, false otherwise
     */
    bool validate_packet()
    {
        // Should we validate packet first, or update all valid requests? 
        return false;
    }

  /**
   * @brief Flag that indicates whether there is a new uplink packet in radio_mt_packet_f
   * QuakeManager sets this field whenever it writes a new message 
   * to its mt buffer.
   * UplinkProducer is responsible for checking this field and clearing it. 
   */
   InternalStateField<bool> radio_mt_ready_f; 

  /**
    * @brief Pointer to the uplink buffer in QuakeManager. 
    **/ 
   InternalStateField<char*> radio_mt_packet_f;
    
};

