//
// Devices/SerialDevice.hpp
// PAN
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef PAN_DEVICES_SERIALDEVICE_HPP_
#define PAN_DEVICES_SERIALDEVICE_HPP_

/*
 */

#include "Device.hpp"

#include <HardwareSerial.h>

namespace Devices {
class SerialDevice : public Device {
 public:
 protected:
  /** @brief Constructs the serial device with an infinite timeout and the
   *         specified serial port **/
  SerialDevice(HardwareSerial &serial, unsigned long timeout = 0);

 private:
  /** Timeout value in milliseconds - 0 gives infinite wait **/
  unsigned long timeout;
  /** Timestamp of most recent response request **/
  unsigned long req_timestamp;
  /** Offset to protect millis timer overflow **/
  unsigned long offset;
  /** Flag indicating the device isn't responding **/
  bool unresponsive;
};
}  // namespace Devices

#endif
