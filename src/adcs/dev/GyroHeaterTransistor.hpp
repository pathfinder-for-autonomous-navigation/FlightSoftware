//
// lib/GyroHeaterTransistor/GyroHeaterTransistor.hpp
// ADCS
//
// Contributors:
//   Tanishq Aggarwal (ta335@cornell.edu)
//   Shihao Cao (sfc72@cornell.edu)
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef ADCS_LIB_IMU_GYROHEATERTRANSISTOR_HPP_
#define ADCS_LIB_IMU_GYROHEATERTRANSISTOR_HPP_

#include "Device.hpp"

namespace adcs{
namespace dev{
/** \class GyroHeaterTransistor
 *  Encapsulates the gyroscope heater transistor. */
class GyroHeaterTransistor : public Device {
 public:
  /** Sets the pin associated with this transistor. */
  void setup(unsigned int pin);

  /** Sets pwm to 0, and resets the heater.
   *  @return True. */
  virtual bool reset() override;

  /** Sets pwm to 0 and disables the heater.*/
  virtual void disable() override;

  /** Pwms the pin with the given pwm iff on is true. */
  void actuate(const int pwm, bool on);

 private:
  /** Connected pin. */
  unsigned int pin;
};
}
}

#endif
