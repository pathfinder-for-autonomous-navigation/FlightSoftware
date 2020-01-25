//
// src/adcs/dev/I2CDevice.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_DEV_I2CDEVICE_HPP_
#define SRC_ADCS_DEV_I2CDEVICE_HPP_

#include "Device.hpp"

#include <i2c_t3.h>

#ifndef DEV_I2C_ERROR_COUNT
/** Specifies the number of consecutive failures that marks an \c I2CDevice as
 *  not functional. Can be changed by specifying this macro to the compiler. */
#define DEV_I2C_ERROR_COUNT 3
#endif

#ifndef DEV_I2C_TIMEOUT
/** Specifies the defaul I2C device timeout in microseconds. Can be changed by
 *  specifying this macro to the compiler. */
#define DEV_I2C_TIMEOUT 10000
#endif

namespace adcs {
namespace dev {

/** @class I2CDevice
 *  Defines a common interface and error tracking behavior for all peripherials
 *  communicating over I2C. The function calls are largely based on the i2c_t3
 *  library which can be found at this URL: https://github.com/nox771/i2c_t3. */
class I2CDevice : public Device {
 public:
  /** Marks the device as functional to allow it to resume operation. \c setup
   *  should be used before this function is called. It's intended to be
   *  overridden for more complicated devices.
   *  @return True if the reset was succesful and false otherwise. */
  virtual bool reset() override;
  
 protected:
  /** Initializes the device as not funcitonal. */
  I2CDevice();
  /** Initializes the device by setting it's I2C bus, address, and timeout
   *  values in microseconds. The device still needs to be reset before being
   *  used. */
  void setup(i2c_t3 *wire, uint8_t addr, unsigned long timeout);
  /** Specifies whether or not an error has occurred without resetting the error
   *  accumulator.
   *  @return True if an error has occurred and false otherwise. */
  inline bool i2c_peek_errors();
  /** Specifies whether or not as error has occurred and resets the error
   *  accumulator. If this is the \c DEV_I2C_ERROR_COUNT consecutive error, then
   *  the device is autonomously disabled.
   *  @return True if an error has occurred and false otherwise. */
  inline bool i2c_pop_errors();
  /** Inlined call to \c beginTransmission from the i2c_t3 library. This
   *  device's I2C address is passed as an argument. */
  inline void i2c_begin_transmission();
  /** Inlined call to \c endTransmission from the i2c_t3 library. Any errors are
   *  added to the error accumulator.
   *  @param[in] s End the transmission with a STOP or NOSTOP bit. */
  inline void i2c_end_transmission(i2c_stop s = I2C_STOP);
  /** Inlined call to \c sendTransmission from the i2c_t3 library. This is the
   *  non-blocking equivalent of \c i2c_end_transmission and requires a call to
   *  \c i2c_finish.
   *  @param[in] s End the transmission with a STOP or NOSTOP bit. */
  inline void i2c_send_transmission(i2c_stop s = I2C_STOP);
  /** Inlined call to \c requestFrom from the i2c_t3 library. Any errors are
   *  added to the error accumulator. 
   *  @param[in] len Number of bytes requested.
   *  @param[in] s End the transmission with a STOP or NOSTOP bit. */
  inline void i2c_request_from(unsigned int len, i2c_stop s = I2C_STOP);
  /** Inlined call to \c sendRequest from the i2c_t3 library. This is the
   *  non-blocking equivalent of \c i2c_request_from and requires a call to \c
   *  i2c_finish.
   *  @param[in] len Number of bytes requested.
   *  @param[in] s End the transmission with a STOP or NOSTOP bit. */
  inline void i2c_send_request(unsigned int len, i2c_stop s = I2C_STOP);
  /** Inlined call to \c done from the i2c_t3 library.
   *  @return False if a non-blocking I2C command is still running on this bus
   *          and true otherwise. */
  inline bool i2c_done() const;
  /** Inlined call to \c finish from the i2c_t3 library. Any errors are added to
   *  the error accumulator. */
  inline void i2c_finish();
  /** Inlined call to \c write from the i2c_t3 library. Any errors are added to
   *  the error accumulator.
   *  @oaram[in] data Byte to be written. */
  inline void i2c_write(uint8_t data);
  /** Templated inlined call to \c write from the i2c_t3 library. Any errors are
   *  added to the error accumulator. The endianess is determined by the
   *  platform the code is being run on.
   *  @tparam T Data type of the array.
   *  @param[in] data Source array for outgoing data.
   *  @param[in] len Length of the array in terms of the type \c T. */
  template <typename T>
  inline void i2c_write(T const *data, unsigned int len);
  /** Inlined call to \c readByte from the i2c_t3 library. Any errors are added
   *  to the error accumulator.
   *  @return Byte value read in over I2C. */
  inline uint8_t i2c_read();
  /** Templated inlined call to \c read from the i2c_t3 library. Any errors are
   *  added to the error accumulator.
   *  @tparam T Data type of the array.
   *  @param[out] data Destination array for incoming data.
   *  @param[in] len Length of the array in terms of the type \c T. */
  template <typename T>
  inline void i2c_read(T *data, unsigned int len);

 private:
  /** Pointer to the I2C bus this I2C device communicated on. */
  i2c_t3 *wire;
  /** Address for this I2C device. */
  uint8_t addr;
  /** I2C timeout value for this I2C device in microseconds. */
  unsigned long timeout;
  /** Consecutive error counter. */
  unsigned int error_count;
  /** Error accumulator. */
  bool error_acc;
};
}  // namespace dev
}  // namespace adcs

#include "I2CDevice.inl"

#endif
