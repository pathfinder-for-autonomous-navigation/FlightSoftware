/** @file DeviceManager.hpp
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains declaration for DeviceManager, which is a helper class for accessing/controlling
 * multiple devices at the same time.
 */

#ifndef PAN_DEVICES_DEVICEMANAGER_HPP_
#define PAN_DEVICES_DEVICEMANAGER_HPP_

/*   The DeviceManager class is intended to serve as a single location where
 * references to all of a microcontrollers devices are listed. This is
 * convenient for high level functionality like comms frames, device enabling &
 * disabling, etc.
 *   The intended use case is as follows:
 *  1. Create a constant vector of pointers to all of the devices conencted to
 *     the microcontroller.
 *  2. Enum the device identifiers (position in the vector) for easy, efficient
 *     access - this also makes static casting via the get_device function
 *     easier to perform.
 *  3. Pass a reference of the device pointer vector to the DeviceManager
 *     constructor.
 */

#include "Device.hpp"
#include <stdint.h>

#include <vector>

namespace Devices {
/** Data structure to hold all devices interface with a given microcontroller.
 *  The devices are mapped with a number (i.e. there location in the provided
 *  vector reference). **/
class DeviceManager {
 public:
  /** @brief Constructs a device manager with the specified Device * vector. **/
  DeviceManager(std::vector<Device *> const &devices);
  /** @brief Returns a device from the array and casts it to the templated type.
   *  @returns a reference to the desired device.
   * **/
  template <class Device>
  inline Device &get_device(std::size_t i);
  /** @brief Returns the device count.
   *  @returns device count. **/
  inline std::size_t get_device_count() const;
  /** @brief Executes a function on the specified range of devices - note the
   *         lower bound is inclusive and the upper bound is exclusive. **/
  template <class Device>
  inline void for_each(void (*func)(Device &), unsigned char e);

  bool device_setup_fault; // True if there is a fault in setting up devices.
 protected:
  /** Device pointer vector. **/
  std::vector<Device *> const &devices;
};
}  // namespace Devices

#include "DeviceManager.inl"

#endif
