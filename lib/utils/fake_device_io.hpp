#ifndef FAKE_DEVICE_IO_HPP_
#define FAKE_DEVICE_IO_HPP_

#include <GPSTime.hpp>
#include <Arduino.h>

void write_double_arr(HardwareSerial& s, volatile double (&d)[3]);
void write_double_arr(usb_serial_class &s, volatile double (&d)[3]);
void write_double_arr_binary(usb_serial_class &s, volatile double (&d)[3]);
void read_double_arr(usb_serial_class &s, volatile double *arr);
void read_double_arr(HardwareSerial& s, volatile double *arr);
void write_gps_time(HardwareSerial& s, volatile gps_time_t& t);
void read_gps_time(HardwareSerial& s, volatile gps_time_t* t);

#endif