#ifndef DCDC_HPP_
#define DCDC_HPP_

#include "../Devices/Device.hpp"

namespace Devices {
/**
 * @brief Enables or disables the 7-24V DC-DC converters.
 * **/
class DCDC : public Device {
public:
  //! Enum as a descriptor for whether the DCDC system is on or off
  enum DCDC_STATE { OFF = 0, ON = 1 };

  //! Default enable pin for DCDC.
  static constexpr unsigned char DEFAULT_ENABLE_PIN = 24;

  /** @brief Default constructor. Loads a set of hardcoded pins into the valve
   * table.**/
  DCDC(const std::string &name, unsigned char en);

  bool setup() override;
  bool is_functional() override;
  void disable() override;
  void reset() override;

  /** @brief Turn on DCDC by holding the enable pin high. **/
  void enable();

private:
  //! Pin that must be high to enable Spike and Hold.
  unsigned char enable_pin_;
  //! Whether or not Spike and Hold is on or not.
  bool is_enabled;
};
}

#endif