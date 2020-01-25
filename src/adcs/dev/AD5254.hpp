//
// src/adcs/dev/AD5254.hpp
// FlightSoftware
//
// Contributors:
//   Nathan Zimmerberg  nhz2@cornell.edu
//   Kyle Krol          kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_DEV_AD5254_HPP_
#define SRC_ADCS_DEV_AD5254_HPP_

#include "I2CDevice.hpp"

#ifndef AD5254_RDAC_DEFAULT
/** Specifies the defaul rdac value for the AD5254 potentiometer. Can be changed
 *  by specifying this macro to the compiler. */
#define AD5254_RDAC_DEFAULT 0x00
#endif

namespace adcs {
namespace dev {

/** @class AD5254
 *  Simple driver to adjust potentiometer resistances of the AD5254. **/
class AD5254 : public I2CDevice {
 public:
  /** @enum ADDR
   *  Possible addresses for the potentiometer. **/
  enum ADDR {
    A00 = 0b0101100,
    A01 = 0b0101101,
    A10 = 0b0101110,
    A11 = 0b0101111
  };
  /** Sets up the potentiometer on the specified bus with the given settings.
   *  See the \c I2CDevice class documentation for more details. */
  void setup(i2c_t3 *wire, uint8_t addr, unsigned long timeout = DEV_I2C_TIMEOUT);
  /** Sets the rdac values to their default value and if the communication is
   *  succesfull then the device is enabled.
   *  @return True if the reset was succesful and false otherwise. */
  virtual bool reset() override;
  /** Attempts to set the potentiometer to it's default impedance before marking
   *  it as not functional. **/
  virtual void disable() override;
  /** Sets the zeroth rdac setting. **/
  inline void set_rdac0(uint8_t rdac0) { this->rdac[0] = rdac0; }
  /** @return Zeroth zeroth setting. **/
  inline uint8_t get_rdac0() const { return this->rdac[0]; }
  /** Sets the first rdac setting. **/
  inline void set_rdac1(uint8_t rdac1) { this->rdac[1] = rdac1; }
  /** @return First rdac setting. **/
  inline uint8_t get_rdac1() const { return this->rdac[1]; }
  /** Sets the second rdac setting. **/
  inline void set_rdac2(uint8_t rdac2) { this->rdac[2] = rdac2; }
  /** @return Second rdac setting. **/
  inline uint8_t get_rdac2() const { return this->rdac[2]; }
  /** Sets the third rdac setting. **/
  inline void set_rdac3(uint8_t rdac3) { this->rdac[3] = rdac3; }
  /** @return Third rdac setting. **/
  inline uint8_t get_rdac3() const { return this->rdac[3]; }
  /** Starts writing the rdac settings with a non-blocking I2C call. */
  void start_write_rdac();
  /** Completes a hanging rdac \c start_write_rdac call.
   *  @return True if successful and false otherwise. */
  bool end_write_rdac();
  /** Updates the potentiometer's rdac settings.
   *  @return True if successful and false otherwise. **/
  bool write_rdac();

 private:
  /** Backing array for rdac settings. **/
  uint8_t rdac[4];
};
}  // namespace dev
}  // namespace adcs

#endif
