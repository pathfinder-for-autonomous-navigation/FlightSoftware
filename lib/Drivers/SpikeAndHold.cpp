#include "SpikeAndHold.hpp"
#include "../Devices/Device.hpp"
#include "Arduino.h"
#include <algorithm>
#include <climits>
#include <tuple>

using namespace Devices;

std::array<unsigned char, SpikeAndHold::NUM_VALVES>
    SpikeAndHold::DEFAULT_VALVE_PINS = {3, 4, 5, 6, 27, 28};

SpikeAndHold::SpikeAndHold(const std::string &name,
                           const std::array<unsigned char, 6> &pins,
                           unsigned char en)
    : Device(name), valve_pins(pins), enable_pin_(en) {}

bool SpikeAndHold::setup() {
  for (unsigned char i = 0; i < SpikeAndHold::NUM_VALVES; i++) {
    pinMode(valve_pins[i], OUTPUT);
  }
  pinMode(enable_pin_, OUTPUT);
  digitalWrite(enable_pin_, OFF);
  is_enabled = false;
  return true;
}

bool SpikeAndHold::is_functional() { return is_enabled; }

void SpikeAndHold::disable() {
  shut_all_valves();
  digitalWrite(enable_pin_, OFF);
  is_enabled = false;
}

void SpikeAndHold::enable() {
  digitalWrite(enable_pin_, ON);
  is_enabled = true;
}

void SpikeAndHold::reset() {
  shut_all_valves();
  disable();
  delay(10);
  enable();
}

void SpikeAndHold::execute_schedule(
    const std::array<unsigned int, SpikeAndHold::NUM_VALVES> &sch) {
  // Create 2 ms gap between valve openings, and open the ones that should be
  // opened.
  for (unsigned char i = 0; i < NUM_VALVES; i++) {
    if (sch[i] != 0) {
      digitalWrite(valve_pins[i + 2], OPEN);
      delay(3);
    }
  }

  // Order valves by firing length.
  std::array<std::pair<unsigned int, unsigned char>, 6> valve_firings;
  for (unsigned char i = 0; i < NUM_VALVES; i++)
    valve_firings[i] = {sch[i], i};
  sort(valve_firings.begin(), valve_firings.end());

  // Convert delays to differences in delays
  for (unsigned char i = 1; i < NUM_VALVES; i++) {
    valve_firings.at(i).first =
        valve_firings.at(i - 1).first - valve_firings.at(i).first;
  }
  // Wait the appropriate amount between firings, and then close the appropriate
  // valve.
  for (unsigned char i = 0; i < NUM_VALVES; i++) {
    auto x = valve_firings.at(i);
    delay(x.first);
    if (x.first != 0)
      delay(2);
    digitalWrite(valve_pins[x.second + 2], CLOSED);
  }

  // Just to be extra sure that all valves end up being closed, close them off.
  shut_all_valves();
}

void SpikeAndHold::shut_all_valves() {
  for (unsigned char i = 0; i < NUM_VALVES; i++)
    digitalWrite(valve_pins[i], CLOSED);
}