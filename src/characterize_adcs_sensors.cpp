#include <Arduino.h>
#include <i2c_t3.h>
#include <ADCS.hpp>
#include <DCDC.hpp>
#include <array>
#include <adcs_constants.hpp>

Devices::ADCS adcs("adcs", Wire, Devices::ADCS::ADDRESS);
std::array<float,3> wheel_commanded_rate= {0.0f,0.0f,0.0f};
std::array<float,3> magnetorquer_commanded_moment= {0.0f,0.0f,0.0f};
Devices::DCDC dcdc;

bool test_get_who_am_i(){
    unsigned char temp;
    adcs.get_who_am_i(&temp);
    
    return Devices::ADCS::WHO_AM_I_EXPECTED == temp;
}

/** Resets the GOMSpace's watchdog timer with a simple read of the Who
 *  Am I register. */
void ping_gomspace() {
  // Set the Who Am I read address
  Wire.beginTransmission(0x02);
  Wire.write(0x01);
  Wire.write(0x09);
  if (Wire.endTransmission(I2C_NOSTOP, 100000))
    Serial.println("GS Transmission Failed!");
  // Read from the address
  if (Wire.requestFrom(0x02, 3, I2C_STOP, 100000) < 3)
    Serial.println("GS Request Failed!");
  uint8_t buf[3];
  Wire.read(buf, 3);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); //13 is led
  digitalWrite(LED_BUILTIN, HIGH);
  dcdc.setup();
  dcdc.enable_adcs();
  delay(1000);
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
  delay(2000);
  adcs.i2c_set_timeout(100);
  adcs.setup();
  delay(2000);
  adcs.set_mode(ADCSMode::ADCS_ACTIVE);
  adcs.set_mtr_mode(MTRMode::MTR_ENABLED);
  adcs.set_mtr_cmd(magnetorquer_commanded_moment);
  adcs.set_rwa_mode(RWA_SPEED_CTRL,wheel_commanded_rate);
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  if (!test_get_who_am_i()){
    Serial.println("ADCS box Transmission Failed!");
  }
  digitalWrite(LED_BUILTIN, LOW);
}

void serialEvent() {
  digitalWrite(LED_BUILTIN, HIGH); // LED on
  byte incomingByte = Serial.read();
  std::array<float,3> junk{NAN};
  std::array<float,3> gyro{NAN};
  std::array<float,3> magnetometer1{NAN};
  std::array<float,3> magnetometer2{NAN};
  float gyro_temperature=NAN;
  float reading_time=NAN;
  switch (incomingByte){
    case 'r'://Get a sensor reading, takes about 400ms
      adcs.set_imu_mode(MAG1_CALIBRATE);
      delay(200);// get a good reading
      adcs.get_imu(&magnetometer1,&junk,&gyro_temperature);
      adcs.set_imu_mode(MAG2_CALIBRATE);
      delay(200);// get a good reading
      adcs.get_imu(&magnetometer2,&gyro,&gyro_temperature);
      reading_time=float(millis())/1000.0f;
      if (!adcs.i2c_data_is_valid()){
        Serial.println("ADCS box Transmission Failed!");
        gyro={NAN,NAN,NAN};
        magnetometer1={NAN,NAN,NAN};
        magnetometer2={NAN,NAN,NAN};
        gyro_temperature=NAN;
      }
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
      Serial.printf("%.7e,",wheel_commanded_rate[0]);
      Serial.printf("%.7e,",wheel_commanded_rate[1]);
      Serial.printf("%.7e,",wheel_commanded_rate[2]);
      Serial.printf("%.7e,",magnetorquer_commanded_moment[0]);
      Serial.printf("%.7e,",magnetorquer_commanded_moment[1]);
      Serial.printf("%.7e,",magnetorquer_commanded_moment[2]);
      Serial.println();
      break;
    case 'w'://Set wheel rate, in rad/s
      wheel_commanded_rate[0]= Serial.parseFloat();
      wheel_commanded_rate[1]= Serial.parseFloat();
      wheel_commanded_rate[2]= Serial.parseFloat();
      adcs.set_rwa_mode(RWA_SPEED_CTRL,wheel_commanded_rate);
      break;
    case 'm'://Set magnetorquer moment in A*m^2
      magnetorquer_commanded_moment[0]= Serial.parseFloat();
      magnetorquer_commanded_moment[1]= Serial.parseFloat();
      magnetorquer_commanded_moment[2]= Serial.parseFloat();
      adcs.set_mtr_cmd(magnetorquer_commanded_moment);
      break;
  }
  Serial.readStringUntil('\n');//clear out rest of command.
  Serial.println('a');//Write acknowledge, tells the computer the teensy is ready for more commands
  digitalWrite(LED_BUILTIN, LOW); // LED off
}

void loop() {
  delay(5000);
  ping_gomspace();
}