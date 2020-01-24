//
// src/adcs/dev/Magnetorquer.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_DEV_MAGNETORQUER_HPP_
#define SRC_ADCS_DEV_MAGNETORQUER_HPP_

#include "Device.hpp"

namespace adcs {
namespace dev {

/** \class MTR
 *  Encapsulates the signed PWM actuation behavior of a magnetorquer and
 *  abstracts away the H-bridge. */
class Magnetorquer : public Device {
 public:
  /** Sets the forward and backward pins associated with this magnetorquer. */
  void setup(unsigned int f_pin, unsigned int r_pin);
  /** Sets the PWM value to zero and marks the magnetorquer as functional.
   *  @return True. */
  virtual bool reset() override;
  /** Sets the PWM value to zero and marks the magnetorquer as disabled. */
  virtual void disable() override;
  /** Actuates the MTR with the specified signed PWM value. A negative PWM value
   *  corresponds to an actuation in the reverse direction and positive
   *  corresponds to the forward direction.
   *  @param[in] signed_pwm Requested PWM value. */
  void actuate(int signed_pwm);

 private:
  /** Forward enable pin. */
  unsigned int f_pin;
  /** Reverse enable pin. */
  unsigned int r_pin;
};
}  // namespace dev
}  // namespace adcs

#endif
