#include <Arduino.h>
#include <HardwareSerial.h>

int incomingByte = 0;    // for incoming serial data

void setup() {
    Serial.begin(9600);    // opens serial port, sets data rate to 9600 bps
    Serial4.begin(115200);    // opens serial port, sets data rate to 9600 bps
}

void loop() {
  // send data only when you receive data:
  if (Serial4.available() > 0) {
  
    // read the incoming byte:
    incomingByte = Serial4.read();
  
    // say what you got:
    Serial.print(incomingByte, HEX);
  }
  
}