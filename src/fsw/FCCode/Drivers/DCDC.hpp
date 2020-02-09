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
    static constexpr unsigned char ADCSMotorDCDC_EN = 24;
    static constexpr unsigned char SpikeDockDCDC_EN = 25;

    /**
     * @brief Default constructor. **/
    DCDC(const std::string &name) : Device(name){}
#ifdef DESKTOP
    bool setup();
    bool is_functional();
    void disable();
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
#endif
    /**
     * @brief Turn on ADCS DCDC by holding the enable pin high.
     **/
    void enable_adcs();

    /**
     * @brief Returns if ADCS is enabled.
     **/
    const bool adcs_enabled();

    /**
     * @brief Turn on the Spike and Hold, pressure sensor, and docking motor
     * DCDC by holding the enable pin high.
     **/
    void enable_sph();

    /**
     * @brief Returns if spike and hold is enabled.
     **/
    const bool sph_enabled();

    /** 
     * @brief Turn off ADCS DCDC by holding the enable pin low.
     **/
    void disable_adcs();

    /** 
     * @brief Turn off the Spike and Hold, pressure sensor, and docking motor
     * DCDC by holding the enable pin low.
     **/
    void disable_sph();

   protected:
    #ifdef DESKTOP
    // Returns whether or not adcs is enabled. Default is false
    bool adcs = false;

    // Returns whether or not spike and hold is enabled. Default is false
    bool sph = false;
    #endif

};
}  // namespace Devices

#endif