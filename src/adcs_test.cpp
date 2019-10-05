#include <Arduino.h>
#include <i2c_t3.h>
#include <ADCS.hpp>
#include <array>

Devices::ADCS adcs("adcs", Wire, Devices::ADCS::ADDRESS);

#ifndef UNIT_TEST
void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    adcs.setup();
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

bool test_set_endianess(){
    adcs.set_endianess(0);

    //lmao don't set endianess to 1
    //stuff will stop working

    // adcs.set_endianess(1);
    // adcs.set_endianess(0);
    return true;
}
bool test_set_mode(){
    adcs.set_mode(0);
    adcs.set_mode(1);
    adcs.set_mode(0);

    return true;
}
bool test_set_rwa_momentum_filter(){
    adcs.set_rwa_momentum_filter(0.77f);
    return true;
}
bool test_set_rwa_ramp_filter(){
    adcs.set_ramp_filter(0.88f);
    return true;
}
bool test_set_mtr_mode(){
    adcs.set_mtr_mode(0);
    adcs.set_mtr_mode(1);
    adcs.set_mtr_mode(0);
    return true;
}
bool test_set_mtr_command(){
    std::array<float,3> cmd = {0.01f,0.01f,-0.01f};

    adcs.set_mtr_cmd(cmd);

    return true;
}
bool test_set_mtr_limit(){

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
    adcs.set_imu_mag_filter(0.39f);
    adcs.set_imu_gyr_filter(0.4f);
    adcs.set_imu_gyr_temp_filter(0.41f);

    //works, what are bounds?
    adcs.set_imu_gyr_temp_kp(88.0f);
    adcs.set_imu_gyr_temp_ki(98.0f);
    adcs.set_imu_gyr_temp_kd(108.0f);
    adcs.set_imu_gyr_temp_desired(18.0f);
    return true;
    
}
bool test_get_who_am_i(){
    unsigned char temp = 2;
    adcs.get_who_am_i(&temp);
    
    Serial.println(temp);
    return 15 == temp;
    //Serial.println(temp[0]);
}

bool test_getset_ssa_mode(){
    //state.cpp default is 0
    adcs.set_ssa_mode(1);
    //should change from 4 to 1
    unsigned char temp = 4;
    adcs.get_ssa_mode(&temp);
    return temp == 1;

}
bool test_set_ssa_voltage_filter(){
    adcs.set_ssa_voltage_filter(0.8f);
    adcs.set_ssa_voltage_filter(0.32f);

    return true;
}
bool test_set_imu_mode(){
    adcs.set_imu_mode(0);
    adcs.set_imu_mode(1);
    adcs.set_imu_mode(2);
    adcs.set_imu_mode(0);

    return true;
}
bool test_get_ssa_vector(){

    std::array<float, 3> ssa_vec_rd = {0.5f,0.5f,0.5f};
    std::array<float, 3> ssa_vec_state = {0.69f, 0.42f, -.88f};           // Sun vector read

    adcs.get_ssa_vector(&ssa_vec_rd);

    return comp_float_arr(ssa_vec_rd,ssa_vec_state, 0.001f);

}

bool test_get_imu(){
//void ADCS::get_imu(std::array<float,3>* gyr_rd,std::array<float,3>* mag_rd,float* gyr_temp_rd){

    std::array<float, 3> gyr_rd = {1.0f,1.0f,1.0f};
    std::array<float, 3> mag_rd = {1.0f,1.0f,1.0f};;

    std::array<float, 3> gyr_state = {3.0f, 1.5f, -1.2f}; 
    std::array<float, 3> mag_state = {0.001f, 0.0008f, -0.0006f};

    float gyr_temp_rd = 1.0f;
    float gyr_temp_state = 42.0f;

    adcs.get_imu(&mag_rd, &gyr_rd, &gyr_temp_rd);

    //Serial.printf("temp: %f\n", gyr_temp_rd);

    return (comp_float_arr(mag_rd,mag_state,0.0001f) && comp_float_arr(gyr_rd,gyr_state,0.001f)
    && comp_float(gyr_temp_rd,gyr_temp_state,0.01f));

    return false;
}


//this is now working
//just not too sure about when rwa mode = 2
//values too small to verify
bool test_set_rwa_mode(){
    std::array<float,3> cmd = {200.0f,400.0f,-500.0f};
    adcs.set_rwa_mode(1,cmd);

    std::array<float,3> cmdAVG = {0.0001f,0.0002f,-0.0003f};
    adcs.set_rwa_mode(2,cmdAVG);

    return true;
}

bool test_get_rwa(){
    //dummy inital values
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

    float reference[20] = 
    {1.0f, 2.0f, 3.0f, 0.0f, 0.0f, // Voltage read
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f};

    bool ret = true;
    //check to make sure returned values are within 0.02
    for(unsigned int i=0; i< sizeof(temp)/sizeof(temp[0]); i++){
        if(abs(temp[i]-reference[i])>0.02){
            ret = false;
        }
    }

    return ret;

}

void loop() {
    Serial.println(adcs.is_functional());

    //no way to test this, but manually verified to work
    Serial.printf("set_endianess: %d\n", test_set_endianess());

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
    //Serial.printf("get_ssa_mode: %d\n", test_get_ssa_mode());

    Serial.printf("get_ssa_voltage: %d\n", test_get_ssa_voltage());

    Serial.printf("get_rwa: %d\n", test_get_rwa());

    Serial.printf("get_imu:%d\n", test_get_imu());

    delay(1000);
}
#endif
