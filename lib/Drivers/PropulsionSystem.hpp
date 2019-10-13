#ifndef PROPULSION_SYSTEM_HPP_
#define PROPULSION_SYSTEM_HPP_

#include <array>
#include "../Devices/Device.hpp"
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
    enum valve_ids {
        intertank_main = 0,    // Main tank 1 to tank 2 valve
        intertank_backup = 1,  // Backup tank 1 to tank 2 valve
        nozzle_1 = 2,          // Nozzle valve
        nozzle_2 = 3,          // Nozzle valve
        nozzle_3 = 4,          // Nozzle valve
        nozzle_4 = 5           // Nozzle valve
    };

    //! Default mapping of physical GPIO pin #s (values) to logical pin #s
    //! (indices).
    static const std::array<unsigned char, 6> default_valve_pins;
    static constexpr unsigned char dcdc_sph_enable_pin = 25;
    static constexpr unsigned char pressure_sensor_low_pin = 20;
    static constexpr unsigned char pressure_sensor_high_pin = 23;
    static constexpr unsigned char temp_inner_pin = 21;
    static constexpr unsigned char temp_outer_pin = 22;

    /** @brief Default constructor. **/
    PropulsionSystem();

    bool setup() override;
    bool is_functional() override;
    void disable() override;
    void reset() override;

    /** @brief Turn on Spike and Hold by holding the enable pin high. **/
    void enable();

    float get_pressure();
    signed int get_temp_inner();
    signed int get_temp_outer();

    /** @brief Set the valves, as specified by the array.
     * 
     * Index i corresponds to nozzle i + 1.
     **/
    void set_valves(const std::array<unsigned char, 4> &setting);

    /** @brief Shut all valves. **/
    void shut_all_valves();

   private:
    //! # of GPIO pin that valve is connected to.
    std::array<unsigned char, 6> valve_pins;
    //! Whether or not Spike and Hold is on.
    bool is_enabled;
};
}  // namespace Devices

#endif