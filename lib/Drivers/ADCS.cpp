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
  return (unsigned short)(65535.0f * (f - min) / (max - min));
}

inline signed short ss(float f, float min, float max) {
  return (signed short)(65535.0f * (f - min) / (max - min) - 32768.0f);
}

//should never be needed again T__T
void print_unsigned_char_array(unsigned char* array){
    for(unsigned int i = 0; i<12;i++){
        Serial.printf("arr: %u\n", array[i]);
    }
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
    i2c_write_to_subaddr(Register::MTR_MODE, mtr_mode);
}

void ADCS::set_mtr_cmd(const std::array<float, 3> mtr_cmd){
    //this method should work
    unsigned char cmd[6];
    for(int i = 0;i<3;i++){
        //comp is the converted short that will be re-assembled
        unsigned short comp = uc(mtr_cmd[i],-0.05667f,0.05667f);
        cmd[2*i] = comp >> 8;
        cmd[2*i+1] = comp & 0xFF; 
    }
    i2c_write_to_subaddr(Register::MTR_COMMAND,cmd,6);
}
void ADCS::set_mtr_limit(const float* mtr_limit){
    unsigned char cmd[2];
    unsigned short comp = uc(*mtr_limit,-0.05667f,0.05667f);
    cmd[0] = comp >> 8;
    cmd[1] = comp & 0xFF; 
    i2c_write_to_subaddr(Register::MTR_LIMIT, cmd, 2);
}
void ADCS::set_rwa_mode(unsigned char rwa_mode, std::array<float,3> rwa_cmd){
    i2c_write_to_subaddr(Register::RWA_MODE, rwa_mode);

    unsigned char cmd[6];
    for(int i = 0;i<3;i++){
        //comp is the converted short that will be re-assembled
        unsigned short comp = 0;
        if(rwa_mode == 1)
            comp = uc(rwa_cmd[i],-680.678f,680.678f);
        else if(rwa_mode == 2)
            comp = uc(rwa_cmd[i],-0.0041875f,0.0041875f);
        cmd[2*i] = comp >> 8;
        cmd[2*i+1] = comp & 0xFF; 
    }
    i2c_write_to_subaddr(Register::RWA_COMMAND,cmd,6);
}
//Tanishq's old method
//void ADCS::set_rwa_mode(unsigned char ssa_mode, const float *a) {}
void ADCS::get_who_am_i(unsigned char* who_am_i) {
    i2c_point_and_read(Register::WHO_AM_I, who_am_i, 1);
}
//Tanishq's old get_rwa header
//void ADCS::get_rwa(float *a, float *b, float *c) {
//this is a debug method
void ADCS::get_rwa_char(unsigned char* rwa_rd12) {
    i2c_point_and_read(Register::RWA_MOMENTUM_RD, rwa_rd12, 12);
}

void ADCS::get_rwa(std::array<float, 3>* rwa_momentum_rd, std::array<float, 3>* rwa_ramp_rd) {
    //read in into an array of chars
    unsigned char readin[12] = {1,1,1,1,1,1,1,1,1,1,1,1};
    i2c_point_and_read(Register::RWA_MOMENTUM_RD, readin, 12);

    for(int i=0;i<3;i++){
        unsigned short c = (((unsigned short)readin[2*i+1]) << 8) | (0xFF & readin[2*i]);
        (*rwa_momentum_rd)[i] = fp(c,-0.009189f,0.009189f);
        //(*rwa_momentum_rd).at(i) = fp(c,-0.009189f,0.009189f);
    }

    //starting from readin[6]
    for(int i=0;i<3;i++){
        unsigned short c = (((unsigned short)readin[2*i+6+1]) << 8) | (0xFF & readin[2*i+6]);
        (*rwa_ramp_rd)[i] = fp(c,-0.0041875f,0.0041875);
    }

}
void ADCS::get_imu(float *a, float *b) {

}
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