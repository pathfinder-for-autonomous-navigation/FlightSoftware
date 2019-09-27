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
    /** **/
    virtual bool i2c_ping() override;
    /** **/
    ADCS(const std::string &name, i2c_t3 &i2c_wire, unsigned char address);
    /** **/
    template <typename T>
    void i2c_point_and_read(unsigned char data_register, T* data, std::size_t len);
    /** **/
    void i2c_read_float(unsigned char data_register, float* data, const float min, const float max, std::size_t len);
    /** **/
    void set_mode(unsigned char mode);
    /** **/
    void set_read_ptr(unsigned short read_ptr);
    /** **/
    void set_rwa_mode(unsigned char rwa_mode, std::array<float,3> rwa_cmd);
    /** **/
    void set_rwa_speed_flt(float rwa_speed_flt);
    /** **/
    void set_rwa_ramp_flt(float rwa_ramp_flt);
    /** **/
    void set_mtr_mode(const unsigned char mtr_mode);
    /** **/
    //void set_mtr_cmd(float const *mtr_cmd);
    /** **/
    void set_mtr_cmd(const std::array<float, 3> mtr_cmd);
    /** **/
    void set_mtr_limit(float const* mtr_limit);
    /** **/
    void set_ssa_adc_flt(float ssa_adc_flt);
    /** **/
    void set_ssa_mode(unsigned char ssa_mode);
    /** **/
    void set_imu_gyr_flt(float imu_gyr_flt);
    /** **/
    void set_imu_mag_flt(float imu_mag_flt);
    /** **/
    void get_who_am_i(unsigned char *who_am_i);
    /** **/
    void get_rwa(float *rwa_speed_cmd_rd, float *rwa_speed_rd, float *rwa_ramp_rd);
    /** **/
    void get_ssa_vector(float *ssa_vector);
    /** **/
    void get_ssa_voltage(float *ssa_voltage);
    /** **/
    void get_ssa_voltage_char(unsigned char* ssa_voltage);
    /** **/
    void get_ssa_mode(unsigned char *ssa_mode);
    /** **/
    void get_ssa_adcs(float *ssa_adc_rd);
    /** **/
    void get_imu(float *gyr_rd, float *mag_rd);
    /** **/
    void update_hat();
};

enum Register : unsigned char {

  WHO_AM_I = 0,
  ENDIANNESS = 1,
  ADCS_MODE = 2,
  READ_POINTER = 3,

  RWA_MODE = 4,
  RWA_COMMAND = 5,
  RWA_COMMAND_FLAG = 11,
  RWA_MOMENTUM_FILTER = 12,
  RWA_RAMP_FILTER = 13,
  RWA_MOMENTUM_RD = 14,
  RWA_RAMP_READ = 20,
  
  MTR_MODE = 26,
  MTR_COMMAND = 27,
  MTR_LIMIT = 33,
  MTR_COMMAND_FLAG = 35,

  SSA_MODE = 36,
  SSA_SUN_VECTOR = 37,
  SSA_VOLTAGE_FILTER = 43,
  SSA_VOLTAGE_READ = 44,

  IMU_MODE = 64,
  IMU_MAG_READ = 65,
  IMU_GYR_READ = 71,
  IMU_GYR_TEMP_READ = 77,
  IMU_MAG_FILTER = 79,
  IMU_GYR_FILTER = 80,
  IMU_GYR_TEMP_FILTER = 81,
  IMU_GYR_TEMP_KP = 82,
  IMU_GYR_TEMP_KI = 86,
  IMU_GYR_TEMP_KD = 90,
  IMU_GYR_TEMP_DESIRED = 94,

  ADCS_HAVT = 95

};

}  // namespace Devices

#endif