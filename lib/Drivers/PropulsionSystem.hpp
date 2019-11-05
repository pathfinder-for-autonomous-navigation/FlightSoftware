#ifndef PROPULSION_SYSTEM_HPP_
#define PROPULSION_SYSTEM_HPP_

#include <array>
#include "../Devices/Device.hpp"
#include <IntervalTimer.h>

namespace Devices {
/**
 * To use the class, update the firing schedule, and then call
 * execute_schedule().
 * Example:
 *
 * SpikeAndHold sph;
 * sph.setup();
 * **/
class PropulsionSystem : public Device {
   public:
    //! Specifies which valve # corresponds to which physical valve
    enum class valve_ids {
        intertank_main = 0,    // Main tank 1 to tank 2 valve
        intertank_backup = 1,  // Backup tank 1 to tank 2 valve
        nozzle_1 = 2,          // Nozzle valve
        nozzle_2 = 3,          // Nozzle valve
        nozzle_3 = 4,          // Nozzle valve
        nozzle_4 = 5           // Nozzle valve
    };

    //! Default mapping of physical GPIO pin #s (values) to logical pin #s
    //! (indices).
    static const std::array<unsigned char, 6> valve_pins;
    static constexpr unsigned char pressure_sensor_low_pin = 20;
    static constexpr unsigned char pressure_sensor_high_pin = 23;
    static constexpr unsigned char temp_sensor_inner_pin = 21;
    static constexpr unsigned char temp_sensor_outer_pin = 22;

    /** @brief Default constructor. **/
    PropulsionSystem();

    bool setup() override;
    bool is_functional() override;

    /**
     * @brief Shuts off all valves and turns off the thrust valve loop timer.
     */
    void disable() override;

    /**
     * @brief Turns on the thrust valve loop timer.
     */
    void enable();

    void reset() override;

    float get_pressure();
    signed int get_temp_inner();
    signed int get_temp_outer();

    /** @brief Set the thrust valve schedule, as specified by the array.
     * 
     * Index i corresponds to nozzle i + 1.
     **/
    void set_thrust_valve_schedule(const std::array<unsigned int, 4> &setting);

    /**
     * @brief Set the tank valve state for the given tank valve.
     * 
     * @param valve 1 if the main tank valve should be set, 0 if the backup
     *              should be set.
     * @param state 1 if the valve should be open, 0 if the valve should be
     *              closed.
     */
    void set_tank_valve_state(bool valve, bool state);

   private:
    // A bunch of these variables are static so that the interval timer can call
    // a static function. Practically, this is OK because we only expect to
    // create the propulsion system object once.

    //! If true, a thrust valve was opened on the current cycle
    // of the thrust valve loop, so another valve (thrust or prop)
    // should not be opened during the current cycle.
    static volatile bool valve_start_locked_out;

    //! Runs thrust_valve_loop every 3 ms. Initialized in setup().
    IntervalTimer thrust_valve_loop_timer;
    //! Loop interval in microseconds.
    static constexpr unsigned int thrust_valve_loop_interval_us = 3000;
    //! Loop interval in milliseconds.
    static constexpr unsigned int thrust_valve_loop_interval_ms =
        thrust_valve_loop_interval_us / 1000;

    //! Tracks if thrust valves are open.
    static volatile bool is_valve_opened[4];

    //! Thrust valve schedule, specified by flight software. Times are in
    // milliseconds.
    static volatile unsigned int thrust_valve_schedule[4];
    static void thrust_valve_loop();

    // Pressure sensor offsets and slopes from PAN-TPS-002 test data
    // (https://cornellprod-my.sharepoint.com/personal/saa243_cornell_edu/_layouts/15/onedrive.aspx?id=%2Fpersonal%2Fsaa243_cornell_edu%2FDocuments%2FOAAN%20Team%20Folder%2FSubsystems%2FSoftware%2Fpressure_sensor_data%2Em&parent=%2Fpersonal%2Fsaa243_cornell_edu%2FDocuments%2FOAAN%20Team%20Folder%2FSubsystems%2FSoftware)
    static constexpr double high_gain_offset = -0.119001938553720;
    static constexpr double high_gain_slope = 0.048713211537332;
    static constexpr double low_gain_offset = 0.154615074342874;
    static constexpr double low_gain_slope = 0.099017990785657;
};
}  // namespace Devices

#endif
