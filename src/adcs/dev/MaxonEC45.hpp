//
// src/adcs/dev/MaxonEC45.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol          kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_DEV_MAXONEC45_HPP_
#define SRC_ADCS_DEV_MAXONEC45_HPP_

#include "AD5254.hpp"
#include "Device.hpp"

#include <Servo.h>

namespace adcs {
namespace dev {

/** @class MaxonEC45
 *  */
class MaxonEC45 : public Device {
 public:
  /** Sets up this mootor on the following clockwise enable, counterclockwise
   *  enable, speed servo pin, potentiometer, and potentiometer rdac channel. */
  void setup(unsigned int cw_pin, unsigned int ccw_pin, unsigned int speed_pin,
             AD5254 *potentiometer, void (AD5254::*const set_r)(uint8_t));
  /** Sets the speed to zero, ramp to zero, disable the clockwise pin, and
   *  disables the counterclockwise pin then marks the device functional. 
   *  @return True. */
  virtual bool reset() override;
  /** Performs the same function as reset expect the device is marked as not
   *  functional. */
  virtual void disable() override;
  /** */
  inline void set_speed(int speed) { this->speed = speed; }
  /** */
  inline int get_speed() const { return this->speed; }
  /** */
  inline void set_axl_ramp(uint8_t axl_ramp) { this->axl_ramp = axl_ramp; }
  /** */
  inline uint8_t get_axl_ramp() const { return this->axl_ramp; }
  /** */
  void actuate();
  /** */
  void stop();

 private:
  /** Clockwise rotation enable pin. */
  unsigned int cw_pin;
  /** Counterclockwise rotation enable pin. */
  unsigned int ccw_pin;
  /** Speed servo. */
  Servo servo;
  /** Potentiometer in control of the analog ramp. */
  AD5254 *potentiometer;
  /** Analog ramp set function. */
  void (AD5254::*set_r)(uint8_t r);
  /** Current motor speed setting. */
  int speed;
  /** Current acceleration ramp setting. */
  uint8_t axl_ramp;
};
}  // namespace dev
}  // namespace adcs

#endif
