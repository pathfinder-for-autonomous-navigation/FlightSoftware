/*
 * This import needs to be BELOW other imports. An issue occurred where the
 * "new" declaration was different across gcc-armnoneeabi and Teensy core, which
 * was only resolved by putting this import below the others so that gcc's definition
 * takes precedence.
 */
#include "MainControlLoopTask.hpp"
#include "StateFieldRegistry.hpp"

#include <ChRt.h>
#include <Arduino.h>

StateFieldRegistry registry;
MainControlLoopTask fcp_task(registry);

void pan_system_setup() {

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