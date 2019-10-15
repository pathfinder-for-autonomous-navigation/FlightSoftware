/** @file ADCS.hpp
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains definition for device interface to ADCS system.
 */

#ifndef PAN_LIB_DRIVERS_ADCS_HPP_
#define PAN_LIB_DRIVERS_ADCS_HPP_

#include <I2CDevice.hpp>
#include <string>
#include <array>

namespace Devices {

class ADCS : public I2CDevice {
   public:
    static constexpr unsigned int ADDRESS = 0x4E;
    static constexpr unsigned int WHO_AM_I_EXPECTED = 0x0F;
    /**
     * @brief quickly tests that the device is active and working on i2c
     * 
     * @return true
     * returns true if get_who_am_i returns the expected value
     * @return false 
     * returns false if get_who_am_i does not return the expected value
     */
    virtual bool i2c_ping() override;
    /**
     * @brief Construct a new ADCS object
     * 
     * @param name The name
     * @param i2c_wire The assoicated i2c wire
     * @param address The address on i2c bus
     */
    ADCS(const std::string &name, i2c_t3 &i2c_wire, unsigned char address);
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
     * @brief Set the rwa mode
     * 
     * @param rwa_mode mode of RWA
     * 0x00 – Reaction wheels disabled (default) 
     * 0x01 – Speed control 
     * 0x02 – Acceleration control 
     * 
     * @param rwa_cmd Command associated with specific RWA ramp mode, 3 floats
     * The 3 floats map to three 16-bit unsigned integers mapping to the x axis, y axis, and z axis in the body frame respectively. 
     * When in speed control, the number maps [-680.678, 680.678] radiances per second. 
     * When in acceleration control, the number maps [-0.0041875, 0.0041875] N m. 
     * Defaults to 0x0000 0x0000 0x0000 
     */
    void set_rwa_mode(const unsigned char rwa_mode, const std::array<float,3>& rwa_cmd);
    /**
     * @brief Set the reaction wheel momentum read exponential filter constant 
     * 
     * @param rwa_speed_flt momentum read exponential filter, a float from [0.0, 1.0].
     * The float maps to an eight-bit unsigned integer mapping from [0.0, 1.0].
     * Defaults to 0xFF. 
     */
    void set_rwa_momentum_filter(const float mom_filter);
    /**
     * @brief Set the rwa ramp filter
     * 
     * @param rwa_ramp_flt ramp read exponential filter, a float from [0.0, 1.0].
     * The float maps to an eight-bit unsigned integer mapping from [0.0, 1.0].
     * Defaults to 0xFF.   
     */
    void set_ramp_filter(const float ramp_filter);
    /**
     * @brief Set the magnetorquer mode
     * 
     * @param mtr_mode single byte representing mode
     * 0x00 – Disabled (default) 
     * 0x01 – Enabled 
     */
    void set_mtr_mode(const unsigned char mtr_mode);
    /**
     * @brief Set the magnetorquer mode
     * 
     * @param mtr_cmd Array of three floats, mapping to 16-bit unsigned integers for the
     * x axis, y axis, and z axis in the body frame respectively. 
     * Each value maps to a magnetic moment command in the range [-0.05667, 0.05667] A m^2 
     */
    void set_mtr_cmd(const std::array<float, 3>& mtr_cmd);
    /**
     * @brief Set the magnetorquer maximum moment limit
     * 
     * @param mtr_limit float representing the limit
     * Float maps to a 16-bit unsigned integer mapping to [-0.05667, 0.05667] A m^2.
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
     * @brief Set the imu mode
     * 
     * @param char represnting the imu mode
     * 0x00 – Use magnetometer one (default) 
     * 0x01 – Use magnetometer two 
     * 0b1X – Calibrate the magnetometer in use (specified by the free bit) 
     */
    void set_imu_mode(const unsigned char mode);
    /**
     * @brief Set the imu magnetometer exponential filter
     * 
     * @param mag_filter float representing imu filter from [0.0, 1.0].
     * The float maps to an eight-bit unsigned integer mapping from [0.0, 1.0]. 
     * Defaults to 0xFF. 
     */
    void set_imu_mag_filter(const float mag_filter);
    /**
     * @brief Set the imu gyroscope exponential filter
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
     * @param desired a float from [-40.0, 85.0]
     * The float maps to an eight-bit unsigned integer mapping from [-40.0, 85.0] degrees Celsius.  
     */
    void set_imu_gyr_temp_desired(const float desired);
    
    /**
     * @brief Get the who_am_i value
     * 
     * @param who_am_i A output pointer where the read value is stored
     */
    void get_who_am_i(unsigned char *who_am_i);
    /**
     * @brief Get the rwa momentum and ramp readings
     * 
     * @param rwa_momentum_rd Pointer to output std::array of floats for momentum
     * 3 unsigned shorts map to wheel angular momentums from [-0.009189, 0.009189] kg m^2 s^-1 
     * in the x, y, and z direction in the body frame. 
     * 
     * @param rwa_ramp_rd Pointer to output std::array of floats for ramp
     * 3 unsigned shorts map to wheel torque commands from [-0.0041875, 0.0041875] N m 
     * in the x, y, and z direction in the body frame. 
     */
    void get_rwa(std::array<float, 3>* rwa_momentum_rd, std::array<float, 3>* rwa_ramp_rd);
    /**
     * @brief Get the ssa mode
     * 
     * @param ssa_mode Pointer to output current ssa mode value
     */
    void get_ssa_mode(unsigned char *ssa_mode);
    /**
     * @brief Get the ssa vector
     * 
     * @param ssa_sun_vec Pointer to output std::array of floats
     * Three 16-bit unsigned integers that encode the unit vector 
     * to the sun in the body frame of the spacecraft. 
     * The 16-bit unsigned integers map [-1.0, 1.0] and are encoded into floats.
     */
    void get_ssa_vector(std::array<float, 3>* ssa_sun_vec);
    /**
     * @brief Get the ssa voltage array
     * 
     * @param voltages Pointer to output std::array of floats of
     * Consists of 20 eight-bit unsigned integers mapped to floats specifying 
     * the voltage measurements at each photodiode on the range [0.0, 3.3] V. 
     */
    void get_ssa_voltage(std::array<float, 20>* voltages);
    /**
     * @brief Get the imu magnetoruqer, gyroscope and gyroscope temperature reading
     * 
     * @param mag_rd Pointer to output std::array of floats
     * Three unsigned shorts encode the magnetic field measurement in the 
     * body frame of the spacecraft on the range [-16e-4, 16e-4] T. 
     * 
     * @param gyr_rd Pointer to output std::array of floats
     * Three unsigned shorts encode the angular rate measurement in the 
     * body frame of the spacecraft on the range [-4.363, 4.363] radians s^-1.
     *  
     * @param gyr_temp_rd Pointer to output float
     * One unsigned short encodes the gyroscope temperature on the range [-103, 153] degrees Celsius. 
     */
    void get_imu(std::array<float,3>* mag_rd,std::array<float,3>* gyr_rd,float* gyr_temp_rd);
    
    /** **/
    void update_hat();
};

}  // namespace Devices

#endif