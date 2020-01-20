//
// lib/MMC34160PJ/MMC34160PJ.hpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO : Calibrate function

#ifndef PAN_ADCS_LIB_MMC34160PJ_MMC34160PJ_HPP_
#define PAN_ADCS_LIB_MMC34160PJ_MMC34160PJ_HPP_

/** @addtogroup lib
 *  @} */

#include <I2CDevice.hpp>

/** @class MMC34160PJ
 *  Driver operating the MMC34160PJ magnetometer in single shot mode with
 *  variable sample rate. There is also a calibration function to handle
 *  temperature/intrinsic bias. */
class MMC34160PJ : public dev::I2CDevice {
 public:
  /** @enum REG
   *  Enumerates all possible register addresses for the magnetometer. */
  enum REG : uint8_t {
   OUT = 0x00,
   XOUT = 0x00,
   XOUT_LOW = 0x00,
   XOUT_HIGH = 0x01,
   YOUT = 0x02,
   YOUT_LOW = 0x02,
   YOUT_HIGH = 0x03,
   ZOUT = 0x04,
   ZOUT_LOW = 0x04,
   ZOUT_HIGH = 0x05,
   STATUS = 0x06,
   INTERNAL_CONTROL_0 = 0x07,
   INTERNAL_CONTROL_1 = 0x09,
   PRODUCT_ID = 0x20 // 0x06
  };
  /** @enum ST
   *  Enumerates all possible sample rates for the magnetometer. */
  enum SR : uint8_t {
    /** Sample rate of 1.5 Hz. */
    HZ_1_5 = 0b0000,
    /** Sample rate of 13 Hz. */
    HZ_13 = 0b0100,
    /** Sample rate of 25 Hz. */
    HZ_25 = 0b1000,
    /** Sample rate of 50 Hz. THis is the default sample rate. */
    HZ_50 = 0b1100
  };
  /** See the \c I2CDevice class for more details. The address defaults to 0x30
   *  for this device. */
  void setup(i2c_t3 *wire, unsigned long timeout = DEV_I2C_TIMEOUT);
  /** Places the device in continous measurement mode with the current sample
   *  rate setting. If this is succesful then the device is marked as
   *  functional.
   *  @return True if the reset was succesful and false otherwise. */
  virtual bool reset() override;
  /** Attempts to disable continuous measurement mode. */
  virtual void disable() override;
  /** Calibrates the magnetometer and updates the offset parameter. This process
   *  will take about 11 ms and should be done periodically to ensure accuracy.
   *  The device will be marked as disabled and the \c reset function must be
   *  called to resume readings.
   *  @returns True if the calibration was succesful and false otherwise. */
  void calibrate();
  /** @return True if a new read is complete and false otherwise. */
  bool is_ready();
  /** Reads in data from the most recently completed read. The data is stored
   *  in the magnetometer's data backing array.
   *  @return true for a successful read and false otherwise. */
  bool read();
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

 private: 
  /** @internal Fills the set/reset capacitor over the course of 50 ms. */
  void fill_capacitor();
  /** @internal Triggers the set operation. */
  void set_operation();
  /** @internal Triggers the reset operation. */
  void reset_operation();
  /** @internal Triggers a single shot read of the magnetometer.
   *  @return True if succesful and false otherwise. */
  bool single_read(uint16_t *array);
  /** Magnetic field backing vector. */
  uint16_t b_vec[3];
  /** Magnetic field sample rate. */
  uint8_t sample_rate;
  /** Magnetomter offset calibration value. */
  uint16_t offset[3];
};

/** @} */

#endif
