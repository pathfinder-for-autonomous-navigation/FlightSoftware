#include <adcs/constants.hpp>
#include <fsw/FCCode/Drivers/ADCS.hpp>

#include <Arduino.h>
#include <array>
#include <bitset>
#include <i2c_t3.h>

Devices::ADCS adcs_d(Wire, Devices::ADCS::ADDRESS);
int cnt;
#ifndef UNIT_TEST
void setup() {
    Serial.begin(9600);
    while(!Serial);
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    adcs_d.setup();
    cnt = 0;
}

template <class T, std::size_t N>
bool comp_float_arr(std::array<T,N> a,std::array<T,N> b,float margin){
    bool ret = true;
    for(unsigned int i = 0;i<N;i++){
        if(abs(a[i]-b[i])>margin)
            ret = false;
    }
    return ret;
}

bool comp_float(float a,float b,float margin){
    if(abs(a-b)>margin)
        return false;
    return true;
}

bool test_set_mode(){
    adcs_d.set_mode(adcs::ADCSMode::ADCS_PASSIVE);
    adcs_d.set_mode(adcs::ADCSMode::ADCS_ACTIVE);
    adcs_d.set_mode(adcs::ADCSMode::ADCS_PASSIVE);

    return true;
}

bool test_set_rwa_speed_filter(){
    //arbitrary test value
    adcs_d.set_rwa_speed_filter(0.77f);
    return true;
}

bool test_set_rwa_ramp_filter(){
    //arbitrary test value
    adcs_d.set_ramp_filter(0.88f);
    return true;
}

bool test_set_mtr_mode(){
    adcs_d.set_mtr_mode(adcs::MTRMode::MTR_DISABLED);
    adcs_d.set_mtr_mode(adcs::MTRMode::MTR_ENABLED);
    adcs_d.set_mtr_mode(adcs::MTRMode::MTR_DISABLED);
    return true;
}

bool test_set_mtr_command(){
    //arbitrary test values
    std::array<float,3> cmd = {0.01f,0.01f,-0.01f};

    adcs_d.set_mtr_cmd(cmd);

    return true;
}

bool test_set_mtr_limit(){

    //arbitrary test values
    float lim = 0.0420f;
    float nlim = -0.0234f;
    float lim1 = 0.035;
    float nlim1 = -0.045;
    adcs_d.set_mtr_limit(lim);
    adcs_d.set_mtr_limit(nlim);
    adcs_d.set_mtr_limit(lim1);
    adcs_d.set_mtr_limit(nlim1);
    
    return true;
}

bool test_set_imu_filters(){
    //arbitrary test values
    adcs_d.set_imu_mag_filter(0.39f);
    adcs_d.set_imu_gyr_filter(0.4f);
    adcs_d.set_imu_gyr_temp_filter(0.41f);

    adcs_d.set_imu_gyr_temp_kp(88.0f);
    adcs_d.set_imu_gyr_temp_ki(98.0f);
    adcs_d.set_imu_gyr_temp_kd(108.0f);
    adcs_d.set_imu_gyr_temp_desired(18.0f);
    return true;
    
}

bool test_get_who_am_i(){
    unsigned char temp;
    adcs_d.get_who_am_i(&temp);
    
    return Devices::ADCS::WHO_AM_I_EXPECTED == temp;
}

bool test_getset_ssa_mode(){
    //state.cpp default is 0
    adcs_d.set_ssa_mode(adcs::SSAMode::SSA_IN_PROGRESS);
    unsigned char temp = 0;
    adcs_d.get_ssa_mode(&temp);
    return temp == adcs::SSAMode::SSA_IN_PROGRESS;

}

bool test_set_ssa_voltage_filter(){
    //arbitrary test values
    adcs_d.set_ssa_voltage_filter(0.8f);
    adcs_d.set_ssa_voltage_filter(0.32f);

    return true;
}

bool test_set_imu_mode(){
    adcs_d.set_imu_mode(adcs::IMUMode::MAG1);
    adcs_d.set_imu_mode(adcs::IMUMode::MAG2);
    adcs_d.set_imu_mode(adcs::IMUMode::MAG1_CALIBRATE);
    adcs_d.set_imu_mode(adcs::IMUMode::MAG2_CALIBRATE);
    adcs_d.set_imu_mode(adcs::IMUMode::MAG1);

    return true;
}

bool test_set_havt(){

    std::bitset<adcs::havt::max_devices> cmd(0x0F0F0F0F);

    adcs_d.set_havt(cmd);

    return true;
}

bool test_get_ssa_vector(){

    //arbitrary test values
    std::array<float, 3> ssa_vec_rd = {0.5f,0.5f,0.5f};
    std::array<float, 3> ssa_vec_state = {0.69f, 0.42f, -.88f};

    adcs_d.get_ssa_vector(&ssa_vec_rd);

    return comp_float_arr(ssa_vec_rd,ssa_vec_state, 0.001f);

}

