#ifndef DOCKING_SYSTEM_HPP_
#define DOCKING_SYSTEM_HPP_

#include "../Devices/Device.hpp"
#include <cmath>
#include <common/constant_tracker.hpp>

#ifndef DESKTOP
#include <Arduino.h>
#endif

namespace Devices {
/**
 * @brief Logical interface to docking motor.
 * 
 * The flight software is responsible for determining if the system is docked
 * or not.
 **/
class DockingSystem : public Devices::Device {
   public:
    //! Default pin for docking motor I1.
    TRACKED_CONSTANT_SC(unsigned char, motor_i1_pin, 14);
    //! Default pin for docking motor I2.
    TRACKED_CONSTANT_SC(unsigned char, motor_i2_pin, 15);
    //! Default pin for docking motor direction pin.
    TRACKED_CONSTANT_SC(unsigned char, motor_direction_pin, 16);
    //! Default pin for docking motor sleep pin.
    TRACKED_CONSTANT_SC(unsigned char, motor_sleep_pin, 17);
    //! Default pin for docking motor step.
    TRACKED_CONSTANT_SC(unsigned char, motor_step_pin, 39);
    //! Docking switch pin
    TRACKED_CONSTANT_SC(unsigned char, switch_pin, 36);

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
     * @brief Manually step motor by a given amount.
     * 
     * @param angle in degrees.
     */
    void step_motor(float angle);

    /** 
     * @brief Turn the motor shaft by 180 degrees.
     **/
    void start_halfturn();

    /**
     * @brief Manually step motor by one step.
     */
    static void step_motor();

    /**
     * @brief Cancel stepping of motor.
     */
    void cancel();

    /**
     * @brief Return current step angle.
     * 
     * @param angle in degrees.
     */
    float get_step_angle() const;

    /**
     * @brief Check state of docking switch.
     * 
     * @return True if switch is pressed.
     */
    bool check_docked() const;

    /**
     * @brief Adjust step angle for docking motor.
     * 
     * @param parameter
     */
    void set_step_angle(float angle);

    /**
     * @brief Set number of steps left to turn.
     */
    void set_turn_angle(float angle);

    /**
     * @brief Returns number of steps left to turn.
     */
    unsigned int get_steps() const;

    #ifdef DESKTOP
    void set_dock(bool dock);
    #endif

   private:
    // Sets how many degrees the motor turns in one step.
    // In testing with a load, a step_delay = 4000 took about 22.5 seconds to 
    // turn 180 degrees, so there were 22.5/4000E-6 = 5625 steps taken 
    // -> step_angle = 180/5625 = 0.032 deg/step 
    float step_angle = (0.032*M_PI)/180.0f;
    // Sets the delay between steps (in microseconds), which affects the speed and torque
    TRACKED_CONSTANT_SC(unsigned int, step_delay, 4000);

    // Status of motor sleep pin (and therefore of overall docking motor.)
    #ifndef DESKTOP
    bool is_enabled = false;
    #endif
    
    #ifdef DESKTOP
    //checks whether or not the motor is docked
    bool is_docked = false;
    #endif

    //number of steps left to complete
    static volatile unsigned int steps;
};
}  // namespace Devices

#endif
