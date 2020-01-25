//
// src/adcs/dev/LIS2MDLTR.hpp
// FlightSoftware
//
// Contributors:
//   Anusha Choudhury ac978@cornell.edu
//   Kyle Krol        kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_DEV_LIS2MDLTR_HPP_
#define SRC_ADCS_DEV_LIS2MDLTR_HPP_

#include "I2CDevice.hpp"

namespace adcs {
namespace dev {

/** @class LIS2MDLTR
 *  Driver operating the LIS2MDLTR magnetometer in continuous mode with variable
 *  sample rate. There is also a calibration function to handle
 *  temperature/intrinsic bias. */
class LIS2MDLTR : public I2CDevice {
 public:
  /** @enum REG
   *  Possible addresses for the registers of the magnetometer. **/
  enum REG : uint8_t  {
    OUT = 0x68,
    XOUT = 0x68,
    XOUT_LOW = 0x68,
    XOUT_HIGH = 0x69,
    YOUT = 0x6A,
    YOUT_LOW = 0x6A,
    YOUT_HIGH = 0x6B,
    ZOUT = 0x6C,
    ZOUT_LOW = 0x6C,
    ZOUT_HIGH = 0x6D,
    TEMP= 0x6E,
    TEMP_LOW = 0x6E,
    TEMP_HIGH = 0x6F,
    STATUS = 0x67,
    CFG_A = 0x60,
    CFG_B = 0x61,
    CFG_C = 0x62,
    WHO_AM_I = 0x4F
  };
  /** @enum SR
   *  Enumerates all possible sample rates for the magnetometer. */
  enum SR : uint8_t {
    /** Sample rate of 10 Hz. */
    HZ_10 = 0b0000,
    /** Sample rate of 20 Hz. */
    HZ_20 = 0b0100,
    /** Sample rate of 50 Hz. */
    HZ_50 = 0b1000,
    /** Sample rate of 100 Hz. THis is the default sample rate. */
    HZ_100 = 0b1100
  };
  /** See the \c I2CDevice class for more details. The address defaults to 0x30
   *  for this device. */
  void setup(i2c_t3 *wire, unsigned long timeout = DEV_I2C_TIMEOUT);
  /** Places the device in continous measurement mode with the current sample
   *  rate setting. If this is succesful then the device is marked as
   *  functional.
   *  @return True if the reset was succesful and false otherwise. */
  virtual bool reset() override;
  /** Attempts to place the device in idle mode and marks it disabled. */
  virtual void disable() override;
  /** @return True if a new read is complete and false otherwise. */
  bool is_ready();
  /** Reads in data from the most recently completed read. The data is stored
   *  in the magnetometer's data backing array.
   *  @return true for a successful read and false otherwise. */
  bool read();

  // TODO
  /** */
  void calibrate();

  /** Sets the sample rate of the magnetometer. Note that the \c reset function
   *  must be called for a new sample rate to be applied. */
  inline void set_sample_rate(SR sample_rate) { this->sample_rate = sample_rate; }
  /** @return Most recent sample rate setting. */
  inline uint8_t get_sample_rate() const { return this->sample_rate; }
  /** @return Most recent magnetic field x component in the frame of the
   *  magnetometer. */
  inline int16_t get_b_x() const { return this->b_vec[0]; }
  /** @return Most recent magnetic field y component in the frame of the
   *  magnetometer. */
  inline int16_t get_b_y() const { return this->b_vec[1]; }
  /** @return Most recent magnetic field z component in the frame of the
   *  magnetometer. */
  inline int16_t get_b_z() const { return this->b_vec[2]; }
  
  //static constexpr uint8_t SINGLE = 0x8D;
  //static constexpr uint8_t OFF_CANC = 0x12;
  /** Checks if there exists a new read for the first data array.
   * If it exists it puts the read in the specified data array, otherwise the read is 0.
   **/
  //virtual void new_read_1();
  /** Checks if there exists a new read for the first data array.
   * If it exists it puts the read in the specified data array, otherwise the read is 0.
   **/
  //virtual void new_read_2();
  /**Constructor**/
  // Magnetometer(i2c_t3 &wire, uint8_t ADDR, unsigned long timeout =0);
  // Reads the average x component of the magnetic field in milliGauss
  //inline int16_t get_read_x() const {return this->rdata[0] + this->sdata[0];}
  // Reads the average y component of the magnetic field in milliGauss
  //inline int16_t get_read_y() const {return this->rdata[1]+this->sdata[1];}
  // Reads the average z component of the magnetic field in milliGauss
  //inline int16_t get_read_z() const {return this->rdata[2] + this->sdata[2];}

 private:
  /** Magnetic field backing vector. */
  uint16_t b_vec[3];
  /** Magnetic field sample rate. */
  uint8_t sample_rate;
};
}  // namespace dev
}  // namespace adcs

#endif
