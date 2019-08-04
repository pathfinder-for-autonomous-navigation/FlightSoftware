#include "PropulsionControl.hpp"
#include "StateField.hpp"

/*
 * This import needs to be BELOW other imports. An issue occurred where the
 * "new" declaration was different across gcc-armnoneeabi and Teensy core, which
 * was only resolved by putting this import below the others so that gcc's definition
 * takes precedence.
 */
#include <Arduino.h>

void setup() {
    auto registry_ptr = std::make_shared<StateFieldRegistry>();

    auto tank_inner_temperature_ptr = std::make_shared<
        ReadableStateField<temperature_t, temperature_t, SerializerConstants::temp_sz>>("prop.temp_inner");
    auto tank_outer_temperature_ptr = std::make_shared<
        ReadableStateField<temperature_t, temperature_t, SerializerConstants::temp_sz>>("prop.temp_outer");
    auto tank_pressure_ptr =
        std::make_shared<ReadableStateField<float, float, PropulsionControl::tank_pressure_sz>>("prop.pressure");
    auto firing_time_ptr =
        std::make_shared<WritableStateField<gps_time_t, bool, SerializerConstants::gps_time_sz>>("gnc.manuever.time");
    auto firing_vector_ptr = std::make_shared<
        WritableStateField<f_vector_t, float, PropulsionControl::prop_firing_sz>>("gnc.manuever.vector");

    std::unique_ptr<PropulsionControl> psm =
        PropulsionControl::create(registry_ptr, tank_inner_temperature_ptr, tank_outer_temperature_ptr,
                                  tank_pressure_ptr, firing_time_ptr, firing_vector_ptr);
}

void loop() {}