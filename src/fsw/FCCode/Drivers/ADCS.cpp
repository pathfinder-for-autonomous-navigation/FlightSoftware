/** @file ADCS.hpp
 * @author Kyle Krol, Shihao Cao
 * @date Spring 2018, Fall 2019
 * @brief Contains implementation for device interface to ADCS system.
 */

#include "ADCS.hpp"

#include <adcs/constants.hpp>
#include <adcs/state_registers.hpp>

#include <cstring>

using namespace Devices;

#ifndef DESKTOP
ADCS::ADCS(i2c_t3 &i2c_wire, unsigned char address)
    : I2CDevice("adcs", i2c_wire, address, 1000) {}
#else
ADCS::ADCS()
    : I2CDevice("adcs", 0) {}
#endif

bool ADCS::i2c_ping() {
    unsigned char temp = 0;
    get_who_am_i(&temp); 
    return temp==WHO_AM_I_EXPECTED;
    }

template <typename T>
void ADCS::i2c_point_and_read(unsigned char data_register, T* data, std::size_t len) {
    set_read_ptr(data_register);
    i2c_request_from(len);
    i2c_read(data, len);
}
inline float fp(signed char si, float min, float max) {
  return min + (((float) si) + 128.0f) * (max - min) / 255.0f;
}

inline float fp(unsigned char ui, float min, float max) {
  return min + ((float) ui) * (max - min) / 255.0f;
}

inline float fp(signed short si, float min, float max) {
  return min + (((float) si) + 32768.0f) * (max - min) / 65535.0f;
}

inline float fp(unsigned short ui, float min, float max) {
  return min + ((float) ui) * (max - min) / 65535.0f;
}

inline unsigned char uc(float f, float min, float max) {
  return (unsigned char)(255.0f * (f - min) / (max - min));
}

inline signed char sc(float f, float min, float max) {
  return (signed char)(255.0f * (f - min) / (max - min) - 128.0f);
}

inline unsigned short us(float f, float min, float max) {
  return (unsigned short)(65535.0f * (f - min) / (max - min));
}

inline signed short ss(float f, float min, float max) {
  return (signed short)(65535.0f * (f - min) / (max - min) - 32768.0f);
}

void ADCS::set_mode(const unsigned char mode) {
    i2c_write_to_subaddr(adcs::ADCS_MODE, mode);
}

void ADCS::set_read_ptr(const unsigned char read_ptr){
    i2c_write_to_subaddr(adcs::READ_POINTER, read_ptr);

}

void ADCS::set_rwa_mode(const unsigned char rwa_mode,const std::array<float,3>& rwa_cmd){
    i2c_write_to_subaddr(adcs::RWA_MODE, rwa_mode);

    unsigned char cmd[6];
    for(int i = 0;i<3;i++){
        unsigned short comp = 0;
        if(rwa_mode == 1)
            comp = us(rwa_cmd[i],adcs::rwa::min_speed_command,adcs::rwa::max_speed_command);
        else if(rwa_mode == 2)
            comp = us(rwa_cmd[i],adcs::rwa::min_torque,adcs::rwa::max_torque);
        cmd[2*i] = comp;
        cmd[2*i+1] = comp >> 8;
    }
    i2c_write_to_subaddr(adcs::RWA_COMMAND,cmd,6);
}

void ADCS::set_rwa_speed_filter(const float mom_filter){
    unsigned char comp = uc(mom_filter,0.0f,1.0f);
    i2c_write_to_subaddr(adcs::RWA_SPEED_FILTER, comp);
}

void ADCS::set_ramp_filter(const float ramp_filter){
    unsigned char comp = uc(ramp_filter,0.0f,1.0f);
    i2c_write_to_subaddr(adcs::RWA_RAMP_FILTER, comp);
}

void ADCS::set_mtr_mode(const unsigned char mtr_mode){
    i2c_write_to_subaddr(adcs::MTR_MODE, mtr_mode);
}

void ADCS::set_mtr_cmd(const std::array<float, 3> &mtr_cmd){
    unsigned char cmd[6];
    for(int i = 0;i<3;i++){
        unsigned short comp = us(mtr_cmd[i],adcs::mtr::min_moment,adcs::mtr::max_moment);
        cmd[2*i] = comp;
        cmd[2*i+1] = comp >> 8; 
    }
    i2c_write_to_subaddr(adcs::MTR_COMMAND,cmd,6);
}

