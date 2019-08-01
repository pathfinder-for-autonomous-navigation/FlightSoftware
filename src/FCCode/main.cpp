#include "PropulsionStateMachine.hpp"
#include "StateField.hpp"
#include <Arduino.h>

void setup() {}

void loop() {
  StateFieldRegistry registry;
  std::unique_ptr<PropulsionStateMachine> psm =
      PropulsionStateMachine::create(registry);
}