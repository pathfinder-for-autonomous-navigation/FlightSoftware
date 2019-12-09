#include "BitStream.h"
#include <fixed_array.hpp>
BitStream::BitStream(char* input, uint32_t stream_size) :
  bit_offset(0),
  stream(reinterpret_cast<uint8_t*>(input)),
  byte_offset(0),
  max_len(stream_size)
{
}

BitStream::BitStream(const std::vector<bool>& bit_array, char* res) :
  bit_offset(0),
  byte_offset(0)
{
  size_t stream_size = (bit_array.size() + 7)/8;
  for (int i = 0; i < stream_size; ++i)
  {
    res[i] = 0;
    for (int j = 0; j < 8; ++j)
    {
      res[i] |= ( (bit_array[i*8 + j]) <<  j );
    }
  }
  max_len = stream_size;
  stream = reinterpret_cast<uint8_t*>(res);
}

bool BitStream::has_next()
{
  return byte_offset < max_len;
}

uint8_t BitStream::nextN(size_t num_bits)
{
  if (num_bits > 8) return 0;
  uint8_t u8 = 0;
  uint8_t bits_read = 0;
  uint8_t old = *reinterpret_cast<uint8_t*>(stream + byte_offset);
  old >>= bit_offset;
  // Read until the next byte starts
  size_t amt = 8 - bit_offset;
  for (; bits_read < amt && bits_read < num_bits; old >>= 1, ++bits_read)
    u8 = bit_array::modify_bit(u8, bits_read, old&1);

  // See if there is a next byte to write
  if (byte_offset + 1 < max_len && bits_read < num_bits)
  {
    old = *reinterpret_cast<uint8_t*>(stream + byte_offset + 1);
    // Read to the beginning of the next byte
    for (size_t i = 0; i < bit_offset && bits_read < num_bits; ++i, old >>= 1, ++bits_read)
    {
      u8 = bit_array::modify_bit(u8, bits_read, old&1);
    }
  }
  seekG(bits_read, bs_end);
  return u8;
}

uint32_t BitStream::peekN(size_t i)
{
  uint32_t byte_offset_ = byte_offset, bit_offset_ = bit_offset;

  uint32_t res = nextN(i);

  // rewind
  byte_offset = byte_offset_;
  bit_offset = bit_offset_;
  return res;
}

size_t BitStream::nextN(size_t num_bits, uint8_t* res)
{
  memset(res, 0, (num_bits + 7)/8);
  // just read it 8 at a time
  size_t num_iters = num_bits/8;
  size_t modulo = num_bits%8;
  for (int i = 0; i < num_iters && has_next(); ++i)
  {
    res[i] = nextN(8);
  }
  if (modulo != 0 && has_next())
  {
    res[num_iters++] = nextN(modulo);
  }
  return num_iters;
}

size_t BitStream::peekN(size_t num_bits, uint8_t* res)
{
  uint32_t byte_offset_ = byte_offset, bit_offset_ = bit_offset;

  uint32_t amt = nextN(num_bits, res);

  // rewind
  byte_offset = byte_offset_;
  bit_offset = bit_offset_;
  return amt;
}

void BitStream::nextN(size_t num_bits, std::vector<bool>& bit_arr)
{
  if (bit_arr.size() < num_bits)
  {
    cout << "idk how you expect to shove " << num_bits << " bits into an array of size " << bit_arr.size() << endl;
    return;
  }
  size_t old_size = bit_arr.size();
  bit_arr.resize(old_size, 0);
  for (int i = 0; i < num_bits; ++i)
  {
    bit_arr[i] = nextN(1);
  }
}

void BitStream::peekN(size_t num_bits, std::vector<bool>& bit_arr)
{

  size_t old_size = bit_arr.size();
  bit_arr.resize(old_size, 0);

  for (int i = 0; i < num_bits; ++i)
  {
    bit_arr[i] = lookN(1, (bit_offset+i)%8, byte_offset + (i/8));
  }
}

uint32_t BitStream::lookN(size_t i, uint32_t our_bit_off, int our_byte_off)
{
  our_bit_off %= 8;
  if (our_byte_off >= max_len)
    our_byte_off = max_len - 1;
  // save the old ones
  uint32_t old_byte = byte_offset, old_bit = bit_offset;
  // set our values
  byte_offset = our_byte_off, bit_offset = our_bit_off;
  uint32_t res = nextN(i);
  //rewind
  byte_offset = old_byte, bit_offset = old_bit;
  return res;
}

void BitStream::seekG(size_t amt, int dir)
{
  if (dir != -1 && dir != 1)
    return;
  
  // calculate new byte offset
  size_t desired_off = 8*byte_offset + bit_offset;
  desired_off += (dir*amt);

  size_t new_byte_off = desired_off/8;
  size_t new_bit_off = desired_off%8;

  if (new_byte_off > max_len || new_byte_off < 0)
    return;

  byte_offset = new_byte_off;
  bit_offset = new_bit_off;

}

