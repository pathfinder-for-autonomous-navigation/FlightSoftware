#ifndef PROPULSION_SYSTEM_HPP_
#define PROPULSION_SYSTEM_HPP_
/**
 * PropulsionSystem.hpp
 * Specifies the Driver for the propulsion system, which consists of an inner tank
 * and an (outer) thrust tank.
 * 
 * Features:
 *  - opens and closes valves on both tanks
 *  - reads temperature sensors on both tanks
 *  - reads pressure sensor for Tank2
 *  - enforces firing schedule for Tank2
 * 
 * Dependencies: 
 *  - SpikeAndHold must be enabled in order to use this system
 */
#include <array>
#include <fsw/FCCode/Devices/Device.hpp>
#ifndef DESKTOP
#include <Arduino.h>
#endif
namespace Devices {
class Tank;
/**
 * PropulsionSystem class defines the interface for the propulsion system, which 
 * consists of an inner tank, Tank1, and the (outer) thrust tank, Tank2. 
 * This driver provides functionality for opening and closing valves on both tanks,
 * setting and enforcing the thrust schedule on Tank2
 * 
 * It consists of a static Tank1 (inner tank) and Tank2 (thrust tank) data objects. 
 * These data objects are "models" and represents the state of the tanks (i.e. which 
 * valves are open, the current tank pressure, the current temperature). 
 * Models do not change their own states. 
 * The "controller" is PropulsionSystem, which operates on the Tank models
 * in order to change their states. 
 *  
 * A schedule is a ordered list of four integers {0, 0, 0, 0} corresponding to the
 * four valves on Tank2. A schedule is "zero" if and only if all four integers are 0. 
 * It represents the duration for which we would like to open each valve. 
 * 
 * We say that the PropulsionSystem is "scheduled to fire" when Tank2 has a non-zero
 * schedule and interrupts are enabled. Tank2 is
 * "ready to fire" when it is "scheduled to fire" or is currently firing. 
 *  
 * This driver does not automatically disable interrupts once Tank2 is done firing. 
 * Therefore, we say that Tank2 is "done firing" when it has finished firing. 
 * This is indicated when its schedule is set to 0 (although interrupts may still be on).
 * 
 * Specifications:
 * Tank1 valves are at numbered 0, 1
 * Tank2 valves are at numbered 0, 1, 2, 3
 * we can open another valve. Tank1 10*1000 ms. Tank2 is 3ms
 * 
 * "enabled" means that the IntervalTimer (interrupts) for tank2 is on. This implies that:
 *      - either tank2 is "scheduled (to fire)" 
 *      - or tank2 has already fired
 * 
 * Dependencies: 
 * SpikeAndHold must be enabled in order to use this system
 * 
 * Usage:
 * - Call setup to setup this device
 * - Use set_schedule to set a firing schedule for Tank2. 
 * - Use start_firing() to start firing Tank2
 * - Use clear_schedule() to reset the schedule
 * - Use is_firing() to check if Tank2 is currently firing
 * 
 * Implementation Notes and Warnings:
 * The only public methods that can change the states in tank1 or tank2 are the
 * methods in PropulsionSystem.
 * Prop controller is responsible for pressurizing tank1 before asking tank2 to fire
 * 
 * IntervalTimer (thrust_valve_loop_timer) enforces
 *  - tank2 3ms mandatory wait time
 *  - tank2 firing schedule
 * 
 * Only tank2 has a schedule since only tank2 uses the IntervalTimer to fire.
 * 
 **/
#define PropulsionSystem Devices::_PropulsionSystem::Instance()
class _PropulsionSystem : public Device {
    _PropulsionSystem();
public:
    inline static _PropulsionSystem& Instance()
    {
        static _PropulsionSystem Instance;
        return Instance;
    }

    /**
     * @brief Enables INPUT/OUTPUT on the valve pins and sensor pins of tank1 and tank2
     * @return True if successfully setup both tank1 and tank2 and all pins
     */
    bool setup() override;
    /**
     * @brief Resets all runtime (transient) values to their default values
     * 
     * EFFECTS:
     *  - Shuts off all valves in both tank1 and tank2 immediately 
     *  - Clears tank2 schedule 
     *  - Disables tank2 IntervalTimer
     */
    void reset() override;

    /**
     * @brief Turns on IntervalTimer thrust_value_loop_timer, causes an interrupt
     * every 3 ms
     * @return True if tank2 will start firing right now.
     * 
     */
    bool start_firing();

    /**
     * @brief Turns off IntervalTimer (disables thrust_valve_loop_timer interrupts)
     * 
     * EFFECTS:
     *  - Ends tank2 IntervalTimer
     *  - Closes all tank2 valves
     */
    void disable() override;

    /**
     * @brief 
     */
    bool is_functional() override;

