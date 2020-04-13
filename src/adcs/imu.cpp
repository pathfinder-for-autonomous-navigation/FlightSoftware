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

#ifdef IMU_LOG_LEVEL
  #undef LOG_LEVEL
  #define LOG_LEVEL IMU_LOG_LEVEL
#endif

#include "constants.hpp"
#include "imu.hpp"
#include "imu_calibration.hpp"
#include "imu_config.hpp"
#include "utl/convert.hpp"
#include "utl/logging.hpp"

namespace adcs {
namespace imu {

dev::LIS2MDLTR mag1;

lin::Vector3f mag1_rd({
  0.0f,
  0.0f,
  0.0f
});

dev::MMC34160PJ mag2;

lin::Vector3f mag2_rd({
  0.0f,
  0.0f,
  0.0f
});

static void update_mag1(unsigned char mag1_mode, float mag_flt) {
  lin::Vector3f data;

  // Calibrate magnetometer one if requested
  // if (mag1.is_functional() && (mag1_mode == IMUMAGMode::IMU_MAG_CALIBRATE))
  //   mag1.calibrate();
  // TODO :  ^^^^ --> Implement calibrate function

  // Attempt a read if ready and ensure it was succesful
  if (!mag1.is_functional()) return;
  if (!mag1.is_ready()) return;
  if (!mag1.read()) return;

  // Read in data and transform to the body frame
  data = {
    utl::fp(mag1.get_b_x(), min_mag1_rd_mag, max_mag1_rd_mag),
    utl::fp(mag1.get_b_y(), min_mag1_rd_mag, max_mag1_rd_mag),
    utl::fp(mag1.get_b_z(), min_mag1_rd_mag, max_mag1_rd_mag)
  };
  data = mag1_to_body * data;

  // Update the filtered magnetic field reading for magnetometer one
  mag1_rd = mag1_rd + (data - mag1_rd) * mag_flt;

  LOG_TRACE_header
  LOG_TRACE_println("Updated magnetometer one reading " + String(data(0)) + " "
      + String(data(1)) + " " + String(data(2)))

  LOG_TRACE_header
  LOG_TRACE_println("Updated, filtered magnetometer one reading "
      + String(mag1_rd(0)) + " " + String(mag1_rd(1)) + " "
      + String(mag1_rd(2)))
}

static void update_mag2(unsigned char mag2_mode, float mag_flt) {
  lin::Vector3f data;

  // Calibrate magnetometer two if requested
  if (mag2.is_functional() && (mag2_mode == IMUMAGMode::IMU_MAG_CALIBRATE))
    mag2.calibrate();

  // Attempt a read if ready and ensure it was succesful
  if (!mag2.is_functional()) return;
  if (!mag2.is_ready()) return;
  if (!mag2.read()) return;

  // Read in data and transform to the body frame
  data = {
    utl::fp(mag2.get_b_x(), min_mag2_rd_mag, max_mag2_rd_mag),
    utl::fp(mag2.get_b_y(), min_mag2_rd_mag, max_mag2_rd_mag),
    utl::fp(mag2.get_b_z(), min_mag2_rd_mag, max_mag2_rd_mag)
  };
  data = mag2_to_body * data;

  // Update the filtered magnetic field reading for magnetometer two
  mag2_rd = mag2_rd + (data - mag2_rd) * mag_flt;

  LOG_TRACE_header
  LOG_TRACE_println("Updated magnetometer two reading " + String(data(0)) + " "
      + String(data(1)) + " " + String(data(2)))

  LOG_TRACE_header
  LOG_TRACE_println("Updated, filtered magnetometer two reading "
      + String(mag2_rd(0)) + " " + String(mag2_rd(1)) + " "
      + String(mag2_rd(2)))
}

dev::LSM6DSM gyr;

lin::Vector3f gyr_rd = lin::zeros<float, 3, 1>();

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

  LOG_TRACE_header
  LOG_TRACE_println("Updated gyroscope reading " + String(data(0)) + " "
      + String(data(1)) + " " + String(data(2)))

  LOG_TRACE_header
  LOG_TRACE_println("Updated, filtered gyroscope reading " + String(gyr_rd(0))
      + " " + String(gyr_rd(1)) + " " + String(gyr_rd(2)))

  LOG_TRACE_header
  LOG_TRACE_println("Updated gyroscope temperature reading "
      + String(temp_data))

  LOG_TRACE_header
  LOG_TRACE_println("Updated, filtered gyroscope reading "
      + String(gyr_temp_rd))
}

void setup() {
  LOG_INFO_header
  LOG_INFO_printlnF("Initializing the IMU module")

  // Setup and initialize the first magnetometer
  mag1.setup(mag1_wire, mag1_timeout);
  mag1.set_sample_rate(dev::LIS2MDLTR::SR::HZ_50);
  mag1.reset();

#if LOG_LEVEL >= LOG_LEVEL_ERROR
  if (!mag1.is_functional()) {
    LOG_ERROR_header
    LOG_ERROR_printlnF("Magnetometer1 initialization failed")
  }
#endif

  // Setup and initialize the second magnetometer
  mag2.setup(mag2_wire, mag2_timeout);
  mag2.set_sample_rate(dev::MMC34160PJ::SR::HZ_50);
  mag2.reset();
  mag2.calibrate();

#if LOG_LEVEL >= LOG_LEVEL_ERROR
  if (!mag2.is_functional()) {
    LOG_ERROR_header
    LOG_ERROR_printlnF("Magnetometer2 initialization failed")
  }
#endif

  // Setup and initialize the gyroscope
  gyr.setup(gyr_wire, gyr_addr, gyr_timeout);
  gyr.reset();

#if LOG_LEVEL >= LOG_LEVEL_ERROR
  if (!gyr.is_functional()) {
    LOG_ERROR_header
    LOG_ERROR_printlnF("Gyroscope initialization failed")
  }
#endif

  LOG_INFO_header
  LOG_INFO_printlnF("Complete")
}

void update_sensors(unsigned char mag1_mode, unsigned char mag2_mode,
    float mag_flt, float gyr_flt, float gyr_temp_eq, float gyr_temp_flt,
    float gry_temp_k_p, float gyr_temp_k_i, float gyr_temp_k_d) {
  LOG_TRACE_header
  LOG_TRACE_printlnF("Updating IMU sensors")

  update_gyr(gyr_flt, gyr_temp_eq, gyr_temp_flt, gry_temp_k_p, gyr_temp_k_i,
      gyr_temp_k_d);
  update_mag1(mag1_mode, mag_flt);
  update_mag2(mag2_mode, mag_flt);

  LOG_TRACE_header
  LOG_TRACE_println("Complete")
}
}  // namespace imu
}  // namespace adcs
