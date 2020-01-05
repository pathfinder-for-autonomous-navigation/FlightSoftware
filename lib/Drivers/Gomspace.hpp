#include <string>
#include "../Devices/I2CDevice.hpp"

#ifndef GOMSPACE_HPP_
#define GOMSPACE_HPP_

namespace Devices {
/**
 * @brief Device class for interacting with the Gomspace battery management
 * system.
 */
class Gomspace : public I2CDevice {
   public:
    #ifndef DESKTOP
    static constexpr i2c_t3& wire = Wire; /**< I2C bus of Gomspace device **/
    #endif
    static constexpr unsigned char address = 0x02; /**< I2C address of Gomspace device **/
    

    // TODO
    //! Gomspace voltage level designations
    enum VOLTAGES {
        GS_CRITICAL = 6000,
        GC_NORMAL = 6000,
        FC_SAFE = 6000,
        FC_NORMAL = 6000,
    };

    enum DEVICE_PINS {
        PIKSI = 7,
        SPIKE_AND_HOLD = 6,
        QUAKE = 5,
        ADCS = 4,
    };

    /**< "Housekeeping" data struct; contains Gomspace state information. */
    struct __attribute__((packed)) eps_hk_t {
        unsigned short int vboost[3];            //! Voltage of boost converters [mV] [PV1,
                                                 //! PV2, PV3] //! Voltage of battery [mV]
        unsigned short int vbatt;                //! Voltage of battery [mV]
        unsigned short int curin[3];             //! Current in [mA]
        unsigned short int cursun;               //! Current from boost converters [mA]
        unsigned short int cursys;               //! Current out of battery [mA]
        unsigned short int reserved1;            //! Reserved for future use
        unsigned short int curout[6];            //! Current out (switchable outputs) [mA]
        unsigned char output[8];                 //! Status of outputs**
        unsigned short int output_on_delta[8];   //! Time till power on** [s]
        unsigned short int output_off_delta[8];  //! Time till power off** [s]
        unsigned short int latchup[6];           //! Number of latch-ups
        unsigned int wdt_i2c_time_left;          //! Time left on I2C wdt [s]
        unsigned int wdt_gnd_time_left;          //! Time left on I2C wdt [s]
        unsigned char wdt_csp_pings_left[2];     //! Pings left on CSP wdt
        unsigned int counter_wdt_i2c;            //! Number of WDT I2C reboots
        unsigned int counter_wdt_gnd;            //! Number of WDT GND reboots
        unsigned int counter_wdt_csp[2];         //! Number of WDT CSP reboots
        unsigned int counter_boot;               //! Number of EPS reboots
        short int temp[6];        //! Temperatures [degC] [0 = TEMP1, TEMP2, TEMP3, TEMP4,
                                  //! BP4a, BP4b]
        unsigned char bootcause;  //! Cause of last EPS reset
        unsigned char battmode;   //! Mode for battery [0 = initial, 1 = undervoltage,
                                  //! 2 = safemode, 3 = nominal, 4=full]
        unsigned char pptmode;    //! Mode of PPT tracker [1=MPPT, 2=FIXED]
        unsigned short int reserved2;
    };

    struct __attribute__((packed)) eps_hk_vi_t {
        unsigned short int vboost[3];  //! Voltage of boost converters [mV] [PV1, PV2, PV3]
        unsigned short int vbatt;      //! Voltage of battery [mV]
        unsigned short int curin[3];   //! Current in [mA]
        unsigned short int cursun;     //! Current from boost converters [mA]
        unsigned short int cursys;     //! Current out of battery [mA]
        unsigned short int reserved1;  //! Reserved for future use
    };

    struct __attribute__((packed)) eps_hk_out_t {
        unsigned short int curout[6];            //! Current out (switchable outputs) [mA]
        unsigned char output[8];                 //! Status of outputs**
        unsigned short int output_on_delta[8];   //! Time till power on** [s]
        unsigned short int output_off_delta[8];  //! Time till power off** [s]
        unsigned short int latchup[6];           //! Number of latch-ups
    };

    struct __attribute__((packed)) eps_hk_wdt_t {
        unsigned int wdt_i2c_time_left;       //! Time left on I2C wdt [s]
        unsigned int wdt_gnd_time_left;       //! Time left on I2C wdt [s]
        unsigned char wdt_csp_pings_left[2];  //! Pings left on CSP wdt
        unsigned int counter_wdt_i2c;         //! Number of WDT I2C reboots
        unsigned int counter_wdt_gnd;         //! Number of WDT GND reboots
        unsigned int counter_wdt_csp[2];      //! Number of WDT CSP reboots
    };

    struct __attribute__((packed)) eps_hk_basic_t {
        unsigned int counter_boot;  //! Number of EPS reboots
        short int temp[6];          //! Temperatures [degC] [0 = TEMP1, TEMP2, TEMP3, TEMP4,
                                    //! BATT0, BATT1]
        unsigned char bootcause;    //! Cause of last EPS reset
        unsigned char battmode;     //! Mode for battery [0 = initial, 1 = undervoltage,
                                    //! 2 = safemode, 3 = nominal, 4=full]
        unsigned char pptmode;      //! Mode of PPT tracker [1=MPPT, 2=FIXED]
        unsigned short int reserved2;
    };

