/** @file DeviceManager.inl
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains implementation for DeviceManager, which is a helper class for accessing/controlling
 * multiple devices at the same time.
 */

#ifndef PAN_DEVICES_DEVICEMANAGER_INL_
#define PAN_DEVICES_DEVICEMANAGER_INL_

#include <stdint.h>
#include "Device.hpp"
#include "DeviceManager.hpp"

namespace Devices {

DeviceManager::DeviceManager(std::vector<Device *> const &devices)
    : devices(devices) {
  // empty
}

template <class Device>
inline Device &DeviceManager::get_device(std::size_t i) {
  return *static_cast<Device *>(this->devices[i]);
}

inline std::size_t DeviceManager::get_device_count() const {
  return this->devices.size();
}

template <class Device>
inline void DeviceManager::for_each(void (*func)(Device &), unsigned char e) {
  for (std::size_t i = 0; i < e / sizeof(Device&); i++)
    func(*static_cast<Device *>(this->devices[i]));
}
}  // namespace Devices

#endif
