#ifndef piksi_mode_t_enum_
#define piksi_mode_t_enum_

enum class piksi_mode_t {
  spp, // Single Point Position
  fixed_rtk, // Fixed RTK fix
  float_rtk, // Float RTK fix

  no_fix, // Insufficient GPS signal for any fix
  
  sync_error, // Time of weeks not sync'd, 
  nsat_error, // #Satellites < 4

  crc_error, // Corrupted data caused a CRC error
  time_limit_error, // Exceeded 900 microseconds during process_buffer loop
  data_error, // Unexpected data, but otherwise good packet, ex weird flag
  no_data_error, // No bytes to read from the buffer

  dead //Piksi not functional

};

#endif
