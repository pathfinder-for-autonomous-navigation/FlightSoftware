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
 *  - enforces mandatory wait time between opening valves on both tanks
 * 
 * Dependencies: 
 *  - SpikeAndHold must be enabled in order to use this system
 */
#include <array>
#ifndef DESKTOP
#include <fsw/FCCode/Devices/Device.hpp>
#include <Arduino.h>
#include <IntervalTimer.h>
#endif


#ifdef DESKTOP
uint32_t micros(void);
#else
namespace Devices {
#endif

class TimedLock;
class Tank;
class Tank1;
class Tank2;
/**
 * PropulsionSystem class defines the interface for the propulsion system, which 
 * consists of an inner tank, Tank1, and the (outer) thrust tank, Tank2. 
 * This driver provides functionality for opening and closing valves on both tanks,
 * setting and enforcing the thrust schedule 
 * 
 * It consists of a static Tank1 (inner tank) and Tank2 (thrust tank) data objects. 
 * 
 * Specifications:
 * Tank1 valves are at numbered 0, 1
 * Tank2 valves are at numbered 0, 1, 2, 3
 * mandatory_wait_time is the duration of time after opening a valve before 
 * we can open another valve. Tank1 10*1000 ms. Tank2 is 3ms
 * 
 * State Definitions:
 * "enabled" means that the IntervalTimer for tank2 is on. This implies that:
 *      - either tank2 is "scheduled (to fire)" 
 *      - or tank2 has already fired
 * "scheduled (to fire)" means that tank2 currently has a scheduled start time
 * in the future
 * 
 * "start time" refers to the time in micros at which tank2 will fire
 * Prop controller is responsible for pressurizing tank1 before asking tank2 to fire
 * 
 * Dependencies: 
 * SpikeAndHold must be enabled in order to use this system
 * 
 * Usage:
 * - Call setup to setup this device
 * - Use set_schedule to set a firing schedule for tank2. 
 * - Use enable to turn on the schedule when we are close to the start_time.
 * - Use disable to prematurely cancel the schedule.
 * - Use clear_schedule to reset the schedule and start_time to 0.
 * - Use is_tank2_ready to check if tank2 is scheduled to fire right now or in the future
 * - Use is_firing to check if we are still executing the schedule
 * - Use disable to stop the timer when is_firing() returns false
 * 
 * Implementation Notes and Warnings:
 * The only public methods that can change the states in tank1 or tank2 are the
 * methods in PropulsionSystem.
 * 
 * TimedLock enforces
 *  - tank2 scheduled start time and the 
 *  - tank1 10s mandatory wait time
 * IntervalTimer (thrust_valve_loop_timer) enforces
 *  - tank2 3ms mandatory wait time
 *  - tank2 firing schedule
 * 
 * Only tank2 has a schedule since only tank2 uses the IntervalTimer to fire.
 * 
 * No mandatory wait time is enforced when opening valves from different tanks. For example,
 * suppose we open a valve on tank1. Although we cannot open the other valve for another 10s,
 * we can immediately open any valve on tank2.
 * 
 **/
#ifdef DESKTOP
    uint32_t micros(){ return 0; }
#endif
#ifndef DESKTOP
class PropulsionSystem : public Device {
#else
class PropulsionSystem{
#endif
public:
    PropulsionSystem();

    /**
     * @brief Enables INPUT/OUTPUT on the valve pins and sensor pins of tank1 and tank2
     * @return True if successfully setup both tank1 and tank2 and all pins
     */
#ifndef DESKTOP
    bool setup() override;
#else
    bool setup();
#endif
    /**
     * @brief Resets all runtime (transient) values to their default values
     * 
     * EFFECTS:
     *  - Shuts off all valves in both tank1 and tank2 immediately 
     *  - Clears tank2 schedule 
     *  - Disables tank2 IntervalTimer
     *  - Unlocks both tank1 and tank2 locks
     */
#ifndef DESKTOP
    void reset() override;
#else
    void reset();
#endif

    /**
     * @brief Turns on IntervalTimer thurst_value_loop_timer, causes an interrupt
     * every 3 ms
     * @return True if tank2 is now set to fire at time tank2.start_time.
     * 
     * EFFECTS:
     *  - If timer has already been enabled, calls disable() and return false. 
     * REQUIRE: 
     *  - tank2.start_time must be set to set at least 2.9ms into the future
     */
    bool enable();

    /**
     * @brief Return true if start_time_us is at least 3001 us into the future
     */
    static bool is_start_time_ok(uint32_t start_time_us);

    /**
     * @brief Turns off IntervalTimer (disables thrust_valve_loop_timer interrupts)
     * 
     * EFFECTS:
     *  - Ends tank2 IntervalTimer
     *  - Closes all tank2 valves
     *  - If tank2.start_time is in the future, reset tank2 lock
     */
#ifndef DESKTOP
    void disable() override;
#else
    void disable();
#endif

    /**
     * @brief True if Spike and Hold is enabled
     */
#ifndef DESKTOP
    bool is_functional() override;
#else
    bool is_functional();
#endif

