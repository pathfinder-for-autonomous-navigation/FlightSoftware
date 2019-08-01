#include "FakePiksi.hpp"
#include <AttitudeMath.hpp>
#include <fake_device_io.hpp>

using namespace Devices;

bool FakePiksi::setup() {
    _serial_port.begin(BAUD_RATE);
    return true;
}

bool FakePiksi::is_functional() {
    _serial_port.write('p');
    delay(2);
    if (_serial_port.read() != 'P') return false;
    return true;
}

void FakePiksi::reset() {}
void FakePiksi::disable() {}

bool FakePiksi::process_buffer() { return true; }

void FakePiksi::get_gps_time(gps_time_t *time) { read_gps_time(_serial_port, time); }

void FakePiksi::get_pos_ecef(std::array<double, 3> *position) {
    _serial_port.write("r");
    read_double_arr(_serial_port, position->data());
}

unsigned char FakePiksi::get_pos_ecef_nsats() {
    return 2;  // Doesn't matter what this number is; we just need a downlink for
               // downlink practice
}

static volatile unsigned char flag = 0;  // Value is set by get_baseline_ecef
unsigned char FakePiksi::get_pos_ecef_flags() { return flag; }

void FakePiksi::get_baseline_ecef(std::array<double, 3> *position) {
    _serial_port.write("l");
    read_double_arr(_serial_port, position->data());

    // Set GPS flag based on distance
    float p[3];
    for (int i = 0; i < 3; i++) p[i] = (*position)[i];
    if (vect_mag(p) > 200) {
        flag = 0;
    } else if (vect_mag(p) > 100) {
        flag = 1;
    } else {
        flag = 2;
    }
}

unsigned char FakePiksi::get_baseline_ecef_nsats() {
    return 2;  // Doesn't matter what this number is; we just need a downlink for
               // downlink practice
}

void FakePiksi::get_vel_ecef(std::array<double, 3> *velocity) {
    _serial_port.write("u");
    read_double_arr(_serial_port, velocity->data());
}

unsigned char FakePiksi::get_vel_ecef_nsats() {
    return 2;  // Doesn't matter what this number is; we just need some number for
               // downlink practice
}

void FakePiksi::get_base_pos_ecef(std::array<double, 3> *position) {
    _serial_port.write("x");
    read_double_arr(_serial_port, position->data());
}

unsigned int FakePiksi::get_iar() {
    return 2;  // Doesn't matter what this number is; we just need a downlink for
               // downlink practice
}