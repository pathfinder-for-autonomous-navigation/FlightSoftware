/** @file ADCS.hpp
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains definition for device interface to ADCS system.
 */

#ifndef PAN_LIB_DRIVERS_ADCS_HPP_
#define PAN_LIB_DRIVERS_ADCS_HPP_

#include <I2CDevice.hpp>
#include <string>

class ADCS : public Devices::I2CDevice {
 public:
  static constexpr unsigned int ADDRESS = 0x00;
  /** **/
  virtual bool i2c_ping() override;
  /** **/
  virtual std::string& name() const override;
  /** **/
  ADCS(i2c_t3 &i2c_wire, unsigned char address);
  /** **/
  void set_mode(unsigned char mode);
  /** **/
  void set_read_ptr(unsigned short read_ptr);
  /** **/
  void set_rwa_mode(unsigned char rwa_mode, float const *rwa_cmd);
  /** **/
  void set_rwa_cmd(float const *rwa_cmd);
  /** **/
  void set_rwa_speed_flt(float rwa_speed_flt);
  /** **/
  void set_rwa_ramp_flt(float rwa_ramp_flt);
  /** **/
  void set_mtr_cmd(float const *mtr_cmd);
  /** **/
  void set_ssa_adc_flt(float ssa_adc_flt);
  /** **/
  void set_ssa_mode(unsigned char ssa_mode);
  /** **/
  void set_imu_gyr_flt(float imu_gyr_flt);
  /** **/
  void set_imu_mag_flt(float imu_mag_flt);
  /** **/
  void get_rwa(float *rwa_speed_cmd_rd, float *rwa_speed_rd,
               float *rwa_ramp_rd);
  /** **/
  void get_ssa(unsigned char &ssa_mode, float *ssa_vec_rd);
  /** **/
  void get_ssa_adcs(float *ssa_adc_rd);
  /** **/
  void get_imu(float *gyr_rd, float *mag_rd);
  /** **/
  void update_hat();
};

#endif