/** @file ADCS.hpp
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains implementation for device interface to ADCS system.
 */

#include "ADCS.hpp"

ADCS::ADCS(const std::string& name, 
           i2c_t3 &i2c_wire,
           unsigned char address) : I2CDevice(name, i2c_wire, address) {}

bool ADCS::i2c_ping() { return false; }
void ADCS::set_mode(unsigned char mode) {}
void ADCS::set_ssa_mode(unsigned char ssa_mode) {}
void ADCS::set_mtr_cmd(const float* a) {}
void ADCS::set_rwa_mode(unsigned char ssa_mode, const float* a) {}
void ADCS::get_rwa(float* a, float* b, float* c) {}
void ADCS::get_imu(float* a, float* b) {}
void ADCS::get_ssa(unsigned char& a, float* b) {}
void ADCS::update_hat() {}