#include <Arduino.h>
#include <SpikeAndHold/SpikeAndHold.hpp>
using namespace Devices;

SpikeAndHold sph;

void setup() {

}

void loop() {
    unsigned int schedule[4] = {100,200,300,400};
    sph.load_schedule(schedule);
    sph.execute_schedule();
}