//
// src/adcs/ssa.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifdef SSA_LOG_LEVEL
  #undef LOG_LEVEL
  #define LOG_LEVEL SSA_LOG_LEVEL
#endif

#include "constants.hpp"
#include "ssa.hpp"
#include "ssa_config.hpp"
#include "utl/logging.hpp"

namespace adcs {
namespace ssa {

dev::ADS1015 adcs[5];

lin::Matrix<float, 5, 4> voltages = lin::zeros<float, 5, 4>();

void setup() {
  LOG_INFO_header
  LOG_INFO_printlnF("Initializing the SSA module")

  // Configure alert pins for input
  pinMode(adc2_alrt, INPUT);
  pinMode(adc3_alrt, INPUT);
  pinMode(adc4_alrt, INPUT);
  pinMode(adc5_alrt, INPUT);
  pinMode(adc6_alrt, INPUT);

  // Setup the ADCs' pin configurations
  adcs[0].setup(adc2_wire, adc2_addr, adc2_alrt, adcx_timeout);
  adcs[1].setup(adc3_wire, adc3_addr, adc3_alrt, adcx_timeout);
  adcs[2].setup(adc4_wire, adc4_addr, adc4_alrt, adcx_timeout);
  adcs[3].setup(adc5_wire, adc5_addr, adc5_alrt, adcx_timeout);
  adcs[4].setup(adc6_wire, adc6_addr, adc6_alrt, adcx_timeout);

  // Set the ADCs' gain value and reset them
  for (auto &adc : adcs) {
    adc.set_gain(dev::ADS1015::GAIN::ONE);
    adc.reset();
  }

#if LOG_LEVEL >= LOG_LEVEL_ERROR
  for (unsigned int i = 0; i < 5; i++) {
    if (!adcs[i].is_functional()) {
      LOG_ERROR_header
      LOG_ERROR_println("ADC" + String(i + 2) + " initialization failed")
    }
  }
#endif

  LOG_INFO_header
  LOG_INFO_println("Complete")
}

// Allows iterations across ADC channels in a loop
static dev::ADS1015::CHANNEL const channels[4] = {
  dev::ADS1015::CHANNEL::SINGLE_0, dev::ADS1015::CHANNEL::SINGLE_1,
  dev::ADS1015::CHANNEL::SINGLE_2, dev::ADS1015::CHANNEL::SINGLE_3
};

void update_sensors(float adc_flt) {
  int16_t val;
  lin::Matrix<float, 5, 4> readings(voltages);

  LOG_TRACE_header
  LOG_TRACE_printlnF("Updating SSA sensors")

  for (unsigned int j = 0; j < 4; j++) {
    // Begin read on channels[j] for each enabled ADC
    for (unsigned int i = 0; i < 5; i++)
      if (adcs[i].is_functional()) adcs[i].start_read(channels[j]);
    
    // End read on channels[j] and store the result
    for (unsigned int i = 0; i < 5; i++)
      if (adcs[i].is_functional())
        if (adcs[i].end_read(val))
          readings(i, j) = 4.096f * ((float)val) / 2048.0f;
  }

  // Filter results
  voltages = voltages + adc_flt * (readings - voltages);

#if LOG_LEVEL >= LOG_LEVEL_TRACE
  LOG_TRACE_header
  LOG_TRACE_printlnF("SSA voltage readings matrix:")
  for (unsigned int i = 0; i < voltages.rows(); i++) {
    for (unsigned int j = 0; j < voltages.cols(); j++)
      LOG_TRACE_print(" " + String(readings(i, j)))
    LOG_TRACE_println()
  }

  LOG_TRACE_header
  LOG_TRACE_printlnF("SSA filtered voltages matrix:")
  for (unsigned int i = 0; i < voltages.rows(); i++) {
    for (unsigned int j = 0; j < voltages.cols(); j++)
      LOG_TRACE_print(" " + String(voltages(i, j)))
    LOG_TRACE_println()
  }
#endif

  LOG_TRACE_header
  LOG_TRACE_printlnF("Complete")
}

unsigned char calculate_sun_vector(lin::Vector3f &sun_vec) {
  lin::Matrixf<0, 3, 20, 3> A, Q;
  lin::Vectorf<0, 20> b;
  lin::Matrix<float, 3, 3> R;
  lin::Vector3f x;

  // Prepare the least squares problem
  lin::size_t k = 0;
  for (lin::size_t i = 0; i < 5; i++) {
    // Skip ADCs that aren't functional
    if (!adcs[i].is_functional()) continue;

    for (lin::size_t j = 4 * i; j < 4 * i + 4; j++) {
      if (voltages(j) > sensor_voltage_thresh) {
        lin::row(A, k) = lin::row(normals, j);
        b(k) = voltages(j);
        k++;
      }
    }
  }
  // Ensure system is overdefined
  if (k < sensor_count_thresh) return SSAMode::SSA_FAILURE;
  // Calculate sun vector
  b.resize(k, 1); // Hacky resize call
  A.resize(k, 3);
  lin::qr(A, Q, R);
  lin::backward_sub(R, x, (lin::transpose(Q) * b).eval());
  // Return sun vector
  sun_vec = x / lin::norm(x);
  return SSAMode::SSA_COMPLETE;
}
}  // namespace ssa
}  // namespace adcs
