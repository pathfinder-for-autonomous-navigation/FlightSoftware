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

// "ifndef UNIT_TEST" used to stop "multiple definition" linker errors when running
// tests
#ifndef UNIT_TEST
void setup() {}

void loop() {
    static StateFieldRegistry registry;
    static MainControlLoop fcp(registry, PAN::flow_data);

    fcp.execute();
}
#endif
