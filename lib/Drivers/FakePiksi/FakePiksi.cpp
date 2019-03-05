#include "FakePiksi.hpp"
#include <Arduino.h>

using namespace Devices;

bool FakePiksi::setup() {
    _serial_port.begin(BAUD_RATE);
}

bool FakePiksi::is_functional() {
    _serial_port.write("PING");
    delay(2);
    if (_serial_port.read() != 0x50) return false; // Capital "P" in ASCII
    return true;
}

void FakePiksi::reset() {}
void FakePiksi::disable() {}

static std::string fake_piksi_name = "Fake Piksi";
std::string& FakePiksi::name() const {
    return fake_piksi_name;
}

void FakePiksi::get_gps_time(gps_time_t* time) {
    _serial_port.write("TIME");
    delay(2);
    char buf[sizeof(gps_time_t)];
    _serial_port.readBytes(buf, sizeof(gps_time_t));
    memcpy(time, buf, sizeof(gps_time_t));
}

void FakePiksi::get_pos_ecef(std::array<double, 3>* position) {
    _serial_port.write("POS");
    delay(2);
    char buf[sizeof(std::array<double, 3>)];
    _serial_port.readBytes(buf, sizeof(std::array<double, 3>));
    memcpy(position, buf, sizeof(std::array<double, 3>));
}

unsigned char FakePiksi::get_pos_ecef_nsats() { 
    return 2; // Doesn't matter what this number is; we just need a downlink
}

void FakePiksi::get_baseline_ecef(std::array<double, 3>* position) {
    _serial_port.write("BASELINE");
    delay(2);
    char buf[sizeof(std::array<double, 3>)];
    _serial_port.readBytes(buf, sizeof(std::array<double, 3>));
    memcpy(position, buf, sizeof(std::array<double, 3>));
}

unsigned char FakePiksi::get_baseline_ecef_nsats() { 
    return 2; // Doesn't matter what this number is; we just need a downlink
}

void FakePiksi::get_vel_ecef(std::array<double, 3>* velocity) { 
    _serial_port.write("VEL");
    delay(2);
    char buf[sizeof(std::array<double, 3>)];
    _serial_port.readBytes(buf, sizeof(std::array<double, 3>));
    memcpy(velocity, buf, sizeof(std::array<double, 3>));
}

unsigned char FakePiksi::get_vel_ecef_nsats() { 
    return 2; // Doesn't matter what this number is; we just need a downlink
}

void FakePiksi::get_base_pos_ecef(std::array<double, 3>* position) { 
    _serial_port.write("BASELINE");
    delay(2);
    char buf[sizeof(std::array<double, 3>)];
    _serial_port.readBytes(buf, sizeof(std::array<double, 3>));
    memcpy(position, buf, sizeof(std::array<double, 3>));
}

unsigned int FakePiksi::get_iar() { 
    return 2; // Doesn't matter what this number is; we just need a downlink
}