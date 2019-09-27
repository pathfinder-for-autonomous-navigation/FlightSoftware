/** @file ADCS.hpp
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains implementation for device interface to ADCS system.
 */

#include "ADCS.hpp"

using namespace Devices;

ADCS::ADCS(const std::string &name, i2c_t3 &i2c_wire, unsigned char address)
    : I2CDevice(name, i2c_wire, address) {}

bool ADCS::i2c_ping() {
    unsigned char temp = 4;
    get_who_am_i(&temp); 
    return temp==15;
    }

template <typename T>
void ADCS::i2c_point_and_read(unsigned char data_register, T* data, std::size_t len) {
    //Devices:Register::READ_POINTER = 3
    i2c_write_to_subaddr(3, data_register);
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
//float f to unsigned char
inline unsigned char uc(float f, float min, float max) {
  return (unsigned char)(255.0f * (f - min) / (max - min));
}

inline signed char sc(float f, float min, float max) {
  return (signed char)(255.0f * (f - min) / (max - min) - 128.0f);
}

inline unsigned short us(float f, float min, float max) {
  return (unsigned short)(65535.0f * (f - min) / (max / min));
}

inline signed short ss(float f, float min, float max) {
  return (signed short)(65535.0f * (f - min) / (max - min) - 32768.0f);
}

void ADCS::i2c_read_float(unsigned char data_register, float* data, const float min, const float max, std::size_t len) {
    unsigned char temp[len];
    i2c_point_and_read(data_register,temp,20);
    for(unsigned int i = 0;i<len;i++){
        data[i] = fp(temp[i], min, max);
    }

}

void ADCS::set_mode(unsigned char mode) {
    i2c_write_to_subaddr(Register::ADCS_MODE, mode);
}
void ADCS::set_ssa_mode(unsigned char ssa_mode) {
    i2c_write_to_subaddr(Register::SSA_MODE, ssa_mode);
}
void ADCS::set_mtr_mode(const unsigned char mtr_mode){

}
// void ADCS::set_mtr_cmd(const float *a) {
//     unsigned char cmd[6];
//     for(int i = 0;i<6;i++){
//         cmd[i] = uc(a[i],-)
//     }
//     i2c_write_to_subaddr(Register::MTR_COMMAND, std::array<float,3>);
// }
void ADCS::set_mtr_cmd(const std::array<float, 3> mtr_cmd){
    // unsigned short cmd[6];
    // for(int i =0;i<6;i++){
    //     cmd[i] = uc(mtr_cmd[i],-0.05667f,0.05667f);
    // }
    // i2c_write_to_subaddr(Register::MTR_COMMAND,cmd);
    // unsigned char cmd[3];
    // for(int i =0;i<3;i++){
    //    cmd[i] = uc(mtr_cmd[i],-0.05667f,0.05667f);
    // }
    // i2c_write_to_subaddr(Register::MTR_COMMAND,cmd,3);
    unsigned char cmd[6];
    for(int i = 0;i<3;i++){
        unsigned short comp = uc(mtr_cmd[i],-0.05667f,0.05667f);
        // cmd[2*i] = comp & 0xFF;
        // cmd[2*i+1] = comp >> 8; 
        cmd[2*i] = comp >> 8;
        cmd[2*i+1] = comp & 0xFF; 
    }
    i2c_write_to_subaddr(Register::MTR_COMMAND,cmd,6);
}
void ADCS::set_mtr_limit(const float* mtr_limit){}
void ADCS::set_rwa_mode(unsigned char ssa_mode, const float *a) {}
void ADCS::get_who_am_i(unsigned char* who_am_i) {
    i2c_point_and_read(Register::WHO_AM_I, who_am_i, 1);
}
void ADCS::get_rwa(float *a, float *b, float *c) {}
void ADCS::get_imu(float *a, float *b) {}
void ADCS::get_ssa_mode(unsigned char* a) {
    i2c_point_and_read(Register::SSA_MODE, a, 1);
}
void ADCS::get_ssa_vector(float *b) {
    
}

void ADCS::get_ssa_voltage(float* b) {
    i2c_read_float(Register::SSA_VOLTAGE_READ,b,0.0f,3.3f,20);
}
//this method was reserved for debugging, probably don't use in flight?
//it returns the raw char's corresponding the the encoded floats as chars
void ADCS::get_ssa_voltage_char(unsigned char* b){
    i2c_point_and_read(Register::SSA_VOLTAGE_READ,b,20);
}

void ADCS::update_hat() {}