bool test_get_imu(){
    //arbitrary test values
    std::array<float, 3> gyr_rd = {1.0f,1.0f,1.0f};
    std::array<float, 3> mag_rd = {1.0f,1.0f,1.0f};;

    std::array<float, 3> gyr_state = {3.0f, 1.5f, -1.2f}; 
    std::array<float, 3> mag_state = {0.001f, 0.0008f, -0.0006f};

    float gyr_temp_rd = 1.0f;
    float gyr_temp_state = 42.0f;

    adcs_d.get_imu(&mag_rd, &gyr_rd, &gyr_temp_rd);

    return (comp_float_arr(mag_rd,mag_state,0.0001f) && comp_float_arr(gyr_rd,gyr_state,0.001f)
    && comp_float(gyr_temp_rd,gyr_temp_state,0.01f));

    return false;
}

bool test_set_rwa_mode(){
    //arbitrary test values
    std::array<float,3> cmd = {200.0f,400.0f,-500.0f};
    adcs_d.set_rwa_mode(1,cmd);

    std::array<float,3> cmdAVG = {0.0001f,0.0002f,-0.0003f};
    adcs_d.set_rwa_mode(2,cmdAVG);

    return true;
}

bool test_get_rwa() {
    //arbitrary test values
    std::array<float, 3> rwa_speed_rd = {1.0f,1.0f,1.0f};
    std::array<float, 3> rwa_ramp_rd = {1.0f,1.0f,1.0f};;

    std::array<float, 3> rwa_speed_state = {400.0f, -300.0f, 200.0f}; // Speed read
    std::array<float, 3> rwa_ramp_state = {0.001f, 0.002f, -0.003f};

    adcs_d.get_rwa(&rwa_speed_rd, &rwa_ramp_rd);

    return comp_float_arr(rwa_speed_rd,rwa_speed_state,0.1f)
     && comp_float_arr(rwa_ramp_rd,rwa_ramp_state,0.01f);
}

bool test_get_ssa_voltage(){
    std::array<float,20> temp;
    adcs_d.get_ssa_voltage(&temp);

    //arbitrary test values
    std::array<float,20> reference =
    {1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f};

    return comp_float_arr(reference,temp,0.02f);

}

bool test_get_havt(){
    std::bitset<adcs::havt::max_devices> temp(0);
    std::bitset<adcs::havt::max_devices> reference(0xF0F01F1F);

    Serial.printf("ref havt: %u\n", reference.to_ulong());
    adcs_d.get_havt(&temp);
    Serial.printf("read havt: %u\n", temp.to_ulong());

    return reference == temp;
}

bool test_everything(){
    return 
    adcs_d.is_functional() &&
    adcs_d.i2c_ping() &&

    test_set_mode() &&
    test_set_rwa_mode() &&
    test_set_rwa_speed_filter() &&
    test_set_rwa_ramp_filter() &&
    test_set_mtr_mode() &&
    test_set_mtr_command() &&
    test_set_mtr_limit() &&
    test_getset_ssa_mode() &&
    test_set_ssa_voltage_filter() &&
    test_set_imu_mode() &&
    test_set_imu_filters() &&
    test_set_havt() &&
    
    test_get_who_am_i() &&
    test_get_ssa_vector() &&
    test_get_ssa_voltage() &&
    test_get_rwa() &&
    test_get_imu() &&
    test_get_havt();
}

void loop() {
    Serial.println("");

    Serial.printf("is functional: %d\n", adcs_d.is_functional());

    Serial.printf("i2c ping: %d\n", adcs_d.i2c_ping());

    //no way to test this, but manually verified to work
    Serial.printf("set_mode: %d\n", test_set_mode());

    //set read ptr implicitly works

    //no way to test this, but manually verified to work;
    Serial.printf("set_rwa_mode: %d\n", test_set_rwa_mode());

    //works
    Serial.printf("set_rwa_speed_filter: %d\n", test_set_rwa_speed_filter());

    //works
    Serial.printf("set_ramp_filter: %d\n", test_set_rwa_ramp_filter());

    //works
    Serial.printf("set_mtr_mode: %d\n", test_set_mtr_mode());

    //no way to test this, but manually verified to work;
    Serial.printf("set_mtr_command: %d\n", test_set_mtr_command());
    
    //works
    Serial.printf("set_mtr_limit: %d\n", test_set_mtr_limit());

    //works
    Serial.printf("getset_ssa_mode: %d\n", test_getset_ssa_mode());

    //works
    Serial.printf("set_voltage_filter: %d\n", test_set_ssa_voltage_filter());

    //works; but check calibration idk? - Shihao
    Serial.printf("set_imu_mode: %d\n", test_set_imu_mode());

    //works;
    Serial.printf("set_imu_filters: %d\n", test_set_imu_filters());
    
    //works
    Serial.printf("set_havt: %d\n", test_set_havt());

    //-----------------

    Serial.printf("get_who_am_i: %d\n", test_get_who_am_i());

    Serial.printf("get_ssa_sun_vector: %d\n", test_get_ssa_vector());

    Serial.printf("get_ssa_voltage: %d\n", test_get_ssa_voltage());

    Serial.printf("get_rwa: %d\n", test_get_rwa());

    Serial.printf("get_imu: %d\n", test_get_imu());

    Serial.printf("get_havt: %d\n", test_get_havt());

    Serial.printf("TEST EVERYTHING: %d\n", test_everything());  

    Serial.printf("Count (for debugging): %i\n", cnt);
    cnt++;

    delay(1000);
}
#endif
