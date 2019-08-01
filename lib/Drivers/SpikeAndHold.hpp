#ifndef SPIKE_AND_HOLD_HPP_
#define SPIKE_AND_HOLD_HPP_

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
class SpikeAndHold : public Device {
public:
  //! Number of valves in system.
  static constexpr unsigned char NUM_VALVES = 6;

  //! Specifies which valve # corresponds to which physical valve
  enum VALVE_IDS {
    INTERTANK_MAIN = 0,   // Main tank 1 to tank 2 valve
    INTERTANK_BACKUP = 1, // Backup tank 1 to tank 2 valve
    NOZZLE_1 = 2,         // Nozzle valve
    NOZZLE_2 = 3,         // Nozzle valve
    NOZZLE_3 = 4,         // Nozzle valve
    NOZZLE_4 = 5          // Nozzle valve
  };

  //! Enum as a descriptor for whether a valve is open or closed
  enum VALVE_STATE { CLOSED = 0, OPEN = 1 };

  //! Enum as a descriptor for whether the Spike and Hold system is on or off
  enum SPH_STATE { OFF = 0, ON = 1 };

  //! Default mapping of physical GPIO pin #s (values) to logical pin #s
  //! (indices).
  static std::array<unsigned char, NUM_VALVES> DEFAULT_VALVE_PINS;
  static constexpr unsigned char DEFAULT_ENABLE_PIN = 25;

  /** @brief Default constructor. Loads a set of hardcoded pins into the valve
   * table.**/
  SpikeAndHold(const std::string &name,
               const std::array<unsigned char, NUM_VALVES> &pins,
               unsigned char en);

  bool setup() override;
  bool is_functional() override;
  void disable() override;
  void reset() override;

  /** @brief Turn on Spike and Hold by holding the enable pin high. **/
  void enable();

  /** @brief Execute the schedule specified by the pointer. **/
  void execute_schedule(const std::array<unsigned int, NUM_VALVES> &sch);

  /** @brief Shut all valves. **/
  void shut_all_valves();

private:
  //! # of GPIO pin that valve is connected to.
  std::array<unsigned char, NUM_VALVES> valve_pins;
  //! Pin that must be high to enable Spike and Hold.
  unsigned char enable_pin_;
  //! Whether or not Spike and Hold is on or not.
  bool is_enabled;
};
}

#endif