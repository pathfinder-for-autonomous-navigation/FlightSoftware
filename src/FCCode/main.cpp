#include "StateField.hpp"

/*
 * This import needs to be BELOW other imports. An issue occurred where the
 * "new" declaration was different across gcc-armnoneeabi and Teensy core, which
 * was only resolved by putting this import below the others so that gcc's definition
 * takes precedence.
 */
#include <Arduino.h>
#include <ChRt.h>

void pan_system_setup() {
    auto registry_ptr = std::make_shared<StateFieldRegistry>();

    // Helpful type definitions for state fields
    typedef ReadableStateField<temperature_t, temperature_t, SerializerConstants::temp_sz>
        ReadableTemperatureStateField;
    typedef WritableStateField<gps_time_t, bool, SerializerConstants::gps_time_sz>
        WritableGPSTimeStateField;

    auto tank_inner_temperature_ptr =
        std::make_shared<ReadableTemperatureStateField>("prop.temp_inner");
    auto tank_outer_temperature_ptr =
        std::make_shared<ReadableTemperatureStateField>("prop.temp_outer");
    auto firing_time_ptr = std::make_shared<WritableGPSTimeStateField>("gnc.manuever.time");
}

// "UNIT_TEST" used to stop "multiple definition" linker errors when running
// tests
#ifndef UNIT_TEST
void setup() {
    chBegin(pan_system_setup);
    while (true)
        ;
}

void loop() {}
#endif