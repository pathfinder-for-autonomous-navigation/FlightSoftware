/** @file Device.hpp
 * @author Tanishq Aggarwal
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains declaration for Device interface, which standardizes the
 * access
 * and control of hardware peripherals.
 */

#include <common/Nameable.hpp>

/** \addtogroup Devices
 *  @{ **/

#ifndef PAN_DEVICES_DEVICE_HPP_
#define PAN_DEVICES_DEVICE_HPP_

/** \namespace Devices
 *  @brief Contains all of the general device functionality shared between the
 *         flight and ADCS computers.
 *
 *  This interface ensures the following functionality: a setup functions to be
 *  called on device initiation, a function which specifies whether or not the
 *  device is currently operating properly, a hard reset function, a function
 *  to disable to device, and a function to print testing data over Serial. **/
namespace Devices {
/** \interface Device
 *  @brief Interface from which all peripherial devices will be derived.
 *
 *  This interface ensures that all peripherials in communication with a
 *  flight computer have common functionality. This will be most useful for
 *  communications downlinks and updates on the satellites health. **/
class Device : public Nameable {
   protected:
    const std::string _name;

   public:
    Device(const std::string& name) : _name(name) {}

    const std::string& name() const override { return _name; }

    ~Device();
    /** @brief Sets up communication with the device and verifies
     *         the device is responding to communication attempts.
     *
     *
     *  @returns True if device is working properly, false otherwise. **/
    virtual bool setup();
    /** @brief Verifies the device is responding to communications.
     *  @returns True if device is responding to communications, false otherwise.
     * **/
    virtual bool is_functional();
    /** @brief Attempts to reset a non-functional device. All error state
     *         variables should be reset. In most cases, this should only be
     *         be called as the result of a ground originated command. **/
    virtual void reset();
    /** @brief Disables a device regardless of it's current error state. In most
     *         cases, this should only be called as the result of a ground
     *         originated command. **/
    virtual void disable();
};
}  // namespace Devices

#endif

/** @} **/
