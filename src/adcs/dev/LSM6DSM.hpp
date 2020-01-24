//
// src/adcs/dev/LSM6DSM.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_DEV_LSM6DSM_HPP_
#define SRC_ADCS_DEV_LSM6DSM_HPP_

// TODO : make sure the driver resets setting upon failure
// Checking data ready will be false until the gryo filter has settled
// Implement reset and disable

// Outputs unsigned in the range of +/- 16 gauss

#include "I2CDevice.hpp"

namespace adcs {
namespace dev {

/** @class LSM6DSM
 *  Driver to interface with the gyroscope portion of the LSM6DSM. This driver
 *  operates the IC with a sample rate of 52 Hz to match the on board
 *  temperature sensor. */
class LSM6DSM : public I2CDevice {
 public:
  /** @enum ADDR
   *  Enumerates the possible slave addresses for the gyroscope. */
  enum ADDR : uint8_t {
    /** Used when the SA0 pin is grounded. */
    GND = 0b1101010,
    /** Used when the SA0 pin is connected to Vcc. */
    VCC = 0b1101011
  };
  /** @enum REG
   *  Enumerates all the register addresses for the gyroscope. */
  enum REG : uint8_t {
    WHO_AM_I = 0x0F,  // should be = 0x6A
    CTRL1_XL = 0x10,
    CTRL2_G = 0x11,
    CTRL3_C = 0x12,
    CTRL4_C = 0x13,
    CTRL5_C = 0x14,
    CTRL6_C = 0x15,
    CTRL7_G = 0x16,
    CTRL8_XL = 0x17,
    CTRL9_XL = 0x18,
    CTRL10_C = 0x19,
    STATUS_REG = 0x1E,
    OUT_TEMP_L = 0x20,
    OUT_TEMP_H = 0x21,
    OUTX_L_G = 0x22,
    OUTX_H_G = 0x23,
    OUTY_L_G = 0x24,
    OUTY_H_G = 0x25,
    OUTZ_L_G = 0x26,
    OUTZ_H_G = 0x27
  };
  /** See the \c I2CDevice class for implementation details. */
  void setup(i2c_t3 *wire, uint8_t addr, unsigned long timeout = DEV_I2C_TIMEOUT);
  /** Reconfigures the gyroscope sample rate and mode settings.If this
   *  is succesful, the device is enabled.
   *  @return True if the reset was succesful and false otherwise. */
  virtual bool reset() override;
  /** Attempts the set the gyroscope to power down mode before marking it as not
   *  functional. */
  virtual void disable() override;
  /** @return True if data is ready and false otherwise. */
  bool is_ready();
  /** Checks if the data ready bit is high. If so, a new gyroscope angular
   *  velocity and temperature reading will be gathered.
   *  @return true if new data was gathered and false otherwise. */
  bool read();
  /** @return Current x component of the angular velocity in the frame of the
   *           gyroscope. */
  inline int16_t get_omega_x() const { return this->omega[0]; }
  /** @return Current y component of the angular velocity in the frame of the
   *           gyroscope. */
  inline int16_t get_omega_y() const { return this->omega[1]; }
  /** @return Current z component of the angular velocity in the frame of the
   *           gyroscope. */
  inline int16_t get_omega_z() const { return this->omega[2]; }
  /** @return Current gyroscope temperature reading. */
  inline uint16_t get_temp() const { return this->temp; }

 private:
  /** Gyroscope angular momentum vector. */
  int16_t omega[3];
  /** Gyroscope temperature value. */
  int16_t temp;
};
}  // namespace dev
}  // namespace adcs

#endif
