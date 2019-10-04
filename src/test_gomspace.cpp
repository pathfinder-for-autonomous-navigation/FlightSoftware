#include <Arduino.h>
#include <Gomspace.hpp>
#include <memory>

using namespace Devices;



Gomspace::eps_hk_t hk_data;
Gomspace::eps_config_t config_data;
Gomspace::eps_config2_t config2_data;
std::shared_ptr<Gomspace> gs;

void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
    Serial.begin(9600);
    while(!Serial);
    Serial.println("Hi!");
    gs = std::make_shared<Gomspace>("gomspace", &hk_data, &config_data, &config2_data, Wire, 0x02);
    gs->setup();
}

void loop() {
    Serial.println(gs->is_functional());
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
}