    /**
     * @brief Sets the firing schedule for tank 2. Does not enable tank2 to
     * fire. To do that, call enable()
     * 
     * PreCondition:
     *  - start_time_us must not be more than 71 minutes into the future.
     * Requires:
     *  - IntervalTimer is currently disabled
     *  - start_time_us is some time in the future
     *  - All valve1, ..., valve4 values less than 1000 (1 s)
     * @return True if the requested schedule was successfully set
     */
    static bool set_schedule(
        uint32_t valve1, 
        uint32_t valve2, 
        uint32_t valve3, 
        uint32_t valve4, 
        uint32_t start_time_us);

    /**
     * @brief clears the Tank2 schedule only if IntervalTimer is off
     */
    static bool clear_schedule();

    /**
     * @brief True if tank2 is firing right now or is scheduled to fire in the
     * future
     */
    inline static bool is_tank2_ready()
    {
        return is_enabled;
    }

    /**
     * @brief This tells us if we are done firing since the thrust loop will set the schedule
     * to 0
     * @return True if schedule is zero AND timer is enabled.
     */
    bool is_done_firing();

    /**
     * @brief opens the valve specified by valve_idx in the specified tank
     * */
    static bool open_valve(Tank& tank, size_t valve_idx);

    /**
     * @brief closes the valve specified by valve_idx in the specified tank
     */
    static void close_valve(Tank& tank, size_t valve_idx);

    static Tank1 tank1;
    static Tank2 tank2;

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
    static volatile bool is_enabled;

};

/**
 * 
 * TimedLock is an "at least" lock with units in microseconds. 
 * There is no concept of owner. Once procured, TimedLock remains locked until
 * the end of the duration specified by the most recent procure. 
 * 
 */ 
class TimedLock
{
public:
    inline TimedLock() : end_time(0){}

    /**
     * Attempt to take the lock for us_duration
     */
    inline bool procure(uint32_t us_duration)
    {
        if (micros() < end_time)
        {
            return false;
        }
        else
        {
            end_time = micros() + us_duration;
            // Serial.printf("Current time is %u, Requested us_duration %u, Timed lock set to %u\n",micros(), us_duration, end_time);
            return true;
        }
    }
    /**
     * Check the status of the lock
     */
    inline bool is_free()
    {
        return micros() > end_time;
    }

    inline uint32_t get_end_time()
    {
        return end_time;
    }

    /**
     * Checks for unsigned int overflow and returns the sum of a and b if 
     * their sum does not overflow. Returns 0 in the case of an overflow
     */
    inline static uint32_t safe_add(uint32_t a, uint32_t b)
    {
        // a + b > UINT32_MAX --> b > UINT32_MAX - a
        return (b > UINT32_MAX - a) ? 0 : a + b;
    }

    /**
     * Checks for unsigned int underflow and returns a - b if their difference
     * does not underflow. Returns 0 in the case of an underflow
     */
    inline static uint32_t safe_subtract(uint32_t a, uint32_t b)
    {
        // is a - b < 0 -->  a < b
        return (a < b) ? 0 : a - b;
    }
private:   
    uint32_t end_time;
    friend class PropulsionSystem;
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
    int get_temp();

    /**
     * @brief Returns true if the valve is open
     * 
     * (Digital) reads the pin for the specified valve and returns true
     * if that pin is HIGH
     */
    bool is_valve_open(size_t valve_idx);

    /**
     * @brief Returns true if the TimedLock for this tank is unlocked (free)
     */
    inline bool is_lock_free()
    {
        return valve_lock.is_free();
    }

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
    // mandatory wait time between consecutive openings (in ms)
    uint32_t mandatory_wait_time;
    // pin number of the temperature sensor
    uint8_t temp_sensor_pin;
    // mapping of physical GPIO pin #s (values) to logical pin #s
    uint8_t valve_pins[4];
    // true if the valve is opened
    bool is_valve_opened[4];
    // enforces the mandatory wait time when consescutively opening valves
    TimedLock valve_lock;

    friend class PropulsionSystem;
};

/**
 * Tank1 represents the inner tank in the Propulsion System
 * valve 0 - main intertank valve
 * Valve 1 - backup intertank valve
 */
class Tank1 : public Tank {
public:
    Tank1();
};

/**
 * Tank2 reprsents the outer tank in the Propulsion System
 * Valve 0, 1, 2, 3 - four thrust valves
 */
class Tank2 : public Tank {
public:
    Tank2();

    void setup();

    float get_pressure();

    /**
     * @brief Returns the current value of the schedule for the specified valve
     */
    unsigned int get_schedule_at(size_t valve_num);

private:
    #ifndef DESKTOP
    //! When enabled, runs thrust_valve_loop every 3 ms
    static IntervalTimer thrust_valve_loop_timer;
    #endif

    static uint32_t start_time;
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

    //! Loop interval in microseconds.
    static constexpr unsigned int thrust_valve_loop_interval_us = 3000;
    //! Loop interval in milliseconds.
    static constexpr unsigned int thrust_valve_loop_interval_ms =
        thrust_valve_loop_interval_us / 1000;

    friend class PropulsionSystem;
};
#ifndef DESKTOP
}  // namespace Devices
#endif
#endif
