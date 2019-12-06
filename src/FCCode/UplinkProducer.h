#include <iostream>
using namespace std;

/**
 * UplinkProducer produces uplink packets that are parsed by UplinkConsumer
 * 
 * Format of the packet:
 * [index][data][index][data]... etc
 * 
 */
class UplinkProducer {
  public:

/**
 * Maximum size of the packet
 */ 
size_t _packet_size;

/**
 * Size in bits of the indices
 */
size_t _index_length;

/**
 * Validate the packet by checking that indices are all valid and values seem
 * correct
 */
  void validate_packet();

/**
 * Print a json representation of the current packet
 */
  void print_packet();


  /**
   * Print the packet as an SBD file
   * @param filename The file to save the packet in
   */
  void commit_packet(const char* file_name); 

  /**
   * Construct an UplinkProducer which will make a packet of size packet_size
   * (in bytes) with indices of index_length (in bits)
   * @param packet_size maximum size of the packet in bytes
   * @param index_length number of bits in an index
   */
  UplinkProducer(size_t packet_size, size_t index_length);

};
