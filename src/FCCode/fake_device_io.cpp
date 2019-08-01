#include "fake_device_io.hpp"

void write_double_arr(HardwareSerial &s, volatile double (&d)[3]) {
  s.printf("%lf,%lf,%lf\n", d[0], d[1], d[2]);
}

void write_double_arr(usb_serial_class &s, volatile double (&d)[3]) {
  s.printf("%lf,%lf,%lf\n", d[0], d[1], d[2]);
}

void write_double_arr_binary(usb_serial_class &s, volatile double (&d)[3]) {
  for (int i = 0; i < 3; i++)
    s.write((char *)&d[i], sizeof(double));
}

void read_double_arr(usb_serial_class &s, volatile double *arr) {
  String data_repr = s.readStringUntil('\n');
  sscanf(data_repr.c_str(), "%lf,%lf,%lf", &arr[0], &arr[1], &arr[2]);
}

void read_double_arr(HardwareSerial &s, volatile double *arr) {
  String data_repr = s.readStringUntil('\n');
  sscanf(data_repr.c_str(), "%lf,%lf,%lf", &arr[0], &arr[1], &arr[2]);
}

void write_gps_time(HardwareSerial &s, volatile gps_time_t &t) {
  gps_time_t copy;
  copy.gpstime.wn = t.gpstime.wn;
  copy.gpstime.tow = t.gpstime.tow;
  copy.gpstime.ns = t.gpstime.ns;
  s.printf("%hu,%u,%d\n", copy.gpstime.wn, copy.gpstime.tow, copy.gpstime.ns);
}

void read_gps_time(HardwareSerial &s, volatile gps_time_t *t) {
  String data_repr = s.readStringUntil('\n');
  sscanf(data_repr.c_str(), "%hu,%u,%d", &(t->gpstime.wn), &(t->gpstime.tow),
         &(t->gpstime.ns));
}