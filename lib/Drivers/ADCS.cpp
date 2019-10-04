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
    //i2c_write_to_subaddr(3, data_register);
    set_read_ptr(data_register);
    //all methods are needed lol
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

void ADCS::i2c_read_float(unsigned char data_register, float* data, const float min, const float max, std::size_t len) {
    unsigned char temp[len];
    i2c_point_and_read(data_register,temp,20);
    for(unsigned int i = 0;i<len;i++){
        data[i] = fp(temp[i], min, max);
    }

}
void ADCS::set_endianess(const unsigned char end){
    i2c_write_to_subaddr(Register::ENDIANNESS, end);
}
void ADCS::set_mode(const unsigned char mode) {
    i2c_write_to_subaddr(Register::ADCS_MODE, mode);
}
void ADCS::set_read_ptr(const unsigned char end){
    i2c_write_to_subaddr(Register::READ_POINTER, end);

}

//sets rwa_mode and rwa_cmd
void ADCS::set_rwa_mode(const unsigned char rwa_mode,const std::array<float,3>& rwa_cmd){
    i2c_write_to_subaddr(Register::RWA_MODE, rwa_mode);

    unsigned char cmd[6];
    for(int i = 0;i<3;i++){
        unsigned short comp = 0;
        if(rwa_mode == 1)
            comp = us(rwa_cmd[i],-680.678f,680.678f);
        else if(rwa_mode == 2)
            comp = us(rwa_cmd[i],-0.0041875f,0.0041875f);
        cmd[2*i] = comp;
        cmd[2*i+1] = comp >> 8;
    }
    i2c_write_to_subaddr(Register::RWA_COMMAND,cmd,6);
}
void ADCS::set_rwa_momentum_filter(const float mom_filter){
    unsigned char comp = uc(mom_filter,0.0f,1.0f);
    i2c_write_to_subaddr(Register::RWA_MOMENTUM_FILTER, comp);
}
void ADCS::set_ramp_filter(const float ramp_filter){
    unsigned char comp = uc(ramp_filter,0.0f,1.0f);
    i2c_write_to_subaddr(Register::RWA_RAMP_FILTER, comp);
}
void ADCS::set_mtr_mode(const unsigned char mtr_mode){
    i2c_write_to_subaddr(Register::MTR_MODE, mtr_mode);
}
void ADCS::set_mtr_cmd(const std::array<float, 3> &mtr_cmd){
    unsigned char cmd[6];
    for(int i = 0;i<3;i++){
        unsigned short comp = us(mtr_cmd[i],-0.05667f,0.05667f);
        cmd[2*i] = comp;
        cmd[2*i+1] = comp >> 8; 
    }
    i2c_write_to_subaddr(Register::MTR_COMMAND,cmd,6);
}
void ADCS::set_mtr_limit(const float mtr_limit){
    unsigned char cmd[2];
    unsigned short comp = us(mtr_limit,-0.05667f,0.05667f);
    cmd[0] = comp;
    cmd[1] = comp >> 8; 
    i2c_write_to_subaddr(Register::MTR_LIMIT, cmd, 2);
}
void ADCS::set_ssa_mode(const unsigned char ssa_mode) {
    i2c_write_to_subaddr(Register::SSA_MODE, ssa_mode);
}
void ADCS::set_ssa_voltage_filter(const float voltage_filter) {
    unsigned char comp = uc(voltage_filter,0.0f,1.0f);
    i2c_write_to_subaddr(Register::SSA_VOLTAGE_FILTER, comp);
}
//i have no idea what this mode entails with the last "free mode" part
void ADCS::set_imu_mode(const unsigned char mode){
    i2c_write_to_subaddr(Register::IMU_MODE, mode);
}
void ADCS::set_imu_mag_filter(const float mag_filter){
    unsigned char comp = uc(mag_filter,0.0f,1.0f);
    i2c_write_to_subaddr(Register::IMU_MAG_FILTER, comp);
}
void ADCS::set_imu_gyr_filter(const float gyr_filter){
    unsigned char comp = uc(gyr_filter,0.0f,1.0f);
    i2c_write_to_subaddr(Register::IMU_GYR_FILTER, comp);
}
void ADCS::set_imu_gyr_temp_filter(const float temp_filter){
    unsigned char comp = uc(temp_filter,0.0f,1.0f);
    i2c_write_to_subaddr(Register::IMU_GYR_TEMP_FILTER, comp);
}
void float_decomp(const float input, std::array<unsigned char, 4>* bytes){
    //unsigned char bytes[4];
    //reinterpret_cast<unsigned char const *>(&f)

    // bytes[0] = (input >> 24) & 0xFF;
    // bytes[1] = (input  >> 16) & 0xFF;
    // bytes[2] = (input >> 8) & 0xFF;
    // bytes[3] = input & 0xFF;
}
void ADCS::set_imu_gyr_temp_kp(const float kp){

}
void ADCS::set_imu_gyr_temp_ki(const float ki){

}
void ADCS::set_imu_gyr_temp_kd(const float kd){

}
void ADCS::set_imu_gyr_temp_target(const float temp){

}

//begin read methods

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
void ADCS::get_imu(std::array<float,3>* mag_rd,std::array<float,3>* gyr_rd,float* gyr_temp_rd){
    unsigned char readin[14];// = {1,1,1,1,1,1,1,1,1,1,1,1};
    i2c_point_and_read(Register::IMU_MAG_READ, readin, 14);

    for(int i=0;i<3;i++){
        unsigned short c = (((unsigned short)readin[2*i+1]) << 8) | (0xFF & readin[2*i]);
        //(*mag_rd)[i] = fp(c,imu::min_mag,imu::max_mag);
        (*mag_rd)[i] = fp(c,-0.0016f,0.0016f);
        //Serial.printf("test: %f\n",(*mag_rd)[i]);
    }

    //starting from readin[6]
    for(int i=0;i<3;i++){
        unsigned short c = (((unsigned short)readin[2*i+6+1]) << 8) | (0xFF & readin[2*i+6]);
        //(*gyr_rd)[i] = fp(c,imu::min_omega,imu::max_omega);
        (*gyr_rd)[i] = fp(c,-125.0f * 0.03490658504f,125.0f * 0.03490658504f);
        //Serial.printf("test: %f\n",(*gyr_rd)[i]);
    } 

    unsigned short c = (((unsigned short)readin[13]) << 8) | (0xFF & readin[12]);
    //gyr_temp_rd = fp(c,imu::min_read_temp,imu::max_read_temp);
    *gyr_temp_rd = fp(c,25.0f-128.0f,25.0f+128.0f);

}
void ADCS::get_ssa_mode(unsigned char* a) {
    i2c_point_and_read(Register::SSA_MODE, a, 1);
}
//void ADCS::get_ssa_vector(float *b) {
void ADCS::get_ssa_vector(std::array<float, 3>* ssa_sun_vec) {
    unsigned char readin[6];
    i2c_point_and_read(Register::SSA_SUN_VECTOR, readin,6);
    for(int i=0;i<3;i++){
        unsigned short c = (((unsigned short)readin[2*i+1]) << 8) | (0xFF & readin[2*i]);

        (*ssa_sun_vec)[i] = fp(c,-1.0f,1.0f);
        //Serial.printf("vec: %f\n",(*ssa_sun_vec)[i]);
    }

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