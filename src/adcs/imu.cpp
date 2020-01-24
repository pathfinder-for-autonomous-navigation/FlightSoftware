//
// src/adcs/imu.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifdef IMU_DEBUG
#define DEBUG
#endif

#include "constants.hpp"
#include "imu.hpp"
#include "imu_calibration.hpp"
#include "imu_config.hpp"
#include "utl/convert.hpp"
#include "utl/debug.hpp"

namespace adcs {
namespace imu {

dev::LIS2MDLTR mag1;

dev::MMC34160PJ mag2;

lin::Vector3f mag_rd({
  0.0f,
  0.0f,
  0.0f
});

static unsigned char update_mag(unsigned char mode, float mag_flt) {
  unsigned char return_mode = mode & 0b10;
  lin::Vector3f data;

  // Switch on the current magnetometer in use
  if ((mode & 0b11111101) != 0b1) {

    // Calibrate the magnetometer if requested
    // if (mag1.is_functional() && (mode & 0b10)) mag1.calibrate();
    // TODO : Implement calibrate function    -->   ^^^^^
    // Attempt a read if ready and ensure it was succesful
    if (!mag1.is_functional()) return return_mode;
    if (!mag1.is_ready()) return return_mode;
    if (!mag1.read()) return return_mode;
    // Read in data and transform to the body frame
    data = {
      utl::fp(mag1.get_b_x(), min_mag1_rd_mag, max_mag1_rd_mag),
      utl::fp(mag1.get_b_y(), min_mag1_rd_mag, max_mag1_rd_mag),
      utl::fp(mag1.get_b_z(), min_mag1_rd_mag, max_mag1_rd_mag)
    };
    data = mag1_to_body * data;

  } else {

    // Calibrate the magnetometer if requested
    if (mag2.is_functional() && (mode & 0b10)) mag2.calibrate();
    // Attempt a read if ready and ensure it was succesful
    if (!mag2.is_functional()) return return_mode;
    if (!mag2.is_ready()) return return_mode;
    if (!mag2.read()) return return_mode;
    // Read in data and transform to the body frame
    data = {
      utl::fp(mag2.get_b_x(), min_mag2_rd_mag, max_mag2_rd_mag),
      utl::fp(mag2.get_b_y(), min_mag2_rd_mag, max_mag2_rd_mag),
      utl::fp(mag2.get_b_z(), min_mag2_rd_mag, max_mag2_rd_mag)
    };
    data = mag2_to_body * mag_rd;

  }
  // Update the filtered magnetic field reading
  mag_rd = mag_rd + (data - mag_rd) * mag_flt;
  DEBUG_print(String(mag_rd(0)) + "," + String(mag_rd(1)) + "," +
      String(mag_rd(2)) + "," + String(data(0)) + "," + String(data(1)) + "," +
      String(data(2)))
  return return_mode;
}

dev::LSM6DSM gyr;

lin::Vector3f gyr_rd({
  0.0f,
  0.0f,
  0.0f
});

float gyr_temp_rd = 0.0f;

static void update_gyr(float gyr_flt, float gyr_temp_eq, float gyr_temp_flt,
    float gry_temp_k_p, float gyr_temp_k_i, float gyr_temp_k_d) {
  lin::Vector3f data;
  float temp_data;

  // Attempt a read if ready and ensure it was succesful
  if (!gyr.is_functional()) return;
  if (!gyr.is_ready()) return;
  if (!gyr.read()) return;

  // Read in angular rate data and transform to the body frame
  data = {
    utl::fp(gyr.get_omega_x(), min_rd_omega, max_rd_omega),
    utl::fp(gyr.get_omega_y(), min_rd_omega, max_rd_omega),
    utl::fp(gyr.get_omega_z(), min_rd_omega, max_rd_omega)
  };
  data = gyr_to_body * data;

  // Read in temperature data and filter
  temp_data = utl::fp(gyr.get_temp(), min_rd_temp, max_rd_temp);
  gyr_temp_rd = gyr_temp_rd + (temp_data - gyr_temp_rd) * gyr_temp_flt;

  // Calibrate the angular rate data based on the temperature
  calibrate(data, gyr_temp_rd);
  gyr_rd = gyr_rd + (data - gyr_rd) * gyr_flt;

  // Command the gyroscope heater
  // TODO : Add gyroscope heater code

  DEBUG_print(String(gyr_rd(0)) + "," + String(gyr_rd(1)) + "," +
      String(gyr_rd(2)) + "," + String(data(0)) + "," + String(data(1)) + "," +
      String(data(2)))
}

void setup() {
  DEBUG_printlnF("#imu> Starting initialization")

  // Setup and initialize the first magnetometer
  mag1.setup(mag1_wire, mag1_timeout);
  mag1.set_sample_rate(dev::LIS2MDLTR::SR::HZ_50);
  mag1.reset();
#ifdef DEBUG
  if (mag1.is_functional()) DEBUG_printlnF("#imu> Magnetometer 1 inititated")
  else DEBUG_printlnF("#imu> Magnetometer 1 initialization failed")
#endif

  // Setup and initialize the second magnetometer
  mag2.setup(mag2_wire, mag2_timeout);
  mag2.set_sample_rate(dev::MMC34160PJ::SR::HZ_50);
  mag2.reset();
#ifdef DEBUG
  if (mag2.is_functional()) DEBUG_printlnF("#imu> Magnetometer 2 inititated")
  else DEBUG_printlnF("#imu> Magnetometer 2 initialization failed")
#endif

  // Setup and initialize the gyroscope
  gyr.setup(gyr_wire, gyr_addr, gyr_timeout);
  gyr.reset();
#ifdef DEBUG
  if (gyr.is_functional()) DEBUG_printlnF("#imu> Gyroscope inititated")
  else DEBUG_printlnF("#imu> Gyroscope initialization failed")
  DEBUG_println("#imu> Initialization complete")
#endif
}

unsigned char update_sensors(unsigned char mode, float mag_flt, float gyr_flt,
    float gyr_temp_eq, float gyr_temp_flt, float gry_temp_k_p,
    float gyr_temp_k_i, float gyr_temp_k_d) {
  update_gyr(gyr_flt, gyr_temp_eq, gyr_temp_flt, gry_temp_k_p, gyr_temp_k_i,
      gyr_temp_k_d); DEBUG_printF(",")
  return update_mag(mode, mag_flt);
}
}  // namespace imu
}  // namespace adcs
