#include <Arduino.h>
#include <i2c_t3.h>
#include <ADCS.hpp>
#include <array>
#include <adcs_constants.hpp>

Devices::ADCS adcs("adcs", Wire, Devices::ADCS::ADDRESS);
int cnt;
#ifndef UNIT_TEST
void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    adcs.setup();
    cnt = 0;
}

template <class T, std::size_t N>
bool comp_float_arr(std::array<T,N> a,std::array<T,N> b,float margin){
    bool ret = true;
    for(unsigned int i = 0;i<N;i++){
        //Serial.printf("diff: %f\n",abs(a[i]-b[i]));
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
    adcs.set_mode(ADCSMode::ADCS_PASSIVE);
    adcs.set_mode(ADCSMode::ADCS_ACTIVE);
    adcs.set_mode(ADCSMode::ADCS_PASSIVE);

    return true;
}

bool test_set_rwa_momentum_filter(){
    //arbitrary test value
    adcs.set_rwa_momentum_filter(0.77f);
    return true;
}

bool test_set_rwa_ramp_filter(){
    //arbitrary test value
    adcs.set_ramp_filter(0.88f);
    return true;
}

bool test_set_mtr_mode(){
    adcs.set_mtr_mode(MTRMode::MTR_DISABLED);
    adcs.set_mtr_mode(MTRMode::MTR_ENABLED);
    adcs.set_mtr_mode(MTRMode::MTR_DISABLED);
    return true;
}

bool test_set_mtr_command(){
    //arbitrary test values
    std::array<float,3> cmd = {0.01f,0.01f,-0.01f};

    adcs.set_mtr_cmd(cmd);

    return true;
}

bool test_set_mtr_limit(){

    //arbitrary test values
    float lim = 0.0420f;
    float nlim = -0.0234f;
    float lim1 = 0.035;
    float nlim1 = -0.045;
    adcs.set_mtr_limit(lim);
    adcs.set_mtr_limit(nlim);
    adcs.set_mtr_limit(lim1);
    adcs.set_mtr_limit(nlim1);
    
    return true;
}

bool test_set_imu_filters(){
    //arbitrary test values
    adcs.set_imu_mag_filter(0.39f);
    adcs.set_imu_gyr_filter(0.4f);
    adcs.set_imu_gyr_temp_filter(0.41f);

    adcs.set_imu_gyr_temp_kp(88.0f);
    adcs.set_imu_gyr_temp_ki(98.0f);
    adcs.set_imu_gyr_temp_kd(108.0f);
    adcs.set_imu_gyr_temp_desired(18.0f);
    return true;
    
}

bool test_get_who_am_i(){
    unsigned char temp;
    adcs.get_who_am_i(&temp);
    
    return Devices::ADCS::WHO_AM_I_EXPECTED == temp;
}

bool test_getset_ssa_mode(){
    //state.cpp default is 0
    adcs.set_ssa_mode(SSAMode::SSA_IN_PROGRESS);
    unsigned char temp = 0;
    adcs.get_ssa_mode(&temp);
    return temp == SSAMode::SSA_IN_PROGRESS;

}

bool test_set_ssa_voltage_filter(){
    //arbitrary test values
    adcs.set_ssa_voltage_filter(0.8f);
    adcs.set_ssa_voltage_filter(0.32f);

    return true;
}

bool test_set_imu_mode(){
    adcs.set_imu_mode(IMUMode::MAG1);
    adcs.set_imu_mode(IMUMode::MAG2);
    adcs.set_imu_mode(IMUMode::MAG1_CALIBRATE);
    adcs.set_imu_mode(IMUMode::MAG2_CALIBRATE);
    adcs.set_imu_mode(IMUMode::MAG1);

    return true;
}

bool test_get_ssa_vector(){

    //arbitrary test values
    std::array<float, 3> ssa_vec_rd = {0.5f,0.5f,0.5f};
    std::array<float, 3> ssa_vec_state = {0.69f, 0.42f, -.88f};

    adcs.get_ssa_vector(&ssa_vec_rd);

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

    adcs.get_imu(&mag_rd, &gyr_rd, &gyr_temp_rd);

    return (comp_float_arr(mag_rd,mag_state,0.0001f) && comp_float_arr(gyr_rd,gyr_state,0.001f)
    && comp_float(gyr_temp_rd,gyr_temp_state,0.01f));

    return false;
}

bool test_set_rwa_mode(){
    //arbitrary test values
    std::array<float,3> cmd = {200.0f,400.0f,-500.0f};
    adcs.set_rwa_mode(1,cmd);

    std::array<float,3> cmdAVG = {0.0001f,0.0002f,-0.0003f};
    adcs.set_rwa_mode(2,cmdAVG);

    return true;
}

bool test_get_rwa() {
    //arbitrary test values
    std::array<float, 3> rwa_momentum_rd = {1.0f,1.0f,1.0f};
    std::array<float, 3> rwa_ramp_rd = {1.0f,1.0f,1.0f};;

    std::array<float, 3> rwa_momentum_state = {0.004f, 0.005f, -0.006f}; // Momentum read
    std::array<float, 3> rwa_ramp_state = {0.001f, 0.002f, -0.003f};

    adcs.get_rwa(&rwa_momentum_rd, &rwa_ramp_rd);

    return comp_float_arr(rwa_momentum_rd,rwa_momentum_state,0.0001f)
     && comp_float_arr(rwa_ramp_rd,rwa_ramp_state,0.0001f);
}

bool test_get_ssa_voltage(){
    std::array<float,20> temp;
    adcs.get_ssa_voltage(&temp);

    //arbitrary test values
    std::array<float,20> reference =
    {1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f};

    return comp_float_arr(reference,temp,0.02f);

}

bool test_everything(){
    return 
    adcs.is_functional() &&
    adcs.i2c_ping() &&

    test_set_mode() &&
    test_set_rwa_mode() &&
    test_set_rwa_momentum_filter() &&
    test_set_rwa_ramp_filter() &&
    test_set_mtr_mode() &&
    test_set_mtr_command() &&
    test_set_mtr_limit() &&
    test_getset_ssa_mode() &&
    test_set_ssa_voltage_filter() &&
    test_set_imu_mode() &&
    test_set_imu_filters() &&

    test_get_who_am_i() &&
    test_get_ssa_vector() &&
    test_get_ssa_voltage() &&
    test_get_rwa() &&
    test_get_imu();
}

void loop() {
    Serial.println("");

    Serial.printf("is functional: %d\n", adcs.is_functional());

    Serial.printf("i2c ping: %d\n", adcs.i2c_ping());

    //no way to test this, but manually verified to work
    Serial.printf("set_mode: %d\n", test_set_mode());

    //set read ptr implicitly works

    //no way to test this, but manually verified to work;
    Serial.printf("set_rwa_mode: %d\n", test_set_rwa_mode());

    //works
    Serial.printf("set_rwa_momentum_filter: %d\n", test_set_rwa_momentum_filter());

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
    

    Serial.printf("get_who_am_i: %d\n", test_get_who_am_i());

    Serial.printf("get_ssa_sun_vector: %d\n", test_get_ssa_vector());

    Serial.printf("get_ssa_voltage: %d\n", test_get_ssa_voltage());

    Serial.printf("get_rwa: %d\n", test_get_rwa());

    Serial.printf("get_imu: %d\n", test_get_imu());

    Serial.printf("TEST EVERYTHING: %d\n", test_everything());  

    Serial.printf("Count (for debugging): %i\n", cnt);
    cnt++;

    delay(1000);
}
#endif
