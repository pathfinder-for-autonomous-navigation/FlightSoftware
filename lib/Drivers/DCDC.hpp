#ifndef DCDC_HPP_
#define DCDC_HPP_

#include "../Devices/Device.hpp"

namespace Devices {
/**
 * @brief Enables or disables the 7-24V DC-DC converters.
 * **/
// #ifdef DESKTOP
// class DCDC{
// #else
class DCDC : public Device {

   public:
    //! Default enable pin for the DCDCs.
    static constexpr unsigned char dcdc_motor_enable_pin = 24;
    static constexpr unsigned char dcdc_sph_enable_pin = 25;

    /**
     * @brief Default constructor. **/
    DCDC(const std::string &name) : Device(name){}
#ifdef DESKTOP
    bool setup();
    bool is_functional();
    void disable();
    void reset();
#else
    bool setup() override;

    /**
     * @brief Returns true, always.
     */
    bool is_functional() override;

    /**
     * @brief Disable both DCDCs.
     **/
    void disable() override;

    /** 
     * @brief Disables and then re-enables both DCDCs.
     **/
    void reset() override;
#endif
    /**
     * @brief Turn on ADCS DCDC by holding the enable pin high.
     **/
    void enable_adcs();

    /**
     * @brief Turn on the Spike and Hold, pressure sensor, and docking motor
     * DCDC by holding the enable pin high.
     **/
    void enable_sph();

    /** 
     * @brief Turn off ADCS DCDC by holding the enable pin low.
     **/
    void disable_adcs();

    /** 
     * @brief Turn off the Spike and Hold, pressure sensor, and docking motor
     * DCDC by holding the enable pin low.
     **/
    void disable_sph();
};
}  // namespace Devices

#endif