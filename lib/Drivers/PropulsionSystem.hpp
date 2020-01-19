#ifndef PROPULSION_SYSTEM_HPP_
#define PROPULSION_SYSTEM_HPP_

#include <array>
#include "../Devices/Device.hpp"
#ifndef DESKTOP
#include <Arduino.h>
#include <IntervalTimer.h>
#endif

namespace Devices {

/**
 * PropulsionSystem.hpp
 * Specifies the Driver for the propulsion system
 * PropulsionSystem class defines the interface for the propulsion system
 * It consists of a static Tank1 and Tank2 object.
 * Static methods of PropulsionSystem such as open_valve and close_valve take
 * a reference to either of these objects.
 * 
 * To use the class, update the firing schedule, and then call
 * execute_schedule().
 * Example:
 *
 * SpikeAndHold sph;
 * sph.setup();
 * **/
class TimedLock
{
public:
    inline TimedLock() : end_time(0){}

    inline TimedLock(uint32_t ms_duration) : 
    end_time(millis() + ms_duration) {}

    // No concept of owner
    inline bool procure(uint32_t ms_duration)
    {
        if (millis() < end_time)
        {
            return false;
        }
        else
        {
            end_time = millis() + ms_duration;
            return true;
        }
    }
    // True if the lock is currently free
    inline bool is_free()
    {
        return millis() > end_time;
    }
 private:   
    uint32_t end_time;
};

class Tank {
public:
    Tank(size_t num_pins);

    void setup();

    /**
     * @brief Turns off all valves immediately
     */
    void reset();

    int get_temp();

    // all pins indexed at 0
    size_t num_pins;
    // mandatory wait time between consecutive openings (in ms)
    uint32_t valve_lock_duration;

    uint8_t temp_sensor_pin;
    // mapping of physical GPIO pin #s (values) to logical pin #s
    uint8_t valve_pins[4];
    // true if the valve is opened
    bool is_valve_opened[4];
    // maximum tolerated temperature of tank
    const int max_tank_temp = 48;
    // enforces the mandatory wait time when consescutively opening valves
    TimedLock valve_lock;
};

class Tank1 : public Tank {
public:
    Tank1();
};

class Tank2 : public Tank {
public:
    Tank2();

    void setup();

    void clear_schedule();

    void set_schedule(const std::array<unsigned int, 4> &setting);

    float get_pressure();

    #ifndef DESKTOP
    //! Runs thrust_valve_loop every 3 ms. Initialized in setup().
    static IntervalTimer thrust_valve_loop_timer;
    #endif
    static volatile unsigned int schedule[4];

    static constexpr int threshold_pressure = 25;
    static constexpr int max_tank_pressure = 75;
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
};

class PropulsionSystem : public Device {
    public:
    PropulsionSystem();

    bool setup() override;

    /**
     * @brief Shuts off all valves, clears tank2 schedule
     */
    void reset() override;

    /**
     * @brief turns on interrupts provided by the thurst_value_loop_timer
     */
    void enable();

    /**
     * @brief turns off interrupts (the thrust_value_loop_timer)
     */
    void disable() override;

    bool is_functional() override;

    /**
     * @brief opens the valve specified by valve_idx in the specified tank
     * */
    static bool open_valve(Tank& tank, size_t valve_idx);

    static void close_valve(Tank& tank, size_t valve_idx);

    /**
     * @brief the function that is ran when interrupts provided by IntervalTimer
     * is enabled
     */
    static void thrust_valve_loop();

    static Tank1 tank1;
    static Tank2 tank2;
    static volatile bool is_enabled;

};

}  // namespace Devices

#endif
