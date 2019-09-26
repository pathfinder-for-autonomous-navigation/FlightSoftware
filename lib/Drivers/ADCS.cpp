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
inline float fp(unsigned char ui, float min, float max) {
  return min + ((float) ui) * (max - min) / 255.0f;
}
void ADCS::i2c_read_float(unsigned char data_register, float* data, std::size_t len) {
    unsigned char temp[len];
    //float out[len] = {2.5};
    i2c_point_and_read(Register::SSA_VOLTAGE_READ,temp,20);

    for(unsigned int i = 0;i<len;i++){
        data[i] = fp(temp[i], 0.0f, 3.3f);
        //out[i] = 1.0f;
        //data[i] = 1.0f;
    }

}

void ADCS::set_mode(unsigned char mode) {}
void ADCS::set_ssa_mode(unsigned char ssa_mode) {}
void ADCS::set_mtr_cmd(const float *a) {}
void ADCS::set_rwa_mode(unsigned char ssa_mode, const float *a) {}
void ADCS::get_who_am_i(unsigned char* who_am_i) {
    //unsigned char test;

    //the block below reads correct, but causes a default case error ADCS size,
    //during read_from_subaddr
    //i2c_write_to_subaddr(Register::READ_POINTER, Register::WHO_AM_I);
    //i2c_read_from_subaddr(Register::WHO_AM_I, who_am_i, 1);
    
    //this block seems to work...
    //I'm not sure if the i2c_finish() is needed
    // i2c_write_to_subaddr(Register::READ_POINTER, Register::WHO_AM_I);
    // i2c_request_from(1);
    // i2c_read(who_am_i, 1);
    
    i2c_point_and_read(Register::WHO_AM_I, who_am_i, 1);

    //i2c_finish();

    // i2c_write_to_subaddr(Register::READ_POINTER, Register::WHO_AM_I);
    // //i2c_request_from_subaddr(Register::WHO_AM_I, 1);
    // i2c_request_from(1);
    // i2c_read(who_am_i, 1);
    // i2c_finish();
    
    //delay(100);
    //i2c_receive_data(who_am_i,1);
    //i2c_receive_data(who_am_i,1,I2C_NOSTOP);
    //i2c_finish();
}
void ADCS::get_rwa(float *a, float *b, float *c) {}
void ADCS::get_imu(float *a, float *b) {}
void ADCS::get_ssa_mode(unsigned char* a) {

    i2c_point_and_read(Register::SSA_MODE, a, 1);
    // i2c_write_to_subaddr(Register::READ_POINTER, Register::SSA_MODE);
    // i2c_request_from(1);
    // i2c_read(a, 1);
    
    //the block below causes no errors ADCS, but may not be reading correctly.
    // i2c_write_to_subaddr(Register::READ_POINTER, Register::SSA_MODE);
    // delay(100);
    // i2c_receive_data(a,1,I2C_NOSTOP);
    // i2c_finish();

    //i2c_read_from_subaddr(Register::SSA_MODE, a, 1);
}
void ADCS::get_ssa_vector(float *b) {
    
}

void ADCS::get_ssa_voltage(float* b) {
    //i2c_point_and_read(Register::SSA_VOLTAGE_READ, b, 20);
    i2c_read_float(Register::SSA_VOLTAGE_READ,b,20);
}
//this method was reserved for debugging, probably don't use in flight?
//it returns the raw char's corresponding the the encoded floats as chars
void ADCS::get_ssa_voltage_char(unsigned char* b){
    i2c_point_and_read(Register::SSA_VOLTAGE_READ,b,20);
}

void ADCS::update_hat() {}