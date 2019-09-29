/*
 * The <Arduino.h> import needs to be BELOW other imports. An issue occurred where the
 * "new" declaration was different across gcc-armnoneeabi and Teensy core, which
 * was only resolved by putting this import below the others so that gcc's built-in definition
 * takes precedence.
 */
#include "FCCode/MainControlLoopTask.hpp"
#include <StateFieldRegistry.hpp>

#include <ChRt.h>
#include <core_pins.h>
#include <wiring.h>

StateFieldRegistry registry;
MainControlLoopTask fcp_task(registry);

void pan_system_setup() {
    fcp_task.init();  // Inits the debug console
    while (true) {
        fcp_task.execute();
        digitalWrite(13, HIGH);
        chThdSleepMilliseconds(50);
        digitalWrite(13, LOW);
        chThdSleepMilliseconds(50);
    }
}

// "ifndef UNIT_TEST" used to stop "multiple definition" linker errors when running
// tests
#ifndef UNIT_TEST
void setup() {
    chBegin(pan_system_setup);
    while (true)
        ;
}

void loop() {}
#endif