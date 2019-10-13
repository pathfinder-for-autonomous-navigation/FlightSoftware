#ifndef DOCKING_MOTOR_HPP_
#define DOCKING_MOTOR_HPP_

#include "../Devices/Device.hpp"

namespace Devices {
/**
 * @brief Logical interface to docking motor.
 * 
 * The flight software is responsible for determining if the system is docked
 * or not.
 * **/
class DockingSystem : public Device {
   public:
    //! Default pin for docking motor I1.
    static constexpr unsigned char motor_i1_pin = 14;
    //! Default pin for docking motor I2.
    static constexpr unsigned char motor_i2_pin = 15;
    //! Default pin for docking motor direction pin.
    static constexpr unsigned char motor_direction_pin = 16;
    //! Default pin for docking motor sleep pin.
    static constexpr unsigned char motor_sleep_pin = 17;
    //! Default pin for docking motor step.
    static constexpr unsigned char motor_step_pin = 39;
    //! Docking switch pin
    static constexpr unsigned char switch_pin = 25;

    /**
     * @brief Construct a new Docking System object.
     */
    DockingSystem();

    bool setup() override;
    bool is_functional() override;
    void disable() override;
    void enable();
    void reset() override;

    /** @brief Turn the docking motor into the "docked" configuration.
     **/
    void dock();

    /** @brief Turn the docking motor into the "undocked" configuration.
     **/
    void undock();

    /**
     * @brief Check state of docking switch.
     * 
     * @return True if switch is pressed.
     */
    bool DockingSystem::check_docked() const;

    /**
     * @brief Adjust rotation parameter for docking motor.
     * 
     * @param parameter
     */
    void DockingSystem::set_rotation_parameter(unsigned int parameter);

   private:
    // Sets how long the motors need to rotate to change between the "docked"
    // and "undocked" states. 
    unsigned int rotation_parameter;

    // Status of motor sleep pin (and therefore of overall docking motor.)
    bool is_enabled;
};
}  // namespace Devices

#endif
