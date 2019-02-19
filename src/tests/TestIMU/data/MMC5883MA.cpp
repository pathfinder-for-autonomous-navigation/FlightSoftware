#include <Arduino.h>
#include <cstdint>
#include <MMC5883MA/MMC5883MA.h>
#include <HardwareSerial.h>
#include <PacketSerial.h>

using namespace Devices;

MMC5883MA imu(Wire, MMC5883MA::ADDR);
unsigned char data[14];

void setup() {
   Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_ISR);
   Serial.begin(9600);
   delay(5000);
   while(!Serial.available()){}
   //imu.setup();
}

#define PACKET_SIZE 16
void loop() {
   Serial.println("a");
   //unsigned char packet[PACKET_SIZE];
   //imu.read(data);
   // from https://stackoverflow.com/questions/6357031/how-do-you-convert-a-byte-array-to-a-hexadecimal-string-in-c
   //Serial.printf("%02X:%02X:%02X:%02X\n", data[0], data[1], data[2], data[3]);
   //delay(200);

   //MMC5883MA::magnetic_field_t magfield;
   Serial.println(imu.i2c_ping());
   //imu.get_mag(&magfield);
   //unsigned int sample_time = micros();
   delay(1000);
   //Serial.printf("%d,%f,%f,%f\n", sample_time, magfield.x, magfield.y, magfield.z);
}