#include <Arduino.h>
#include <Gomspace.hpp>

using namespace Devices;

Gomspace::eps_hk_t hk_data;
Gomspace::eps_config_t config_data;
Gomspace::eps_config2_t config2_data;
Gomspace gs("gomspace", &hk_data, &config_data, &config2_data, Wire1, 0x02);

void setup() {
    gs.setup();
}

void loop() {
    Serial.println(gs.is_functional());
    delay(1000);
}
