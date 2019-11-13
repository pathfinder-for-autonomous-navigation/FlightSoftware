#include <Arduino.h>
#include <i2c_t3.h>
#include <ADCS.hpp>
#include <array>
#include <adcs_constants.hpp>

Devices::ADCS adcs("adcs", Wire, Devices::ADCS::ADDRESS);
std::array<float,3> wheel_commanded_momentum= {0.0f,0.0f,0.0f};
std::array<float,3> magnetorquer_commanded_moment= {0.0f,0.0f,0.0f};


bool test_get_who_am_i(){
    unsigned char temp;
    adcs.get_who_am_i(&temp);
    
    return Devices::ADCS::WHO_AM_I_EXPECTED == temp;
}

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
  adcs.setup();
  test_get_who_am_i();
  adcs.set_mode(ADCSMode::ADCS_ACTIVE);
  adcs.set_mtr_mode(MTRMode::MTR_ENABLED);
  adcs.set_rwa_momentum_filter(0.00f);
  adcs.set_ramp_filter(0.00f);
  adcs.set_mtr_cmd(magnetorquer_commanded_moment);
  adcs.set_rwa_mode(RWA_SPEED_CTRL,wheel_commanded_momentum);
  adcs.set_imu_mag_filter(0.0f);
  adcs.set_imu_gyr_filter(0.0f);
  adcs.set_imu_gyr_temp_filter(0.0f);
}

void serialEvent() {
  byte incomingByte = Serial.read();
  switch (incomingByte){
    case 'r'://Get a sensor reading, takes about 400ms
      std::array<float,3> junk;
      std::array<float,3> gyro;
      std::array<float,3> magnetometer1;
      std::array<float,3> magnetometer2;
      float gyro_temperature;
      float reading_time;
      adcs.set_imu_mode(MAG1_CALIBRATE);
      delay(200);// get a good reading
      adcs.get_imu(&magnetometer1,&junk,&gyro_temperature);
      adcs.set_imu_mode(MAG2_CALIBRATE);
      delay(200);// get a good reading
      adcs.get_imu(&magnetometer2,&gyro,&gyro_temperature);
      reading_time=float(millis())/1000.0f;
      Serial.printf("%.7e,",reading_time);
      Serial.printf("%.7e,",magnetometer1[0]);
      Serial.printf("%.7e,",magnetometer1[1]);
      Serial.printf("%.7e,",magnetometer1[2]);
      Serial.printf("%.7e,",magnetometer2[0]);
      Serial.printf("%.7e,",magnetometer2[1]);
      Serial.printf("%.7e,",magnetometer2[2]);
      Serial.printf("%.7e,",gyro[0]);
      Serial.printf("%.7e,",gyro[1]);
      Serial.printf("%.7e,",gyro[2]);
      Serial.printf("%.7e,",gyro_temperature);
      Serial.printf("%.7e,",wheel_commanded_momentum[0]);
      Serial.printf("%.7e,",wheel_commanded_momentum[1]);
      Serial.printf("%.7e,",wheel_commanded_momentum[2]);
      Serial.printf("%.7e,",magnetorquer_commanded_moment[0]);
      Serial.printf("%.7e,",magnetorquer_commanded_moment[1]);
      Serial.printf("%.7e,",magnetorquer_commanded_moment[2]);
      Serial.println();
      break;
    case 'w'://Set wheel momentum, in Nms
      wheel_commanded_momentum[0]= Serial.parseFloat();
      wheel_commanded_momentum[1]= Serial.parseFloat();
      wheel_commanded_momentum[2]= Serial.parseFloat();
      adcs.set_rwa_mode(RWA_SPEED_CTRL,wheel_commanded_momentum);
      break;
    case 'm'://Set magnetorquer moment in A*m^2
      magnetorquer_commanded_moment[0]= Serial.parseFloat();
      magnetorquer_commanded_moment[1]= Serial.parseFloat();
      magnetorquer_commanded_moment[2]= Serial.parseFloat();
      adcs.set_mtr_cmd(magnetorquer_commanded_moment);
      break;
  }
  Serial.readStringUntil('\n');//clear out rest of command.
  Serial.write('a');//Write acknowledge, tells the computer the teensy is ready for more commands
}

void loop() {
  
  delay(1000);
}