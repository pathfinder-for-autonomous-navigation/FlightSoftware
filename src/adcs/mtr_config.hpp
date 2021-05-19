//
// src/adcs/mtr_config.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol         kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO : Determine magnetic torque rod command transformation

#ifndef SRC_ADCS_MTR_CONFIG_HPP_
#define SRC_ADCS_MTR_CONFIG_HPP_

#include <lin.hpp>

namespace adcs {
namespace mtr {

/** Forward PWM pin for the zeroth magnetorquer. */
static unsigned int const mtr0_f_pin = 7;
/** Backward PWM pin for the zeroth magnetorquer. */
static unsigned int const mtr0_r_pin = 8;
/** Forward PWM pin for the first magnetorquer. */
static unsigned int const mtr1_f_pin = 35;
/** Backward PWM pin for the first magnetorquer. */
static unsigned int const mtr1_r_pin = 36;
/** Forward PWM pin for the second magnetorquer. */
static unsigned int const mtr2_f_pin = 2;
/** Backward PWM pin for the second magnetorquer. */
static unsigned int const mtr2_r_pin = 14;

/** Transforms a vector from the body frame into the commanding frame for the
 *  magnetic torque rods. */
static constexpr lin::Matrix3x3f body_to_mtr = {
#ifdef PAN_LEADER
   0.0f,  0.0f, -1.0f,
  -1.0f,  0.0f,  0.0f,
   0.0f, -1.0f,  0.0f
#elif PAN_FOLLOWER
   0.0f, 0.0f, 1.0f,
  -1.0f, 0.0f, 0.0f,
   0.0f, 1.0f, 0.0f
#else
static_assert(false, "Must define PAN_LEADER or PAN_FOLLOWER");
#endif
};

/** Transforms a vector from the magnetic torque rod frame to the body frame of
 *  the spacecraft. */
static constexpr lin::Matrix3x3f mtr_to_body = {
#ifdef PAN_LEADER
   0.0f, -1.0f,  0.0f,
   0.0f,  0.0f, -1.0f,
  -1.0f,  0.0f,  0.0f
#elif PAN_FOLLOWER
  0.0f, -1.0f, 0.0f,
  0.0f,  0.0f, 1.0f,
  1.0f,  0.0f, 0.0f
#else
static_assert(false, "Must define PAN_LEADER or PAN_FOLLOWER");
#endif
};

static_assert(lin::fro(body_to_mtr * mtr_to_body - lin::identity<lin::Matrix3x3f>()) < 1.0e-5f, "");

}  // namespace mtr
}  // namespace adcs

#endif
