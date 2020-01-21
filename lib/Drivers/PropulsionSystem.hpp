#ifndef PROPULSION_SYSTEM_HPP_
#define PROPULSION_SYSTEM_HPP_
/**
 * PropulsionSystem.hpp
 * Specifies the Driver for the propulsion system
 */
#include <array>
#include "../Devices/Device.hpp"
#ifndef DESKTOP
#include <Arduino.h>
#include <IntervalTimer.h>
#endif

namespace Devices {

class TimedLock;
class Tank;
class Tank1;
class Tank2;
/**
 * PropulsionSystem class defines the interface for the propulsion system.
 * 
 * It consists of a static Tank1 and Tank2 data objects. 
 * Only methods from PropulsionSystem may change the states of either Tank1 or Tank2. 
 * 
 * Tank1 valves are at index 0, 1
 * Tank2 valves are at index 0, 1, 2, 3
 * 
 * Dependencies: 
 * SpikeAndHold must be enabled in order to use this system
 * 
 * Usage:
 * - Use set_schedule to set a firing schedule for tank2. 
 * - Use enable to turn on the schedule when we are close to the start_time.
 * - Use disable to prematurely cancel the schedule.
 * - Use clear_schedule to reset the schedule and start_time to 0.
 * - Use is_tank2_ready to check if tank2 is scheduled to fire right now or in the future
 * - Use is_firing to check if we are still executing the schedule
 * - Use disable to stop the timer when is_firing() returns false
 * 
 * Notes:
 * Only tank2 has a schedule since only tank2 uses the IntervalTimer to fire
 * 
 **/
class PropulsionSystem : public Device {
public:
    PropulsionSystem();

    /**
     * @brief Enables INPUT/OUTPUT on the valve pins and sensor pins of tank1 and tank2
     */
    bool setup() override;

    /**
     * @brief Shuts off all valves immediately, 
     * clears tank2 schedule, disables tank2 IntervalTimer
     */
    void reset() override;

    /**
     * @brief Return true if start_time_us is at least 3001 us into the future
     */
    static bool is_start_time_ok(uint32_t start_time_us);

    /**
     * @brief Turns on interrupts provided by the thurst_value_loop_timer
     * @return True if we tank2 is now set to fire at its current schedule.
     * 
     * If this is called when the system is already enabled, it will disable()
     * and return false. 
     * 
     * tank2.start_time must be set to some time in the future
     */
    bool enable();

    /**
     * @brief Turns off interrupts (the thrust_value_loop_timer)
     * 
     * If this is called before tank2.start_time, it will also reset tank2 lock
     */
    void disable() override;

    /**
     * @brief True if Spike and Hold is enabled
     */
    bool is_functional() override;

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
     * @brief Return true if we are currently firing tank2 
     * (at least one valve pin is HIGH AND interval timer is on)
     */
    bool is_firing();

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

// protected:
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
    uint32_t valve_lock_duration;
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
     * @brief Returns the current value of the schedule for the specified valve_num
     */
    unsigned int get_schedule_at(size_t valve_num);

private:
    #ifndef DESKTOP
    //! When enabled, runs thrust_valve_loop every 3 ms
    static IntervalTimer thrust_valve_loop_timer;
    #endif

    static uint32_t start_time;
    static volatile unsigned int schedule[4];
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

}  // namespace Devices

#endif
