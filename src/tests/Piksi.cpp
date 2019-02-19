#include <Arduino.h>
#include <HardwareSerial.h>
#include <Piksi/Piksi.hpp>
using namespace Devices;

Piksi piksi(Serial4);
bool was_setup_successful;

void setup() {
    Serial.begin(9600);
    was_setup_successful = piksi.setup();
}

void loop() {
    Piksi::position_t position;
    Piksi::position_t baseline_position;
    Piksi::velocity_t velocity;
    while (piksi.process_buffer()) {
        piksi.get_pos_ecef(&position);
        piksi.get_baseline_ecef(&baseline_position);
        Serial.printf("GPS time of week: %d\n", position.tow);
        Serial.printf("ECEF position: %f,%f,%f\n", position.position[0], position.position[1], position.position[2]);
        Serial.printf("Number of satellites: %d\n", piksi.get_pos_ecef_nsats());
        Serial.printf("Position flags: %d\n", piksi.get_pos_ecef_flags());
        Serial.printf("Accuracy: %f\n", position.accuracy);
        Serial.println("----------");
        Serial.printf("GPS time of week: %d\n", baseline_position.tow);
        Serial.printf("Baseline ECEF position: %f,%f,%f\n", baseline_position.position[0], baseline_position.position[1], baseline_position.position[2]);
        Serial.printf("Number of satellites: %d\n", piksi.get_baseline_ecef_nsats());
        Serial.printf("Position flags: %d\n", piksi.get_baseline_ecef_flags());
        Serial.printf("Accuracy: %f\n", baseline_position.accuracy);
        Serial.println("----------");
        Serial.printf("GPS time of week: %d\n", velocity.tow);
        Serial.printf("Velocity: %f,%f,%f\n", velocity.velocity[0], velocity.velocity[1], velocity.velocity[2]);
        Serial.printf("Number of satellites: %d\n", piksi.get_vel_ecef_nsats());
        Serial.printf("Position flags: %d\n", piksi.get_vel_ecef_flags());
        Serial.printf("Accuracy: %f\n", velocity.accuracy);
        delay(3000);
    }   
}