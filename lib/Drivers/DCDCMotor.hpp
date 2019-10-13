#ifndef DCDC_MOTOR_HPP_
#define DCDC_MOTOR_HPP_

#include "../Devices/Device.hpp"

namespace Devices {
/**
 * @brief Enables or disables the 7-24V DC-DC converters.
 * **/
class DCDCMotor : public Device {
   public:
    //! Default enable pin for DCDC.
    static constexpr unsigned char dcdc_motor_enable_pin = 24;

    /** @brief Default constructor. Loads a set of hardcoded pins into the valve
     * table.**/
    DCDCMotor(const std::string &name, unsigned char en);

    bool setup() override;
    bool is_functional() override;
    void disable() override;
    void reset() override;

    /** @brief Turn on DCDC by holding the enable pin high. **/
    void enable();

   private:
    //! Whether or not Spike and Hold is on or not.
    bool is_enabled;
};
}  // namespace Devices

#endif