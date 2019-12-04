#include "bitstream.h"

bitstream::bitstream(char* input, uint32_t stream_size) :
  bit_offset(0),
  stream(reinterpret_cast<uint8_t*>(input)),
  byte_offset(0),
  max_len(stream_size)
{
}

bitstream::bitstream(const std::vector<bool>& bit_array, char* res) :
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

bool bitstream::has_next()
{
  return byte_offset < max_len;
}

uint32_t bitstream::nextN(size_t i)
{
  if (!has_next() || i > 8) // obviously, don't ask for more than 8 bits..
    return 0;
  uint32_t res = ( *(stream + byte_offset) >> bit_offset ) & ( (1ul << i) - 1 );
  // Consume the bits
  bit_offset += i;
  uint32_t num_iters = bit_offset / 8;
  for (; num_iters > 0; --num_iters)
  {
    ++byte_offset;
    bit_offset %= 8;
    uint32_t remain = ( *(stream + byte_offset));
    remain &= ( (1ul << bit_offset) - 1 );
    remain <<= (i - bit_offset);
    res |= remain;
  }
  return res;
}

uint32_t bitstream::peekN(size_t i)
{
  uint32_t byte_offset_ = byte_offset, bit_offset_ = bit_offset;

  uint32_t res = nextN(i);

  // rewind
  byte_offset = byte_offset_;
  bit_offset = bit_offset_;
  return res;
}

size_t bitstream::nextN(size_t num_bits, uint8_t* res)
{
  // just read it 8 at a time
  size_t num_iters = num_bits/8;
  size_t modulo = num_bits%8;
  for (int i = 0; i < num_iters; ++i)
  {
    res[i] = nextN(8);
  }
  if (modulo != 0)
  {
    res[num_iters++] = nextN(modulo);
  }
  return num_iters;
}

size_t bitstream::peekN(size_t num_bits, uint8_t* res)
{
  uint32_t byte_offset_ = byte_offset, bit_offset_ = bit_offset;

  uint32_t amt = nextN(num_bits, res);

  // rewind
  byte_offset = byte_offset_;
  bit_offset = bit_offset_;
  return amt;
}

void bitstream::nextN(size_t num_bits, std::vector<bool>& bit_arr)
{
  if (bit_arr.size() < num_bits)
  {
    cout << "idk how you expect to shove " << num_bits << " bits into an array of size " << bit_arr.size() << endl;
    return;
  }
  size_t old_size = bit_arr.size();
  bit_arr.clear();
  bit_arr.resize(old_size, 0);
  for (int i = 0; i < num_bits; ++i)
  {
    bit_arr[i] = nextN(1);
  }
}

void bitstream::peekN(size_t num_bits, std::vector<bool>& bit_arr)
{

  size_t old_size = bit_arr.size();
  bit_arr.clear();
  bit_arr.resize(old_size, 0);

  for (int i = 0; i < num_bits; ++i)
  {
    bit_arr[i] = lookN(1, (bit_offset+i)%8, byte_offset + (i/8));
  }
}

uint32_t bitstream::lookN(size_t i, uint32_t our_bit_off, int our_byte_off)
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

void bitstream::seekG(size_t amt, int dir)
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

bitstream& operator >>(bitstream& bs, uint32_t& res)
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

bitstream& operator >>(bitstream& bs, uint16_t& res)
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

bitstream& operator >>(bitstream& bs, uint8_t& res)
{
  res = 0;
  res = bs.nextN(8);
  return bs;
}

bitstream& operator >>(bitstream& bs, std::vector<bool>& bit_arr)
{
  bs.nextN(bit_arr.size(), bit_arr);
  return bs;
}

bitstream& operator <<(uint32_t& u32, bitstream& bs)
{
  if (bs.byte_offset + 4 > bs.max_len) 
    return bs;
  
  *reinterpret_cast<uint32_t*>(bs.stream + bs.byte_offset) = u32;
  return bs;
}

bitstream& operator <<(uint16_t& u16, bitstream& bs)
{
  if (bs.byte_offset + 2 > bs.max_len) 
    return bs;
  
  *reinterpret_cast<uint16_t*>(bs.stream + bs.byte_offset) = u16;
  return bs;
}

bitstream& operator <<(uint8_t& u8, bitstream& bs)
{
  if (bs.byte_offset + 1 > bs.max_len) 
    return bs;
  
  *reinterpret_cast<uint8_t*>(bs.stream + bs.byte_offset) = u8;
  return bs;
}

bitstream& operator <<(std::vector<bool>& ba, bitstream& bs)
{
  char backed[ba.size()];
  bitstream bs_other(ba, backed);
  bs_other << bs;
  return bs;
}

bitstream& operator <<(bitstream& bs_other, bitstream& bs)
{
  if (bs_other.max_len + bs.byte_offset > bs.max_len)
    return bs;
  
  memcpy(bs.stream + bs.byte_offset, bs_other.stream, bs_other.max_len);
  return bs;
}