void ADCS::set_mtr_limit(const float mtr_limit){
    unsigned char cmd[2];
    unsigned short comp = us(mtr_limit,adcs::mtr::min_moment,adcs::mtr::max_moment);
    cmd[0] = comp;
    cmd[1] = comp >> 8; 
    i2c_write_to_subaddr(adcs::MTR_LIMIT, cmd, 2);
}

void ADCS::set_ssa_mode(const unsigned char ssa_mode) {
    i2c_write_to_subaddr(adcs::SSA_MODE, ssa_mode);
}

void ADCS::set_ssa_voltage_filter(const float voltage_filter) {
    unsigned char comp = uc(voltage_filter,0.0f,1.0f);
    i2c_write_to_subaddr(adcs::SSA_VOLTAGE_FILTER, comp);
}

void ADCS::set_imu_mode(const unsigned char mode){
    i2c_write_to_subaddr(adcs::IMU_MODE, mode);
}

void ADCS::set_imu_mag_filter(const float mag_filter){
    unsigned char comp = uc(mag_filter,0.0f,1.0f);
    i2c_write_to_subaddr(adcs::IMU_MAG_FILTER, comp);
}

void ADCS::set_imu_gyr_filter(const float gyr_filter){
    unsigned char comp = uc(gyr_filter,0.0f,1.0f);
    i2c_write_to_subaddr(adcs::IMU_GYR_FILTER, comp);
}

void ADCS::set_imu_gyr_temp_filter(const float temp_filter){
    unsigned char comp = uc(temp_filter,0.0f,1.0f);
    i2c_write_to_subaddr(adcs::IMU_GYR_TEMP_FILTER, comp);
}

void float_decomp(const float input, unsigned char* temp){
    //turns the input float into 4 chars
    *(float*)(temp) = input;
}

void ADCS::set_imu_gyr_temp_kp(const float kp){
    unsigned char cmd[4];
    float_decomp(kp, cmd);
    i2c_write_to_subaddr(adcs::IMU_GYR_TEMP_KP,cmd,4);
}

void ADCS::set_imu_gyr_temp_ki(const float ki){
    unsigned char cmd[4];
    float_decomp(ki, cmd);
    i2c_write_to_subaddr(adcs::IMU_GYR_TEMP_KI,cmd,4);
}

void ADCS::set_imu_gyr_temp_kd(const float kd){
    unsigned char cmd[4];
    float_decomp(kd, cmd);
    i2c_write_to_subaddr(adcs::IMU_GYR_TEMP_KD,cmd,4);
}

void ADCS::set_imu_gyr_temp_desired(const float desired){
    unsigned char cmd = uc(desired,adcs::imu::min_eq_temp,adcs::imu::max_eq_temp);
    i2c_write_to_subaddr(adcs::IMU_GYR_TEMP_DESIRED,cmd);
}

void ADCS::set_havt(const std::bitset<adcs::havt::max_devices>& havt_table){
    //4 because 32/8 = 4
    unsigned char cmd[4];

    unsigned int encoded = (unsigned int)havt_table.to_ulong();

    //dissassemble unsigned int into 4 chars
    unsigned char * encoded_ptr = (unsigned char *)(&encoded);
    for (unsigned int i = 0; i < 4; i++){
        cmd[i] = encoded_ptr[i];
    }

    i2c_write_to_subaddr(adcs::HAVT_COMMAND, cmd, 4);
}


void ADCS::get_who_am_i(unsigned char* who_am_i) {
    i2c_point_and_read(adcs::WHO_AM_I, who_am_i, 1);
}

void ADCS::get_rwa(std::array<float, 3>* rwa_speed_rd, std::array<float, 3>* rwa_ramp_rd) {
    unsigned char readin[12];
    std::memset(readin, 0, sizeof(readin));

    #ifdef UNIT_TEST
    for(int i = 0;i<12;i++){
        readin[i] = 255;
    }
    #else
    i2c_point_and_read(adcs::RWA_SPEED_RD, readin, 12);
    #endif

    for(int i=0;i<3;i++){
        unsigned short a = readin[2*i+1] << 8;
        unsigned short b = 0xFF & readin[2*i];
        unsigned short c = a | b;
        (*rwa_speed_rd)[i] = fp(c,adcs::rwa::min_speed_read,adcs::rwa::max_speed_read);
    }
    for(int i=0;i<3;i++){
        unsigned short a = readin[2*i+1+6] << 8;
        unsigned short b = 0xFF & readin[2*i+6];
        unsigned short c = a | b;
        (*rwa_ramp_rd)[i] = fp(c,adcs::rwa::min_torque,adcs::rwa::max_torque);
    }
}

