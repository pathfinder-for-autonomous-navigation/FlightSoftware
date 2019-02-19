#include <Arduino.h>
#include <SPI.h>
#include <MCP4162/MCP4162.hpp>

unsigned char adc_pin = 33;
float vcc = 3.3f;
float external_resistance = 1000.0;

Devices::MCP4162 pot(SPI, 10);

void setup() {
    pinMode(adc_pin, INPUT);

    Serial.begin(9600);
}

/*! Empties the incoming serial buffer */
void empty_serial() {
  while(Serial.available())
    Serial.read();
}

void test_on_char(unsigned char code) {
    switch(code) {
        case 'p':
            // Generate resistance profile by looping through all 256 possible resistor values.
            for(unsigned char i = 0; i < 256; i++) {
                pot.set_wiper(i);
                // Measure resistance using Teensy ADC
                int adc_value = analogRead(adc_pin);
                float voltage = adc_value * vcc / 1024;
                float resistance = external_resistance * (vcc / voltage - 1);
                Serial.print("#");
                Serial.print(i);
                Serial.print(",");
                Serial.println(resistance);
                delay(1000);
            }
            break;
        default:
            Serial.println("!Unrecognized test charactar code");
    }
}

void loop() {
    Serial.println("$Ready for test character input");
    while(Serial.available() < 1);
    unsigned char code = Serial.read();
    empty_serial();
    test_on_char(code);
}