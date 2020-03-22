#ifndef DOCKING_SYSTEM_HPP_
#define DOCKING_SYSTEM_HPP_

#include "../Devices/Device.hpp"
#include <cmath>

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
    static constexpr unsigned char switch_pin = 36;

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
     * @brief Set the direction of the motor turning.
     * 
     * @param clockwise True if we want to turn the motor clockwise.
     */
    void set_direction(bool clockwise);

    /** 
     * @brief Put the docking motor into the "docked" turning configuration.
     **/
    void start_dock();

    /** 
     * @brief Put the docking motor into the "undocked" turning configuration.
     **/
    void start_undock();

    /**
     * @brief Manually step motor by one step.
     */
    void step_motor();

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
    float step_angle = (15.0f*M_PI)/180.0f;

    // Status of motor sleep pin (and therefore of overall docking motor.)
    #ifndef DESKTOP
    bool is_enabled = false;
    #endif
    
    #ifdef DESKTOP
    //checks whether or not the motor is docked
    bool is_docked = false;
    #endif

    //number of steps left to complete
    unsigned int steps = 0;
};
}  // namespace Devices

#endif
