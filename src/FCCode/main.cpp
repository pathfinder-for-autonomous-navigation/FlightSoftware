#include "StateField.hpp"
#include "StateFieldRegistry.hpp"

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
    typedef ReadableStateField<temperature_t> ReadableTemperatureStateField;
    typedef WritableStateField<gps_time_t> WritableGPSTimeStateField;
    
    auto temperature_serializer = std::make_shared<SignedIntSerializer>(-40, 125, SerializerConstants::temp_sz);
    auto gps_time_serializer = std::make_shared<GPSTimeSerializer>();

    auto tank_inner_temperature_ptr =
        std::make_shared<ReadableTemperatureStateField>("prop.temp_inner", temperature_serializer);
    auto tank_outer_temperature_ptr =
        std::make_shared<ReadableTemperatureStateField>("prop.temp_outer", temperature_serializer);
    auto firing_time_ptr = std::make_shared<WritableGPSTimeStateField>("gnc.manuever.time", gps_time_serializer);
}

// "UNIT_TEST" used to stop "multiple definition" linker errors when running
// tests
#ifndef UNIT_TEST
void setup() {
    chBegin(pan_system_setup);
    while (true);
}

void loop() {}
#endif