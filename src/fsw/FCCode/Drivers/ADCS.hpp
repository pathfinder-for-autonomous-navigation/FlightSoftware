/** @file ADCS.hpp
 * @author Shihao Cao
 * @date 6 Feb 2018
 * @brief Contains definition for device interface to ADCS system.
 */

#ifndef PAN_LIB_DRIVERS_ADCS_HPP_
#define PAN_LIB_DRIVERS_ADCS_HPP_

#include <adcs/constants.hpp>
#include <fsw/FCCode/Devices/I2CDevice.hpp>

#include <array>
#include <bitset>

namespace Devices {

class ADCS : public I2CDevice {
   public:
    static constexpr unsigned int ADDRESS = 0x4E;
    static constexpr unsigned int WHO_AM_I_EXPECTED = 0x0F;

    #ifdef UNIT_TEST
    unsigned int mock_ssa_mode = adcs::SSAMode::SSA_IN_PROGRESS;
    #endif
    /**
     * @brief quickly tests that the device is active and working on i2c
     * 
     * @return returns true if get_who_am_i returns the expected value, else false
     */
    virtual bool i2c_ping() override;

    /**
     * @brief Construct a new ADCS object
     * 
     * @param name The name
     * @param i2c_wire The assoicated i2c wire
     * @param address The address on i2c bus
     */
    #ifndef DESKTOP
    ADCS(i2c_t3 &i2c_wire, unsigned char address);
    #else
    ADCS();
    #endif

    /**
     * @brief Sets the read pointer, writes len bytes into data
     * 
     * @param data_register The register address to read from
     * @param data Where to read the bytes into
     * @param len The number of bytes to read
     */
    template <typename T>
    void i2c_point_and_read(unsigned char data_register, T* data, std::size_t len);

    /**
     * @brief Set the mode of the ADCS controller
     * 
     * @param mode
     * 0x00–ADCS passive mode (default) 
     * 0x01–ADCS active mode 
     */
    void set_mode(const unsigned char mode);

    /**
     * @brief Set what address the the ADCS controller should read from
     * 
     * @param read_ptr address to read from
     */
    void set_read_ptr(const unsigned char read_ptr);

    /**
     * @brief Set the reaction wheel assembly mode
     * 
     * @param rwa_mode modes specified by RWAModes
     * 
     * @param rwa_cmd command specific to mode, a float per axis
     * When in speed control, limited by max speed
     * When in acceleration control, limited by max torque
     */
    void set_rwa_mode(const unsigned char rwa_mode, const std::array<float,3>& rwa_cmd);

    /**
     * @brief Set the reaction wheel assembly speed read exponential filter constant 
     * 
     * @param rwa_speed_flt speed read exponential filter, a float from [0.0, 1.0].
     * The float maps to an eight-bit unsigned integer mapping from [0.0, 1.0].
     * Defaults to 0xFF. 
     */
    void set_rwa_speed_filter(const float rwa_speed_filter);

    /**
     * @brief Set the reaction wheel assembly ramp filter
     * 
     * @param rwa_ramp_flt ramp read exponential filter, a float from [0.0, 1.0].
     * The float maps to an eight-bit unsigned integer mapping from [0.0, 1.0].
     * Defaults to 0xFF.   
     */
    void set_ramp_filter(const float ramp_filter);

    /**
     * @brief Set the magnetorquer command mode
     * 
     * @param mtr_mode single byte representing mode
     * 0x00 – Disabled (default) 
     * 0x01 – Enabled 
     */
    void set_mtr_mode(const unsigned char mtr_mode);

    /**
     * @brief Set the magnetorquer command itself
     * 
     * @param mtr_cmd Array of three floats, mapping to 16-bit unsigned integers 
     * for the x axis, y axis, and z axis in the body frame respectively.
     * Each value maps to a magnetic moment command 
     * limited by maximum magnetic moment (and min)
     */
    void set_mtr_cmd(const std::array<float, 3>& mtr_cmd);

    /**
     * @brief Set the magnetorquer maximum moment limit
     * 
     * @param mtr_limit float representing the limit
     * Float maps to a 16-bit unsigned integer 
     * limited by maximum magnetic moment (and min)
     */
    void set_mtr_limit(const float mtr_limit);

    /**
     * @brief Set the sun sensor array mode
     * 
     * @param ssa_mode byte represinting the mode
     * 0x00 – Indicates failure of a sun vector calculation (default) 
     * 0x01 – Starts a sun vector calculation 
     * 0x02 – Indicates success of a sun vector calculation 
     */
    void set_ssa_mode(const unsigned char ssa_mode);

    /**
     * @brief Set the sun sensor array voltage exponential filter
     * 
     * @param voltage_filter float representing voltage filter value from [0.0, 1.0]. 
     * The float maps to an eight-bit unsigned integer mapping from [0.0, 1.0]. 
     * Defaults to 0xFF. 
     */
    void set_ssa_voltage_filter(const float voltage_filter);

    /**
     * @brief Set the inertial measurement unit mode
     * 
     * @param char represnting the imu mode
     * 0x00 – Use magnetometer one (default) 
     * 0x01 – Use magnetometer two 
     * 0b1X – Calibrate the magnetometer in use (specified by the free bit) 
     */
    void set_imu_mode(const unsigned char mode);

    /**
     * @brief Set the inertial mesaurement unit magnetometer exponential filter
     * 
     * @param mag_filter float representing imu filter from [0.0, 1.0].
     * The float maps to an eight-bit unsigned integer mapping from [0.0, 1.0]. 
     * Defaults to 0xFF. 
     */
    void set_imu_mag_filter(const float mag_filter);

