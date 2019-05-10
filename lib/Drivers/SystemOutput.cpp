#include "SystemOutput.hpp"
#include <fake_device_io.hpp>

using namespace Devices;

SystemOutput::SystemOutput(const std::string& n, HardwareSerial& sp) : Device(n), _serial_port(sp) {}

bool SystemOutput::setup() { 
    _serial_port.begin(115200); 
    return true;
}
bool SystemOutput::is_functional() { return true; }
void SystemOutput::reset() { }
void SystemOutput::disable() { }

static void write_double_arr(HardwareSerial& s, volatile double (&arr)[3]) {
    char buf[3*sizeof(double)];
    double arr_buf[3];
    for(int i = 0; i < 3; i++) arr_buf[i] = arr[i];
    memcpy(buf, (char*) arr_buf, 3*sizeof(double));
    s.write(buf, 3*sizeof(double));
    s.flush();
}

void SystemOutput::send_impulse(double (&impulse)[3]) {
    _serial_port.write('i');
    write_double_arr(_serial_port, impulse);
}

void SystemOutput::send_propagated_position(double (&propagated_position)[3]) {
    _serial_port.write('p');
    write_double_arr(_serial_port, propagated_position);
}

void SystemOutput::send_propagated_velocity(double (&propagated_velocity)[3]) {
    _serial_port.write('v');
    write_double_arr(_serial_port, propagated_velocity);
}

void SystemOutput::send_propagated_other_position(double (&propagated_other_position)[3]) {
    _serial_port.write('o');
    write_double_arr(_serial_port, propagated_other_position);
}

void SystemOutput::send_propagated_other_velocity(double (&propagated_other_velocity)[3]) {
    _serial_port.write('w');
    write_double_arr(_serial_port, propagated_other_velocity);
}