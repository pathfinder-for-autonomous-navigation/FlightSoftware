
#pragma once
#include <bitstream.h>
#include "ControlTask.hpp"

/**
 * Uplink provides operations on an Uplink Packet and is stateless
 */
class Uplink {
  public:
  Uplink(StateFieldRegistry& r);

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

  protected:
  /**
   * Validates the packet
   */
  bool _validate_packet(bitstream& bs); 

};