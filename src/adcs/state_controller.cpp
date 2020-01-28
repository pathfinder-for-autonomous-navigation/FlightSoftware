//
// src/adcs/state_controller.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol   kpk63@cornell.edu
//   Shihao Cao  sfc72@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO : Update the HAVT table read once finalized

#ifdef UMB_LOG_LEVEL
  #define LOG_LEVEL UMB_LOG_LEVEL
#endif

#include "constants.hpp"
#include "state.hpp"
#include "state_controller.hpp"
#include "state_registers.hpp"
#include "utl/convert.hpp"
#include "utl/logging.hpp"

#include <bitset>

namespace adcs {
namespace umb {

/** \fn endian_write
 *  Writes a data type out over the slave i2c bus according to the current state
 *  endianness variables. */
template <typename T>
static void endian_write(T t) {
  unsigned char *ptr = (unsigned char *)(&t);
  // if (registers.endianess == Endianness::BIG)
  //   for (unsigned int i = sizeof(T) - 1; i >= 0; i--) umb::wire->write(ptr[i]);
  // else
  // ^^^ Endianess option was removed
    for (unsigned int i = 0; i < sizeof(T); i++) umb::wire->write(ptr[i]);
}

/** \fn endian_write
 *  Writes a data array out over the slave i2c bus according to the current
 *  state endianness variable. */
template <typename T, unsigned int N>
static void endian_write(T const (&t_arr)[N]) {
  for (unsigned int i = 0; i < N; i++) endian_write(t_arr[i]);
}

/** \fn endian_read
 *  Reads a data type in from the slave i2c bus according to the current state
 *  endianness variables. It is assumed the RX buffer contains a sufficient
 *  amount of incoming bytes. */
template <typename T>
static T endian_read() {
  T t;
  unsigned char *ptr = (unsigned char *)(&t);
  // if (registers.endianess == Endianness::BIG)
  //   for (unsigned int i = sizeof(T) - 1; i >= 0; i--) ptr[i] = umb::wire->read();
  // else
  // ^^^ Endianess option was removed
    for (unsigned int i = 0; i < sizeof(T); i++) ptr[i] = umb::wire->read();
  return t;
}

/** \fn endian_read
 *  Reads a data array in over the slave i2c bus according to the current state
 *  endianness variables. It is assumed the RX buffer contains a sufficient
 *  amount of incoming bytes. */
template <typename T, unsigned int N>
static void endian_read(T (&t_arr)[N]) {
  for (unsigned int i = 0; i < N; i++) t_arr[i] = endian_read<T>();
}

/** \fn copy_to
 *  Copies data into a volatile array from a nonvolatile array. */
template <typename T, unsigned int N>
static void copy_to(T const (&t_src)[N], T volatile (&t_dst)[N]) {
  for (unsigned int i = 0; i < N; i++) t_dst[i] = t_src[i];
}

/** \fn copy_to
 *  Copies data from a volatile array to a nonvolatile array. */
template <typename T, unsigned int N>
static void copy_to(T volatile const (&t_src)[N], T (&t_dst)[N]) {
  for (unsigned int i = 0; i < N; i++) t_dst[i] = t_src[i];
}

void on_i2c_recieve(unsigned int bytes) {
  LOG_INFO_header
  LOG_INFO_println("Recieved " + String(bytes) + " over I2C")

  if (umb::wire->available() < 1) return;
  unsigned char address = endian_read<unsigned char>();

  switch (address) {

    // case Register::ENDIANNESS: {
    //   if (umb::wire->available() < 1) break;
    //   registers.endianess = endian_read<unsigned char>();

    //   DEBUG_printF("...endianess set to ") DEBUG_println(registers.endianess)
    // }
    // ^^^ Endianess option was removed

    case Register::ADCS_MODE: {
      if (umb::wire->available() < 1) break;
      registers.mode = endian_read<unsigned char>();

      LOG_INFO_header
      LOG_INFO_println("ADCS_MODE set to " + String(registers.mode))
    }

    case Register::READ_POINTER: {
      if (umb::wire->available() < 1) break;
      registers.read_ptr = endian_read<unsigned char>();

      LOG_INFO_header
      LOG_INFO_println("READ_POINTER set to " + String(registers.read_ptr))

      break;
    }

    case Register::RWA_MODE: {
      if (umb::wire->available() < 1) break;
      registers.rwa.mode = endian_read<unsigned char>();

      LOG_INFO_header
      LOG_INFO_println("RWA_MODE set to " + String(registers.rwa.mode))
    }

    case Register::RWA_COMMAND: {
      if (umb::wire->available() < 6) break;
      float f[3];
      unsigned short t[3];
      endian_read<unsigned short, 3>(t);

      if (registers.rwa.mode == RWAMode::RWA_ACCEL_CTRL)
        for (unsigned int i = 0; i < 3; i++)
          f[i] = utl::fp(t[i], rwa::min_torque, rwa::max_torque);
      else if (registers.rwa.mode == RWAMode::RWA_SPEED_CTRL)
        for (unsigned int i = 0; i < 3; i++)
          f[i] = utl::fp(t[i], rwa::min_speed_command, rwa::max_speed_command);
      else
        break;
      
      copy_to(f, registers.rwa.cmd);
      registers.rwa.cmd_flg = CMDFlag::UPDATED;

      LOG_INFO_header
      LOG_INFO_println("RWA_COMMAND set to " + String(f[0]) + " " +
          String(f[1]) + " " + String(f[2]))

      break;      
    }

    case Register::RWA_SPEED_FILTER: {
      if (umb::wire->available() < 1) break;
      registers.rwa.momentum_flt = utl::fp(endian_read<unsigned char>(), 0.0f, 1.0f);

      LOG_INFO_header
      LOG_INFO_println("RWA_SPEED_FILTER set to " + String(registers.rwa.momentum_flt))
    }

    case Register::RWA_RAMP_FILTER: {
      if (umb::wire->available() < 1) break;
      registers.rwa.ramp_flt = utl::fp(endian_read<unsigned char>(), 0.0f, 1.0f);

      LOG_INFO_header
      LOG_INFO_println("RWA_SPEED_FILTER set to " + String(registers.rwa.ramp_flt))

      break;
    }

    case Register::MTR_MODE: {
      if (umb::wire->available() < 1) break;
      registers.mtr.mode = endian_read<unsigned char>();

      LOG_INFO_header
      LOG_INFO_println("MTR_MODE set to " + String(registers.mtr.mode))
    }

    case Register::MTR_COMMAND: {
      if (umb::wire->available() < 6) break;
      float f[3];
      unsigned short t[3];
      endian_read<unsigned short, 3>(t);
      for (unsigned int i = 0; i < 3; i++)
        f[i] = utl::fp(t[i], mtr::min_moment, mtr::max_moment);
      copy_to(f, registers.mtr.cmd);
      registers.mtr.cmd_flg = CMDFlag::UPDATED;

      LOG_INFO_header
      LOG_INFO_println("MTR_COMMAND set to " + String(f[0]) + " " + String(f[1])
          + " " + String(f[2]))
    }

    case Register::MTR_LIMIT: {
      if (umb::wire->available() < 1) break;

      registers.mtr.moment_limit = utl::fp(endian_read<unsigned short>(), mtr::min_moment, mtr::max_moment);
      registers.mtr.cmd_flg = CMDFlag::UPDATED;

      LOG_INFO_header
      LOG_INFO_println("MTR_LIMIT set to " + String(registers.mtr.moment_limit))

      break;
    }

    case Register::SSA_MODE: {
      if (umb::wire->available() < 1) break;
      if (registers.ssa.mode == SSAMode::SSA_COMPLETE ||
          registers.ssa.mode == SSAMode::SSA_FAILURE) {
        registers.ssa.mode = endian_read<unsigned char>();
      }

      LOG_INFO_header
      LOG_INFO_println("SSA_MODE set to " + registers.ssa.mode)

      break;
    }

    case Register::SSA_VOLTAGE_FILTER: {
      if (umb::wire->available() < 1) break;
      registers.ssa.voltage_flt = utl::fp(endian_read<unsigned char>(), 0.0f, 1.0f);

      LOG_INFO_header
      LOG_INFO_println("SSA_VOLTAGE_FILTER set to " + String(registers.ssa.voltage_flt))

      break;
    }

    case Register::IMU_MODE: {
      if (umb::wire->available() < 1) break;
      registers.imu.mode = endian_read<unsigned char>();

      LOG_INFO_header
      LOG_INFO_println("IMU_MODE set to " + String(registers.imu.mode))

      break;
    }

    case Register::IMU_MAG_FILTER: {
      if (umb::wire->available() < 1) break;
      registers.imu.mag_flt = utl::fp(endian_read<unsigned char>(), 0.0f, 1.0f);

      LOG_INFO_header
      LOG_INFO_println("IMU_MAG_FILTER set to " + String(registers.imu.mag_flt))
    }

    case Register::IMU_GYR_FILTER: {
      if (umb::wire->available() < 1) break;
      registers.imu.gyr_flt = utl::fp(endian_read<unsigned char>(), 0.0f, 1.0f);

      LOG_INFO_header
      LOG_INFO_println("IMU_GYR_FILTER set to " + String(registers.imu.gyr_flt))
    }

    case Register::IMU_GYR_TEMP_FILTER: {
      if (umb::wire->available() < 1) break;
      registers.imu.gyr_temp_flt = utl::fp(endian_read<unsigned char>(), 0.0f, 1.0f);

      LOG_INFO_header
      LOG_INFO_println("IMU_GYR_TEMP_FILTER set to " + String(registers.imu.gyr_temp_flt))

      break;
    }

    case Register::IMU_GYR_TEMP_KP: {
      if (umb::wire->available() < 4) break;
      registers.imu.gyr_temp_p = endian_read<float>();

      LOG_INFO_header
      LOG_INFO_println("IMU_GYR_TEMP_KP set to " + String(registers.imu.gyr_temp_p))
    }

    case Register::IMU_GYR_TEMP_KI: {
      if (umb::wire->available() < 4) break;
      registers.imu.gyr_temp_i = endian_read<float>();

      LOG_INFO_header
      LOG_INFO_println("IMU_GYR_TEMP_KI set to " + String(registers.imu.gyr_temp_i))
    }

    case Register::IMU_GYR_TEMP_KD: {
      if (umb::wire->available() < 4) break;
      registers.imu.gyr_temp_d = endian_read<float>();

      LOG_INFO_header
      LOG_INFO_println("IMU_GYR_TEMP_KD set to " + String(registers.imu.gyr_temp_d))
      
      break;
    }

    case Register::IMU_GYR_TEMP_DESIRED: {
      if (umb::wire->available() < 1) break;
      registers.imu.gyr_desired_temp = utl::fp(endian_read<unsigned char>(), imu::min_eq_temp, imu::max_eq_temp);

      LOG_INFO_header
      LOG_INFO_println("IMU_GYR_TEMP_DESIRED set to " + String(registers.imu.gyr_desired_temp))

      break;
    }

    case Register::HAVT_COMMAND: {
      if (umb::wire->available() < 1) break;
      registers.havt.cmd_table = endian_read<unsigned int>();
      registers.havt.cmd_flg = CMDFlag::UPDATED;

#if LOG_LEVEL >= LOG_LEVEL_INFO
      std::bitset<havt::max_devices>temp(registers.havt.cmd_table);

      //note 32 = havt::max_devices for clarity
      char buffer[33];
      for(int i = 0; i<32; i++){
        if(temp.test(31-i))
          buffer[i] = '1';
        else
          buffer[i] = '0';
      }
      buffer[32] = '\0';

      LOG_INFO_header
      LOG_INFO_printF("HAVT_COMMAND set to ")
      LOG_INFO_println(buffer);
#endif

      break;
    }
    default: {
      LOG_WARN_header
      LOG_WARN_println("Invalid register written to over I2C: "
          + String(address))
    }
  }
}

void on_i2c_request() {
  unsigned char address = registers.read_ptr;

  LOG_INFO_header
  LOG_INFO_println("Read request over I2C at address " + String(address))

  switch (address) {

    case Register::WHO_AM_I: {
      // Output constant who am I register as 0x0F
      endian_write(registers.who_am_i);

      LOG_INFO_header
      LOG_INFO_println("WHO_AM_I read as " + String(registers.who_am_i))

      break;
    }
    
    case Register::RWA_SPEED_RD: {
      float f[3];
      unsigned short t[3];
      // Ouput reaction wheel angular momentum
      copy_to(registers.rwa.momentum_rd, f);
      for (unsigned int i = 0; i < 3; i++)
        t[i] = utl::us(f[i], rwa::min_speed_read, rwa::max_speed_read);
      endian_write(t);

      LOG_INFO_header
      LOG_INFO_println("RWA_SPEED_RD read as " + String(f[0]) + " " + String(f[1])
          + " " + String(f[2]))

      // Output reaction wheels ramp torques
      copy_to(registers.rwa.ramp_rd, f);
      for (unsigned int i = 0; i < 3; i++)
        t[i] = utl::us(f[i], rwa::min_torque, rwa::max_torque);
      endian_write(t);

      LOG_INFO_header
      LOG_INFO_println("RWA_RAMP_RD read as " + String(f[0]) + " "
          + String(f[1]) + " " + String(f[2]))

      break;
    }

    case Register::SSA_MODE: {
      // Output sun sensor mode
      endian_write(registers.ssa.mode);

      LOG_INFO_header
      LOG_INFO_println("SSA_MODE read as " + String(registers.ssa.mode))

      break;
    }

    case Register::SSA_SUN_VECTOR: {
      float f[3];
      unsigned short t[3];
      // Output calculated sun vector
      copy_to(registers.ssa.sun_vec_rd, f);
      for (unsigned int i = 0; i < 3; i++)
        t[i] = utl::us(f[i], -1.0f, 1.0f);
      endian_write(t);

      LOG_INFO_header
      LOG_INFO_println("SSA_SUN_VECTOR read as " + String(f[0]) + " "
          + String(f[1]) + " " + String(f[2]))

      break;
    }

    case Register::SSA_VOLTAGE_READ: {
      float f[20];
      unsigned char t[20];
      // Output sun sensor voltage measurements
      copy_to(registers.ssa.voltage_rd, f);
      for (unsigned int i = 0; i < 20; i++)
        t[i] = utl::uc(f[i], ssa::min_voltage_rd, ssa::max_voltage_rd);
      endian_write(t);

      LOG_INFO_header
      LOG_INFO_println("SSA_VOLTAGE_READ read as " + String(f[0]) + " "
          + String(f[1]) + " " + String(f[2]) + " ...")

      break;
    }

    case Register::IMU_MAG_READ: {
      float f[3];
      unsigned short t[3];
      // Output magnetic field readings
      copy_to(registers.imu.mag_rd, f);
      for (unsigned int i = 0; i < 3; i++)
        t[i] = utl::us(f[i], imu::min_rd_mag, imu::max_rd_mag);
      endian_write(t);

      LOG_INFO_header
      LOG_INFO_println("IMU_MAG_READ read as " + String(f[0]) + " "
          + String(f[1]) + " " + String(f[2]))

      // Output gyroscope angular rate readings
      copy_to(registers.imu.gyr_rd, f);
      for (unsigned int i = 0; i < 3; i++)
        t[i] = utl::us(f[i], imu::min_rd_omega, imu::max_rd_omega);
      endian_write(t);

      LOG_INFO_header
      LOG_INFO_println("IMU_GYR_READ read as " + String(f[0]) + " "
          + String(f[1]) + " " + String(f[2]))

      // Output gyroscope temperature
      endian_write(utl::us(registers.imu.gyr_temp_rd, imu::min_rd_temp, imu::max_rd_temp));

      LOG_INFO_header
      LOG_INFO_println("IMU_GYR_TEMP_READ read as "
          + String(registers.imu.gyr_temp_rd))

      break;
    }

    case Register::HAVT_READ: {
      //table is stored as an unsigned int, so merely write out to i2c
      endian_write(registers.havt.read_table);

#if LOG_LEVEL >= LOG_LEVEL_INFO
      std::bitset<havt::max_devices> temp_bitset(registers.havt.read_table);

      //note 32 = havt::max_devices for clarity
      char buffer[33];
      for(int i = 0; i<32; i++){
        if(temp_bitset.test(31-i))
          buffer[i] = '1';
        else
          buffer[i] = '0';
      }
      buffer[32] = '\0';

      LOG_INFO_header
      LOG_INFO_printF("HAVT_READ read as ")
      LOG_INFO_println(buffer)
      // You should see the below if nothing is connected, MTRs and WHEELS OK
      // REMEMBER DEVICE INDEX STEPS UP FROM RIGHT TO LEFT
      // Read Internal Table As: 00000000000000000000001110111000
#endif

      break;
    }

    default: {
      LOG_WARN_header
      LOG_WARN_println("Invalid address requested over I2C: "
          + String(address))
    }
  }
}
}  // namespace umb
}  // namespace adcs
