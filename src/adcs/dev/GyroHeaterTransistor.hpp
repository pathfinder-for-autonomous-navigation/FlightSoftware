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

#ifndef ADCS_LIB_IMU_GYROHEATERTRANSISTPR_HPP_
#define ADCS_LIB_IMU_GYROHEATERTRANSISTPR_HPP_

#include "Device.hpp"

namespace adcs{
namespace dev{
/** \class GyroHeaterTransistor
 *  Encapsulates the gyroscope heater transistor. */
class GyroHeaterTransistor : public Device {
 public:
  /** Sets the pin associated with this transistor. */
  void setup(unsigned int pin);
  /** Turns off the heater.
   *  @return True. */
  virtual bool reset() override;
  /** Turns off the heater. */
  virtual void disable() override;
  /** Turns on the heater. */
  void actuate();

 private:
  /** Connected pin. */
  unsigned int pin;
};
}
}

#endif