void ADCS::get_imu(std::array<float,3>* mag_rd,std::array<float,3>* gyr_rd,float* gyr_temp_rd){
    unsigned char readin[14];
    std::memset(readin, 0, sizeof(readin));

    #ifdef UNIT_TEST
    for(int i = 0;i<14;i++){
        readin[i] = 255;
    }
    #else
    i2c_point_and_read(adcs::IMU_MAG_READ, readin, 14);
    #endif

    for(int i=0;i<3;i++){
        unsigned short a = readin[2*i+1] << 8;
        unsigned short b = 0xFF & readin[2*i];
        unsigned short c = a | b;        
        (*mag_rd)[i] = fp(c,adcs::imu::min_rd_mag,adcs::imu::max_rd_mag);
    }

    for(int i=0;i<3;i++){
        unsigned short a = readin[2*i+1+6] << 8;
        unsigned short b = 0xFF & readin[2*i+6];
        unsigned short c = a | b;
        (*gyr_rd)[i] = fp(c,adcs::imu::min_rd_omega,adcs::imu::max_rd_omega);
    } 

    unsigned short c = (((unsigned short)readin[13]) << 8) | (0xFF & readin[12]);
    *gyr_temp_rd = fp(c,adcs::imu::min_rd_temp,adcs::imu::max_rd_temp);
}

#ifdef UNIT_TEST
void ADCS::set_mock_ssa_mode(const unsigned char ssa_mode) {
    mock_ssa_mode = ssa_mode;
}
#endif

void ADCS::get_ssa_mode(unsigned char* a) {
    #ifdef UNIT_TEST
    //acceleration control mode, mocking output
    *a = mock_ssa_mode;
    #else
    i2c_point_and_read(adcs::SSA_MODE, a, 1);
    #endif
}

void ADCS::get_ssa_vector(std::array<float, 3>* ssa_sun_vec) {
    unsigned char readin[6];
    std::memset(readin, 0, sizeof(readin));

    #ifdef UNIT_TEST
    for(int i = 0;i<6;i++){
        readin[i] = 255;
    }
    #else
    i2c_point_and_read(adcs::SSA_SUN_VECTOR, readin,6);
    #endif

    for(int i=0;i<3;i++){
        unsigned short c = (((unsigned short)readin[2*i+1]) << 8) | (0xFF & readin[2*i]);

        (*ssa_sun_vec)[i] = fp(c,-1.0f,1.0f);
    }

}

void ADCS::get_ssa_voltage(std::array<float, adcs::ssa::num_sun_sensors>* voltages){
    unsigned char temp[adcs::ssa::num_sun_sensors];
    std::memset(temp, 0, sizeof(temp));

    #ifdef UNIT_TEST
    for(int i = 0;i<adcs::ssa::num_sun_sensors;i++){
        temp[i] = 255;
    }
    #else
    i2c_point_and_read(adcs::SSA_VOLTAGE_READ,temp,adcs::ssa::num_sun_sensors);
    #endif
    
    for(int i = 0;i<adcs::ssa::num_sun_sensors;i++){
        (*voltages)[i] = fp(temp[i], adcs::ssa::min_voltage_rd, adcs::ssa::max_voltage_rd);
    }
}

void ADCS::get_havt(std::bitset<adcs::havt::max_devices>* havt_table){
    //4 because 32/8 = 4
    unsigned char temp[4];
    std::memset(temp, 0, sizeof(temp));

    #ifdef UNIT_TEST
    for(int i = 0;i<4;i++){
        temp[i] = 255;
    }
    #else
    i2c_point_and_read(adcs::HAVT_READ,temp, 4);
    #endif

    unsigned int encoded;
    
    //assemble chars into an int
    unsigned char * encoded_ptr = (unsigned char *)(&encoded);
    for (unsigned int i = 0; i < 4; i++){
        encoded_ptr[i] = temp[i];
    }

    (*havt_table) = std::bitset<adcs::havt::max_devices>(encoded);
}