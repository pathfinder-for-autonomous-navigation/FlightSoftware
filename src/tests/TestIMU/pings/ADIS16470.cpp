#include <ADIS16470/ADIS16470.h>
#include <Arduino.h>
#include <SPI.h>
<<<<<<< HEAD
=======
#include <ADIS16470/ADIS16470.h>
>>>>>>> 280b8f705655f870989d1a33ea2d97f6bfb1f0dc
using namespace Devices;

#define HWSERIAL Serial5

unsigned char cs = 15;
unsigned char dr = 25;
unsigned char rst = 24;
ADIS16470 gyro(cs, dr, rst, SPI);

<<<<<<< HEAD
void setup() {}

void loop() {}
=======
void setup() {
    gyro.setup();
    HWSERIAL.begin(115200);
    delay(100);
}

void loop() {
    unsigned char* burst = gyro.byte_burst();
    float gx = gyro.gyro_scale((burst[2] << 8) + burst[3]);
    float gy = gyro.gyro_scale((burst[4] << 8) + burst[5]);
    float gz = gyro.gyro_scale((burst[6] << 8) + burst[7]);
    float ax = gyro.accel_scale((burst[8] << 8) + burst[9]);
    float ay = gyro.accel_scale((burst[10] << 8) + burst[11]);
    float az = gyro.accel_scale((burst[12] << 8) + burst[13]);
    float temp = gyro.temp_scale((burst[14] << 8) + burst[15]);
    unsigned short int timestamp = (burst[16] << 8) + burst[17];
    unsigned short int checksum = (burst[18] << 8) + burst[19];
    HWSERIAL.printf("%f,%f,%f,%f,%f,%f,%f,%d,%d\n", gx, gy, gz, ax, ay, az, temp, timestamp, checksum);
    delay(100);
}
>>>>>>> 280b8f705655f870989d1a33ea2d97f6bfb1f0dc