    /**< Config data struct; contains output/heater configurations and PPT
     * configuration. */
    struct __attribute__((packed)) eps_config_t {
        unsigned char ppt_mode;                //! Mode for PPT [1 = AUTO, 2 = FIXED]
        unsigned char battheater_mode;         //! Mode for battheater [0 = Manual, 1 = Auto]
        signed char battheater_low;            //! Turn heater on at [degC]
        signed char battheater_high;           //! Turn heater off at [degC]
        unsigned char output_normal_value[8];  //! Nominal mode output value
        unsigned char output_safe_value[8];    //! Safe mode output value
        unsigned short int output_initial_on_delay[8];   //! Output switches: init
                                                         //! with these on delays [s]
        unsigned short int output_initial_off_delay[8];  //! Output switches: init
                                                         //! with these off delays
                                                         //! [s]
        unsigned short int vboost[3];  //! Fixed PPT point for boost converters [mV]
    };

    /**< Config2 data struct; contains battery voltage level definitionss. */
    struct __attribute__((packed)) eps_config2_t {
        unsigned short int batt_maxvoltage;
        unsigned short int batt_safevoltage;
        unsigned short int batt_criticalvoltage;
        unsigned short int batt_normalvoltage;
        unsigned int reserved1[2];
        unsigned char reserved2[4];
    };

    /** @brief Constructs Gomspace interface on the specified wire and with the
     * given address. */
    Gomspace(eps_hk_t *hk_data, eps_config_t *config_data, eps_config2_t *config2_data);

    // Device functions
    bool setup() override;
    void reset() override;
    bool i2c_ping() override;

    /** @brief Get full housekeeping data struct.
     *  @return True if housekeeping data struct was able to be found, false
     * otherwise. */
    bool get_hk();
    /** @brief Get vi housekeeping data struct.
     *  @return True if housekeeping data struct was able to be found, false
     * otherwise. */
    bool get_hk_vi();
    /** @brief Get out housekeeping data struct.
     *  @return True if housekeeping data struct was able to be found, false
     * otherwise. */
    bool get_hk_out();
    /** @brief Get wdt housekeeping data struct.
     *  @return True if housekeeping data struct was able to be found, false
     * otherwise. */
    bool get_hk_wdt();
    /** @brief Get basic housekeeping data struct.
     *  @return True if housekeeping data struct was able to be found, false
     * otherwise. */
    bool get_hk_basic();

    /** @brief Set output channels on or off.
     *  @param output_byte Output byte that masks channels. */
    virtual bool set_output(unsigned char output_byte);
    /** @brief Set a single output on or off, with an optional time delay.
     *  @param channel Channel to set on or off. (See NanoPower documentation to
     * see how channel numbers correspond to outputs.)
     *  @param value Whether to set the channel on or off.
     *  @param time_delay Time delay for change, in seconds. */
    virtual bool set_single_output(unsigned char channel, unsigned char value,
                                   short int time_delay = 0);

    /** @brief Set voltage of photovoltaic inputs.
     * @param voltage1 Voltage of input 1, in mV.
     * @param voltage2 Voltage of input 2, in mV.
     * @param voltage3 Voltage of input 3, in mV. */
    bool set_pv_volt(unsigned short int voltage1, unsigned short int voltage2,
                     unsigned short int voltage3);
    /** @brief Set power point mode (PPT).
     *  @param mode Which mode to use. See NanoPower documentation for available
     * modes. */
    bool set_pv_auto(unsigned char mode);

    /** @brief Turn on the onboard heater. */
    bool turn_on_heater();
    /** @brief Turn off the onboard heater. */
    bool turn_off_heater();
    /** @brief Get heater status.
     *  @return 0 = onboard heater off, 1 = onboard heater is on, 2 = error
     * reading heater. */
    unsigned char get_heater();

    /** @brief Reset boot and WDT counters. */
    bool reset_counters();
    /** @brief Reset I2C watchdog timer. */
    bool reset_wdt();

    /** @brief Get EPS configuration as a struct.
     *  @return Address of EPS configuration struct. */
    bool config_get();
    /** @brief Set EPS configuration.
     *  @param config EPS configuration to set. */
    bool config_set(const eps_config_t &config);

    /** @brief Hard reset the Gomspace (and power-cycle all outputs). */
    bool hard_reset();

    /** @brief Get config2 as a struct. */
    bool config2_get();
    /** @brief Set config2.
     *  @param config Struct of config2 data to set. */
    bool config2_set(const eps_config2_t &config);
    /** @brief Restore default config2. */
    bool restore_default_config2();

    /** @brief Send a ping to the NanoPower unit.
     *  @param value The value to ping with. The device should send this value
     * back.
     *  @return True if the Gomspace replied with the same code, false otherwise.
     */
    bool ping(unsigned char value);
    /** @brief Reboot Gomspace. */
    void reboot();

    // See struct documentation above for more information
    /** Pointer to full housekeeping struct **/
    eps_hk_t *hk;
    /** Pointer to vi housekeeping struct **/
    eps_hk_vi_t *hk_vi;
    /** Pointer to out housekeeping struct **/
    eps_hk_out_t *hk_out;
    /** Pointer to wdt housekeeping struct **/
    eps_hk_wdt_t *hk_wdt;
    /** Pointer to basic housekeeping struct **/
    eps_hk_basic_t *hk_basic;

    /** Configuration storage struct. **/
    eps_config_t *gspace_config;
    /** Configuration 2 storage struct. **/
    eps_config2_t *gspace_config2;

   private:
    bool _set_heater(bool mode);
    // Reads in I2C data and determines if an error code was returned.
    bool _check_for_error(unsigned char port_byte);
    // Commits changes to config2 to the permanent storage of the Gomspace.
    bool _config2_confirm();
    // Flips endianness of incoming data from I2C
    void _hk_vi_endian_flip();
    void _hk_out_endian_flip();
    void _hk_wdt_endian_flip();
    void _hk_basic_endian_flip();
    #ifdef DESKTOP
    unsigned char heater=0;
    #endif
};
}  // namespace Devices

#endif
