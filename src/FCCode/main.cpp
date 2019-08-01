#include <Arduino.h>
#include "PropulsionStateMachine.hpp"
#include "StateField.hpp"

void setup() {}

void loop() {
    StateFieldRegistry registry;
    std::unique_ptr<PropulsionStateMachine> psm = PropulsionStateMachine::create(registry);
}