/**
 * @brief Writes a specified number of bits of the given unsigned byte to the
 * current position of the bitstream. The stream is "consumed", so bit_offset
 * and byte_offset will move to reflect the number of bits written
 * @param u8 8 bit unsigned int to write to the current position in the stream
 * @param num_bits the number of bits to write from u8
 * @return The number of bits written
 * 
 */
size_t BitStream::editN(size_t num_bits, uint8_t u8)
{
  uint8_t old = *reinterpret_cast<uint8_t*>(stream + byte_offset);
  
  uint8_t bits_written = 0;
  // Write until the next byte starts
  size_t amt = 8 - bit_offset;
  for (size_t i = 0; i < amt && bits_written < num_bits; ++i, u8 >>= 1, ++bits_written)
    old = bit_array::modify_bit(old, bit_offset + i, u8&1);
  
  // Write the lower bits
  *reinterpret_cast<uint8_t*>(stream + byte_offset) = old;
  
  // See if there is a next byte to write
  if (byte_offset + 1 < max_len && bits_written < num_bits)
  {
    old = *reinterpret_cast<uint8_t*>(stream + byte_offset + 1);
    // Write to the beginning of the next byte
    for (size_t i = 0; i < bit_offset && bits_written < num_bits; ++i, u8 >>= 1, ++bits_written)
    {
      old = bit_array::modify_bit(old, i, u8&1);
    }
    // Write the rest of the upper bits
    *reinterpret_cast<uint8_t*>(stream + byte_offset + 1) = old;
  }
  seekG(bits_written, bs_end);
  return bits_written;
}

size_t BitStream::editN(size_t num_bits, uint8_t* new_val)
{
  size_t num_iters = num_bits/8;
  size_t modulo = num_bits%8;
  size_t bits_written = 0;
  for (int i = 0; i < num_iters; ++i)
  {
    bits_written = editN(8, new_val[i]);
  }
  if (modulo != 0)
  {
    bits_written = editN(modulo, new_val[num_iters++]);
  }
  return bits_written;
}

/**
 * @brief Write a number of bits from bs_other to this BitStream
 * @param bs_other The source bitstream
 * @param num_bits The number of bits to write
 * @return the number of bits written
 */
size_t BitStream::editN(size_t num_bits, BitStream& bs_other)
{

  return 0;
}

BitStream& operator >>(BitStream& bs, uint32_t& res)
{
  res = 0;
  uint8_t tmp[4];
  bs.nextN(32, tmp);
  for (int i = 0; i < 4; ++i)
  {
    res |= (tmp[i] << (8* i));
  }
  return bs;
}

BitStream& operator >>(BitStream& bs, uint16_t& res)
{
  res = 0;
  uint8_t tmp[2];
  bs.nextN(16, tmp);
  for (int i = 0; i < 2; ++i)
  {
    res |= (tmp[i] << (8* i));
  }
  return bs;
}

BitStream& operator >>(BitStream& bs, uint8_t& res)
{
  res = 0;
  res = bs.nextN(8);
  return bs;
}

BitStream& operator >>(BitStream& bs, std::vector<bool>& bit_arr)
{
  bs.nextN(bit_arr.size(), bit_arr);
  return bs;
}

BitStream& operator <<(uint32_t& u32, BitStream& bs)
{ 
  bs.editN(32, reinterpret_cast<uint8_t*>(&u32));
  return bs;
}

BitStream& operator <<(uint16_t& u16, BitStream& bs)
{
  bs.editN(16, reinterpret_cast<uint8_t*>(&u16));
  return bs;
}

BitStream& operator <<(uint8_t& u8, BitStream& bs)
{
  bs.editN(8, u8);
  return bs;
}

BitStream& operator <<(std::vector<bool>& ba, BitStream& bs)
{
  char backed[ba.size()];
  BitStream bs_other(ba, backed);
  bs_other << bs;
  return bs;
}

BitStream& operator <<(BitStream& bs_other, BitStream& bs)
{
  // if (bs_other.max_len + bs.byte_offset > bs.max_len)
  //   return bs; 
  size_t num_bits = (bs_other.max_len - bs_other.byte_offset)*8 - bs_other.bit_offset;
  size_t arr_size = (num_bits + 7)/8;
  char tmp[arr_size];
  memset(tmp, 0, arr_size);
  bs_other.nextN(num_bits, reinterpret_cast<uint8_t*>(tmp));
  bs.editN(num_bits, reinterpret_cast<uint8_t*>(tmp));
  // size_t cut_off = 8 - bs_other.bit_offset;
  // uint8_t u8 = bs_other.nextN(cut_off);
  // bs.editN(cut_off, u8);
  // num_bits -= cut_off;

  // size_t num_iters = num_bits/8;
  // size_t modulo = num_bits%8;
  // for (int i = 0; i < num_iters && bs_other.has_next() && bs.has_next() && num_bits > 0; ++i)
  // {
  //   bs_other >> u8;
  //   bs.editN(8, u8);
  //   num_bits -= 8;
  // }
  // if (modulo != 0)
  // {
  //   bs_other >> u8;
  //   bs.editN(modulo, u8);
  // }
  return bs;
}