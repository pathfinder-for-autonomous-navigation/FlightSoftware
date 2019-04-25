#include "SystemOutput.hpp"

#ifdef DEBUG

using namespace Devices;

SystemOutput::SystemOutput(HardwareSerial& sp) : _serial_port(sp) {}

bool SystemOutput::setup() { 
    _serial_port.begin(115200); 
    return true;
}
bool SystemOutput::is_functional() { return true; }
void SystemOutput::reset() { }
void SystemOutput::disable() { }

void SystemOutput::send_impulse(double (&impulse)[3]) {
    while(_serial_port.available()) _serial_port.read();
    char buf[3*sizeof(double)];
    double imp[3];
    copy_arr(impulse, &imp);
    memcpy(buf, &imp, 3*sizeof(double));
    _serial_port.write('i');
    _serial_port.write(buf, 3*sizeof(double));
    _serial_port.flush();
}

void SystemOutput::send_propagated_position(double (&propagated_position)[3]) {
    while(_serial_port.available()) _serial_port.read();
    char buf[3*sizeof(double)];
    double prop_p[3];
    copy_arr(propagated_position, &prop_p);
    memcpy(buf, &prop_p, 3*sizeof(double));
    _serial_port.write('p');
    _serial_port.write(buf, 3*sizeof(double));
    _serial_port.flush();
}

void SystemOutput::send_propagated_velocity(double (&propagated_velocity)[3]) {
    while(_serial_port.available()) _serial_port.read();
    char buf[3*sizeof(double)];
    double prop_v[3];
    copy_arr(propagated_velocity, &prop_v);
    memcpy(buf, &prop_v, 3*sizeof(double));
    _serial_port.write('v');
    _serial_port.write(buf, 3*sizeof(double));
    _serial_port.flush();
}

void SystemOutput::send_propagated_other_position(double (&propagated_other_position)[3]) {
    while(_serial_port.available()) _serial_port.read();
    char buf[3*sizeof(double)];
    double prop_o_p[3];
    copy_arr(propagated_other_position, &prop_o_p);
    memcpy(buf, &prop_o_p, 3*sizeof(double));
    _serial_port.write('o');
    _serial_port.write(buf, 3*sizeof(double));
    _serial_port.flush();
}

void SystemOutput::send_propagated_other_velocity(double (&propagated_other_velocity)[3]) {
    while(_serial_port.available()) _serial_port.read();
    char buf[3*sizeof(double)];
    double prop_o_v[3];
    copy_arr(propagated_other_velocity, &prop_o_v);
    memcpy(buf, &prop_o_v, 3*sizeof(double));
    _serial_port.write('w');
    _serial_port.write(buf, 3*sizeof(double));
    _serial_port.flush();
}

#endif