    /**
     * @brief Set the inertial measurement unit gyroscope exponential filter
     * 
     * @param gyr_filter float representing gyroscope filter from [0.0, 1.0].
     * The float maps to an eight-bit unsigned integer mapping from [0.0, 1.0]. 
     * Defaults to 0xFF. 
     */
    void set_imu_gyr_filter(const float gyr_filter);

    /**
     * @brief Set the gyroscope temperature exponential filter constant 
     * 
     * @param temp_filter float representing temperature filter from [0.0, 1.0].
     * The float maps to an eight-bit unsigned integer mapping from [0.0, 1.0]. 
     * Defaults to 0xFF. 
     */
    void set_imu_gyr_temp_filter(const float temp_filter);

    /**
     * @brief Sets the proportional gain on the gyroscope temperature controller.
     * 
     * @param kp proportional gain, full float is passed to ADCS over i2c
     */
    void set_imu_gyr_temp_kp(const float kp);

    /**
     * @brief Sets the integral gain on the gyroscope temperature controller
     * 
     * @param ki integral gain, full float is passed to ADCS over i2c
     */
    void set_imu_gyr_temp_ki(const float ki);

    /**
     * @brief Sets the derivative gain on the gyroscope temperature controller
     * 
     * @param kd derivative gain, full float is passed to ADCS over i2c
     */
    void set_imu_gyr_temp_kd(const float kd);

    /**
     * @brief Sets the desired gyroscope equilibrium temperature. 
     * 
     * @param desired a float from imu::min_eq_temp to imu::max_eq_temp
     * The float maps to an eight-bit unsigned integer 
     * mapping from imu::min_eq_temp to imu::max_eq_temp degrees Celsius.  
     */
    void set_imu_gyr_temp_desired(const float desired);

    /**
     * @brief Sets the availability of ADCS devices.
     * 
     * Only call this method it is necessary to update the state of any ADCS devices.
     * In nominal operation, this method should be untouched.
     * 
     * @param havt_table The commanded state of the ADCS HAVT table
     */
    void set_havt(const std::bitset<adcs::havt::max_devices>& havt_table);
    
    /**
     * @brief Get the who_am_i value
     * 
     * @param who_am_i A output pointer where the read value is stored
     */
    void get_who_am_i(unsigned char *who_am_i);

    /**
     * @brief Get the reaction wheel assembly speed and ramp readings
     * 
     * @param rwa_speed_rd Pointer to output std::array of floats for speeds
     * 3 unsigned shorts map to wheel angular speeds from 
     * rwa::min_speed to rwa::max_speed in radians / s
     * in the x, y, and z direction in the body frame. 
     * 
     * @param rwa_ramp_rd Pointer to output std::array of floats for ramp
     * 3 unsigned shorts map to wheel torque commands from 
     * rwa::min_torque to rwa::max_torque in N m
     * in the x, y, and z direction in the body frame. 
     */
    void get_rwa(std::array<float, 3>* rwa_speed_rd, std::array<float, 3>* rwa_ramp_rd);

    /**
     * @brief Get the sun sensor array mode
     * 
     * @param ssa_mode Pointer to output current ssa mode value
     */
    void get_ssa_mode(unsigned char *ssa_mode);

    #ifdef UNIT_TEST
    /**
     * @brief A MOCKING METHOD, Set the ssa mode 
     * 
     * @param ssa_mode 
     */
    void set_mock_ssa_mode(const unsigned char ssa_mode);
    #endif

    /**
     * @brief Get the sun sensor array vector
     * 
     * @param ssa_sun_vec Pointer to output std::array of floats
     * Three 16-bit unsigned integers that encode the unit vector 
     * to the sun in the body frame of the spacecraft. 
     * The 16-bit unsigned integers map [-1.0, 1.0] and are encoded into floats.
     */
    void get_ssa_vector(std::array<float, 3>* ssa_sun_vec);

    /**
     * @brief Get the sun sensor array voltages
     * 
     * @param voltages Pointer to output std::array of floats of
     * Consists of 20 eight-bit unsigned integers mapped to floats specifying 
     * the voltage measurements at each photodiode on the range 
     * of ssa::min_voltage_rd to ssa::max_voltage_rd Volts
     */
    void get_ssa_voltage(std::array<float, 20>* voltages);

    /**
     * @brief Get the imu magnetoruqer, gyroscope and gyroscope temperature reading
     * 
     * @param mag_rd Pointer to output std::array of floats
     * Three unsigned shorts encode the magnetic field measurement in the 
     * body frame of the spacecraft on the range 
     * imu::min_rd_mag to imu::max_rd_mag in Tesla
     * 
     * @param gyr_rd Pointer to output std::array of floats
     * Three unsigned shorts encode the angular rate measurement in the 
     * body frame of the spacecraft on the range 
     * imu::min_rd_omega to imu::max_rd_omega in radians per second
     *  
     * @param gyr_temp_rd Pointer to output float
     * One unsigned short encodes the gyroscope temperature on 
     * the range imu::min_rd_temp imu::max_rd_temp Celcius
     */
    void get_imu(std::array<float,3>* mag_rd,std::array<float,3>* gyr_rd,float* gyr_temp_rd);
    
    /**
     * @brief Get the adcs havt table bitset
     * 
     * Access the state of the 1st device using havt_table.test(0);
     * 2nd device is at havt_table.test(1);
     * 32nd device is at havt_table.test(31);
     * 
     * The right index of a device can be found in the adcs_havt_devices.hpp register
     * in CommonSoftware.
     * 
     * If a bit of the bitset is high, that device's is_functional() returned true.
     * The mocked return will report a full set of 1's.
     * 
     * @param havt_table Pointer to the bitset that will be read into
     */
    void get_havt(std::bitset<adcs::havt::max_devices>* havt_table);
};

}  // namespace Devices
#endif
