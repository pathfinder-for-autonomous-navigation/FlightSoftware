#pragma once
/**
 * bitstream represents a stream of bits
 * 
 */
#include <vector>
using namespace std;

#define bs_beg -1 // bit stream in the direction towards the beginning
#define bs_end 1 // bit stream in the direction towards end of the stream

/**
 * bitstream operates on bytes (since that is the smallest possible unit)
 * On my computer, the LSB is on the left, so 0xd is 1011 not 1101
 * The nibble ordering is also in little endian so 0xde is stored 0xed 
 */
class bitstream{
  public:
  /**
   * The bit offset within the current byte
   */
  uint32_t bit_offset;
  /**
   * The bit stream
   */
  uint8_t* stream;

  /**
   * The byte offset within the stream, and index into the stream
   */
  uint32_t byte_offset;

  /**
   * The maximum length of the stream in bytes
   */
  uint32_t max_len; 

/**
 * @brief Constructs a bitstream
 * @param input input data of the stream
 * If you intend to change your bitstream, input MUST not be a const char*
 * or you will not be able to change your bitstream
 * @param stream_size the size of the input in bytes
 */
  bitstream(char* input, uint32_t stream_size);

/**
 * @brief Constructs a bitstream from a bit_array
 * @param bit_array the bit array to use
 * @param res Allocate stuff yourself. 
 */
  bitstream(const std::vector<bool>& bit_array, char* res);

/**
 * @brief Returns true if there's a next bit
 * @returns true if there's a next bit
 */
  bool has_next();

/**
 * @brief consumes the next [i] number of bits if possible
 * @return returns a uint8_t representing the next 8 bits, returns 0 if stream
 * is empty
 */
  uint32_t nextN(size_t i);

/**
 * @brief does not consume the next[i] number of bits
 * @return the next [i] number of bits if possible
 */
  uint32_t peekN(size_t i);

/**
 * @brief reads [num_bits] into res
 * @return the number of BYTES written
 */
  size_t nextN(size_t num_bits, uint8_t* res);

/**
 * @brief does not consume the next[i] number of bits
 * @param res the array with the result of the stream
 * @param num_bits the number of bits in total that we want to consume
 * @return the next [i] number of bits if possible
 */
  size_t peekN(size_t num_bits, uint8_t* res);

/**
 * @brief Consumes the next [num_bits] and stores them in [bit_arr]
 * Requires that bit_arr.size() is > num_bits
 * @param num_bits the number of bits we want form the stream
 * @param bit_arr the bit array to store the consumed bits
 */
  void nextN(size_t num_bits, std::vector<bool>& bit_arr);

  void peekN(size_t num_bits, std::vector<bool>& bit_arr);

/**
 * @brief returns the next [i] number of bits from the specified byte offset and
 * bit offset
 * @param i number of bits (1, 2, 4, 8, 16, etc)
 * @param byte_off byte offset to read the bits from relative to the current position
 * @param bit_off the bit offset within the specified byte
 * warning: not tested (yet)
 */
  uint32_t lookN(size_t i, uint32_t bit_off, int byte_off);

/**
 * Move the byte pointer [amt] bits in dir [dir]
 * @brief amt number of bits we want to move the byte pointer
 * @brief dir direction:
 *  bs_beg: direction towards the beginning
 *  bs_end: direction towards the end
 */
  void seekG(size_t amt, int dir);

};

/**
 * @brief consumes 32, 16, or 8 bits of the bit stream into the int
 */ 
bitstream& operator >>(bitstream&, uint32_t&);
bitstream& operator >>(bitstream&, uint16_t&);
bitstream& operator >>(bitstream&, uint8_t&);
bitstream& operator >>(bitstream&, std::vector<bool>&);

/**
 * @brief Special operators for editing at the current position of a bit stream
 * Use seek to get to the desired position. There is no appending to the 
 * bit stream since we do not want to deal with allocation.
 * There's no using char* since bytes may be null 
 */
bitstream& operator <<(uint32_t&, bitstream&);
bitstream& operator <<(uint16_t&, bitstream&);
bitstream& operator <<(uint8_t&, bitstream&);
bitstream& operator <<(std::vector<bool>&, bitstream&);
bitstream& operator <<(bitstream&, bitstream&);