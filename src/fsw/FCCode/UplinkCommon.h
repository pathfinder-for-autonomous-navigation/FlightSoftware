
#pragma once
#include <common/bitstream.h>
#include <common/StateFieldRegistry.hpp>

/**
 * Uplink provides operations on an Uplink Packet and is stateless
 * 
 */
class Uplink {
  public:
  Uplink(StateFieldRegistry& r);

  /**
   * registry is not assumed to be populated when this constructor is called
   * This method initializes fields that require registry to be populated
   */ 
  void init_uplink();

  /**
   * @brief Gets the size of the field indexed by field_index in registry.writable_fields
   * @return returns the length of the field if field_index is valid and 0 for invalid
   */
  size_t get_field_length(size_t field_index);
  
  /**
   * @brief Reference to registry so that we can update and read fields
   */
   StateFieldRegistry& registry;

  /**
   * @brief The number of bits to represent an index
   */
  size_t index_size;

#ifndef DEBUG
  protected:
#endif
  /**
   * Validates the packet
   */
  bool _validate_packet(bitstream& bs); 

  /**
   * Updates the fields of the registry with the uplink packet described by bs
   * This function is here in order to sync producer and consumer
   */
  void _update_fields(bitstream& bs);

};