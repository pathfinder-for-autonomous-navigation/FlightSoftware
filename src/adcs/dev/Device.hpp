//
// src/adcs/dev/Device.hpp
// FlightSoftware
//
// Contributors:
//   Tanishq Aggarwal  ta335@cornell.edu
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_DEV_DEVICE_HPP_
#define SRC_ADCS_DEV_DEVICE_HPP_

namespace adcs {
namespace dev {

/** @class Device
 *  Defines generic behavior all ADCS peripherials share. This includes a setup
 *  function, reset function, disable function, and an is functional check. */
class Device {
 public:
  /** Resets the device by setting the \c functional variable to true. It's
   *  intended to be overriden for more complicated devices.
   *  @return true if the reset is succesful. */
  virtual bool reset() { return this->functional = true; }
  /** Disables the device by setting the \c functional variable to false. It's
   *  intended to be overriden for more complicated devices. */
  virtual void disable() { this->functional = false; }
  /** @return True if the devices is functional and false otherwise. */
  inline bool is_functional() { return this->functional; }

 protected:
  /** Initializes the device as not functional. */
  Device() : functional(false) { }

 private:
  /** Boolean variable holding true if the device is functional and false 
   *  otherwise. */
  bool functional;
};
}  // namespace dev
}  // namespace adcs

#endif
