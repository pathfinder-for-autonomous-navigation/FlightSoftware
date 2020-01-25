#pragma once
/**
 * bitstream represents a stream of bits.
 * The smallest possible bit stream is 8 bits since that is the smallest C++
 * unit
 * 
 * Example:
 * If initialized with data \x12\x34\x56, it will be stored like this:
 * 2    1    4    3    6    5
 * 0100 1000 0010 1100 0110 1010
 * max_len = 3 because this is three bytes
 * 
 * Note:
 * The bitstream must be initialized with a mutable char array if you intend to
 * edit the stream
 * bitstream does not allocate any data, so it must be initialized with data
 * that stays in scope during the lifetime of the bitstream
 */
#include <vector>
#include <cstdint>
#include <cstddef>

#define bs_beg -1 // bit stream in the direction towards the beginning
#define bs_end 1 // bit stream in the direction towards end of the stream

class bitstream{

  public:
  /**
   * The bit offset within the current byte
   */
  uint32_t bit_offset;
  
  /**
   * The bit stream is represented as an array of bytes (8-bit unsigned ints)
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
 * @brief Constructs a bitstream from a char array
 * @param input stream data, must be mutable if you intend to edit the stream
 * @param stream_size the size of the data in bytes
 */
  bitstream(char* input, uint32_t stream_size);

/**
 * @brief Constructs a bitstream from a bit_array
 * @param bit_array the bit array of the data
 * @param res A pointer to a mutable array that will "back" the bit_array. This
 * param must stay in scope during the lifetime of the bitstream and is necessary
 * because bitstream does not allocate memory and bit_array.data() is unspecified
 */
  bitstream(const std::vector<bool>& bit_array, char* res);

/**
 * @brief Returns true if there's a next bit
 * @returns true if there's a next bit
 */
  bool has_next();

/**
 * @brief Attempts to consume a specified number of bits from the current position
 * and returns them in res
 * @param num_bits the number of bits to read
 * @param res a pointer to mutable array that has size of at least (num_bits+7)/8
 * @return the number of bits read
 */
  size_t nextN(size_t num_bits, uint8_t* res);

/**
 * @brief Attempts to consume a specified number of bits from the current position
 * and returns them in bit_arr
 * Requires that bit_arr.size() is >= num_bits
 * @param num_bits the number of bits to read from the stream
 * @param bit_arr the bit array to store the consumed bits
 * @param return 0 if bit_arr is not big enough else the number of bits read
 */
  size_t nextN(size_t num_bits, std::vector<bool>& bit_arr);

/**
 * @brief Same as nextN but does not consume the bits
 * @param num_bits the number of bits to read
 * @param res a pointer to mutable array that has size of at least (num_bits+7)/8
 * @return the number of bits read
 */
  size_t peekN(size_t num_bits, uint8_t* res);

/**
 * @brief Same as nextN but does not consume the bits
 * @param num_bits the number of bits to peek
 * @param bit_arr the bit array to store the consumed bits
 * @return the number of bits read
 */
  size_t peekN(size_t num_bits, std::vector<bool>& bit_arr);

/**
 * @brief Moves the position of the byte and bit pointer to a given offset
 * @param amt number of bits we want to move in
 * @param dir direction:
 *  bs_beg: direction towards the beginning
 *  bs_end: direction towards the end
 * @return 0 if unable to fufill request, else returns amt
 */
  size_t seekG(size_t amt, int dir);

/**
 * @brief Write a number of bits from new_val to this bitstream
 * @param new_val The data source to read from
 * @param num_bits The number of bits to write
 * @return the number of bits written
 */
  size_t editN(size_t num_bits, uint8_t* new_val);

/**
 * @brief Write a number of bits from bs_other to this bitstream
 * @param bs_other The source bitstream
 * @param num_bits The number of bits to write
 * @return the number of bits written
 */
  size_t editN(size_t num_bits, bitstream& bs_other);

/**
 * @brief sets the byte_offset and bit_offset to 0
 */
  void reset();

  private: //These functions are private because they are unsafe

/**
 * @brief Attempts to consume a specified number of bits from the current position
 *  and returns them in u8
 * @param num_bits the number of bits to read. Maximum is 8
 * @param u8 a pointer to the unsigned int to write to
 * @return 0 if num_bits > 8, else the number of bits read
 */
  size_t next(size_t num_bits, uint8_t* u8);

/**
 * @brief Writes a specified number of bits of the given unsigned byte to the
 * current position of the bitstream. The stream is "consumed", so bit_offset
 * and byte_offset will move to reflect the number of bits written
 * @param u8 8 bit unsigned int to write to the current position in the stream
 * @param num_bits the number of bits to write from u8
 * @return The number of bits written
 */
  size_t edit(size_t num_bits, uint8_t* u8);

};

/**
 * @brief Special operators for reading at the current position of a bit stream.
 * Consumes 32, 16, or 8 bits of the bit stream into the specified unsigned int
 */ 
bitstream& operator >>(bitstream&, uint32_t&);
bitstream& operator >>(bitstream&, uint16_t&);
bitstream& operator >>(bitstream&, uint8_t&);
bitstream& operator >>(bitstream&, std::vector<bool>&);

/**
 * @brief Special operators for editing at the current position of a bit stream
 * Use seek to get to the desired position. There is no appending to the 
 * bit stream since we do not want to deal with allocation.
 * Do not use these operators if the bitstream was initialized with const char* 
 */
bitstream& operator <<(uint32_t&, bitstream&);
bitstream& operator <<(uint16_t&, bitstream&);
bitstream& operator <<(uint8_t&, bitstream&);
bitstream& operator <<(std::vector<bool>&, bitstream&);
bitstream& operator <<(bitstream&, bitstream&);