    /**
     * @brief Sets the firing schedule for tank 2. Does not enable tank2 to
     * fire. To do that, call start_firing
     * 
     * Requires:
     *  - IntervalTimer is currently disabled
     *  - All valve1, ..., valve4 values less than 1000 (1 s)
     * @return True if the requested schedule was successfully set
     */
    static bool set_schedule(
        uint32_t valve1, 
        uint32_t valve2, 
        uint32_t valve3, 
        uint32_t valve4);

    /**
     * @brief clears the Tank2 schedule only if IntervalTimer is off
     */
    static bool clear_schedule();

    /**
     * @brief opens the valve specified by valve_idx in the specified tank
     * */
    static bool open_valve( Tank& tank, size_t valve_idx);

    /**
     * @brief closes the valve specified by valve_idx in the specified tank
     */
    static void close_valve( Tank& tank, size_t valve_idx);

    inline static bool is_firing()
    {
        return is_interval_enabled;
    }

private:
    
    /**
     * @brief the function that is ran at each interrupt when the IntervalTimer
     * is enabled
     * Valves may be closed at most 3 ms early. 
     * When all valves have fired, all entries of tank2 schedule will be 0
     */
    static void thrust_valve_loop();

    /**
     * @brief true if tank2's IntervalTimer is on (tank2 is scheduled to fire)
     */
    static bool is_interval_enabled;

};

/**
 * Tank represents a tank in the PropulsionSystem. Public methods of this class
 * may not change its own internal state (side-effects)
 * 
 */
class Tank {
public:
    Tank(size_t num_pins);

    /**
     * @brief (Analog) reads the temperature sensor for this tank and 
     * returns its value.
     */
    int get_temp() const;

    /**
     * @brief Returns true if the valve is open
     * 
     * (Digital) reads the pin for the specified valve and returns true
     * if that pin is HIGH
     */
    bool is_valve_open(size_t valve_idx) const;

protected:
    /**
     * @brief Enables valve INPUT/OUTPUT on valve and sensor pins
     */
    void setup();

    /**
     * @brief Closes all valves immediately
     */
    void close_all_valves();

    size_t num_valves;
    // pin number of the temperature sensor
    uint8_t temp_sensor_pin;
    // mapping of physical GPIO pin #s (values) to logical pin #s
    uint8_t valve_pins[4];
    // true if the valve is opened
    bool is_valve_opened[4];

    friend class _PropulsionSystem;
};

#define Tank1 Devices::_Tank1::Instance()
/**
 * Tank1 represents the inner tank in the Propulsion System
 * valve 0 - main intertank valve
 * Valve 1 - backup intertank valve
 */
class _Tank1 : public Tank {
    _Tank1();
public:
    inline static _Tank1& Instance()
    {
        static _Tank1 Instance;
        return  Instance;
    }
};

#define Tank2 Devices::_Tank2::Instance()
/**
 * Tank2 reprsents the outer tank in the Propulsion System
 * Valve 0, 1, 2, 3 - four thrust valves
 */
class _Tank2 : public Tank {
    _Tank2();
public:

    inline static _Tank2& Instance()
    {
        static _Tank2 Instance;
        return Instance;
    }

    float get_pressure() const;

    /**
     * @brief Returns the current value of the schedule for the specified valve
     */
    unsigned int get_schedule_at(size_t valve_num) const;

private:
    void setup();
    #ifndef DESKTOP
    //! When enabled, runs thrust_valve_loop every 3 ms
    static IntervalTimer thrust_valve_loop_timer;
    #endif

    static volatile unsigned int schedule[4];
    // The minimum duration to assign to a schedule
    // Any value below this value will be ignored by tank2
    static constexpr unsigned int min_firing_duration_ms = 10;
    
    static constexpr unsigned char pressure_sensor_low_pin = 20;
    static constexpr unsigned char pressure_sensor_high_pin = 23;

    // Pressure sensor offsets and slopes from PAN-TPS-002 test data
    // (https://cornellprod-my.sharepoint.com/personal/saa243_cornell_edu/_layouts/15/onedrive.aspx?id=%2Fpersonal%2Fsaa243_cornell_edu%2FDocuments%2FOAAN%20Team%20Folder%2FSubsystems%2FSoftware%2Fpressure_sensor_data%2Em&parent=%2Fpersonal%2Fsaa243_cornell_edu%2FDocuments%2FOAAN%20Team%20Folder%2FSubsystems%2FSoftware)
    static constexpr double high_gain_offset = -0.119001938553720;
    static constexpr double high_gain_slope = 0.048713211537332;
    static constexpr double low_gain_offset = 0.154615074342874;
    static constexpr double low_gain_slope = 0.099017990785657;

    //! Loop interval in milliseconds.
    static constexpr unsigned int thrust_valve_loop_interval_ms = 3; 

    friend class _PropulsionSystem;
};
}  // namespace Devices
#endif
