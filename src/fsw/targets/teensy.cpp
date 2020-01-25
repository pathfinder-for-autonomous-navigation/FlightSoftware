/*
 * The <Arduino.h> import needs to be BELOW other imports. An issue occurred where the
 * "new" declaration was different across gcc-armnoneeabi and Teensy core, which
 * was only resolved by putting this import below the others so that gcc's built-in definition
 * takes precedence.
 */

#include <fsw/FCCode/MainControlLoop.hpp>
#include <common/StateFieldRegistry.hpp>
#include "flow_data.hpp"

#include <core_pins.h>
#include <wiring.h>

void pan_system_setup() {
    StateFieldRegistry registry;
    MainControlLoop fcp(registry, PAN::flow_data);

    while (true) {
        fcp.execute();
    }
}

// "ifndef UNIT_TEST" used to stop "multiple definition" linker errors when running
// tests
#ifndef UNIT_TEST
void setup() {
    pan_system_setup();
    while (true)
        ;
}

void loop() {}
#endif
