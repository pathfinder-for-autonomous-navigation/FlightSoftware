#include <Arduino.h>
#include "StateField.hpp"
#include "PropulsionStateMachine.hpp"

void setup() {

}

void loop() {
    StateFieldRegistry registry;
    std::unique_ptr<PropulsionStateMachine> psm = PropulsionStateMachine::create(registry);
}