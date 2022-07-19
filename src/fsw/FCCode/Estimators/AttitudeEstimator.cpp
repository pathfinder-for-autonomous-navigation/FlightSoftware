#include "AttitudeEstimator.hpp"

#include <adcs/constants.hpp>

#include <gnc/attitude_estimator.hpp>
#include <gnc/constants.hpp>

#include <lin/core.hpp>
#include <lin/generators.hpp>
#include <lin/math.hpp>
#include <lin/queries.hpp>
#include <lin/references.hpp>
#include <lin/views.hpp>
#include <orb/Orbit.h>

/* If the attitude estimator isn't valid for this many cycles while we're trying
 * to hold attitude the fault will be tripped.
 */
TRACKED_CONSTANT_SC(size_t, ATTITUDE_ESTIMATOR_FAULT_PERSISTANCE, 150);

AttitudeEstimator::AttitudeEstimator(StateFieldRegistry &registry) 
    : ControlTask<void>(registry),
      time_valid_fp(FIND_READABLE_FIELD(bool, time.valid)),
      time_s_fp(FIND_INTERNAL_FIELD(double, time.s)),
      orbit_valid_fp(FIND_READABLE_FIELD(bool, orbit.valid)),
      orbit_pos_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.pos)),
      adcs_gyr_functional_fp(FIND_READABLE_FIELD(bool, adcs_monitor.havt_device0)),
      adcs_gyr_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.gyr_vec)),
      adcs_mag1_functional_fp(FIND_READABLE_FIELD(bool, adcs_monitor.havt_device1)),
      adcs_mag1_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.mag1_vec)),
      adcs_mag2_functional_fp(FIND_READABLE_FIELD(bool, adcs_monitor.havt_device2)),
      adcs_mag2_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.mag2_vec)),
      adcs_ssa_mode_fp(FIND_READABLE_FIELD(unsigned char, adcs_monitor.ssa_mode)),
      adcs_ssa_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.ssa_vec)),
      attitude_estimator_b_valid_f("attitude_estimator.b_valid", Serializer<bool>()),
      attitude_estimator_b_body_f("attitude_estimator.b_body", Serializer<lin::Vector3f>(0.0, 1.0e-4, 12)),
      attitude_estimator_valid_f("attitude_estimator.valid", Serializer<bool>()),
      attitude_estimator_q_body_eci_f("attitude_estimator.q_body_eci", Serializer<lin::Vector4f>()),
      attitude_estimator_p_body_eci_sigma_f("attitude_estimator.p_body_eci_sigma_f", Serializer<lin::Vector3f>(0.0, 0.5, 14)),
      attitude_estimator_w_body_f("attitude_estimator.w_body"),
      attitude_estimator_w_bias_body_f("attitude_estimator.w_bias_body", Serializer<lin::Vector3f>(0.0, 0.35, 14)),
      attitude_estimator_w_bias_body_sigma_f("attitude_estimator.w_bias_sigma_body", Serializer<lin::Vector3f>(0.0, 0.1, 14)),
      attitude_estimator_L_body_f("attitude_estimator.L_body", Serializer<lin::Vector3f>(0.0, 0.1, 14)),
      attitude_estimator_reset_cmd_f("attitude_estimator.reset_cmd", Serializer<bool>()),
      attitude_estimator_mag_flag_f("attitude_estimator.mag_flag", Serializer<bool>(), 1),
      attitude_estimator_ignore_sun_vectors_f("attitude_estimator.ignore_sun_vectors", Serializer<bool>(), 1),
      attitude_estimator_reset_persistance("attitude_estimator.reset_persistance", Serializer<unsigned int>(1000)),
      attitude_estimator_safety("attitude_estimator.safety", Serializer<unsigned int>(1000000)),
      attitude_estimator_fault("attitude_estimator.fault", ATTITUDE_ESTIMATOR_FAULT_PERSISTANCE)
{
    add_readable_field(attitude_estimator_b_valid_f);
    add_readable_field(attitude_estimator_b_body_f);
    add_readable_field(attitude_estimator_valid_f);
    add_readable_field(attitude_estimator_q_body_eci_f);
    add_readable_field(attitude_estimator_p_body_eci_sigma_f);
    add_internal_field(attitude_estimator_w_body_f);
    add_readable_field(attitude_estimator_w_bias_body_f);
    add_readable_field(attitude_estimator_w_bias_body_sigma_f);
    add_readable_field(attitude_estimator_L_body_f);
    add_writable_field(attitude_estimator_reset_cmd_f);
    add_writable_field(attitude_estimator_mag_flag_f);
    add_writable_field(attitude_estimator_ignore_sun_vectors_f);
    add_writable_field(attitude_estimator_reset_persistance);
    add_writable_field(attitude_estimator_safety);
    add_fault(attitude_estimator_fault);

    attitude_estimator_valid_f.set(false);
    attitude_estimator_b_body_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_q_body_eci_f.set(lin::zeros<lin::Vector4f>());
    attitude_estimator_p_body_eci_sigma_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_w_body_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_w_bias_body_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_w_bias_body_sigma_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_L_body_f.set(lin::zeros<lin::Vector3f>());
    attitude_estimator_reset_cmd_f.set(false);
    attitude_estimator_mag_flag_f.set(false);           // Prefer magnetometer two
    attitude_estimator_ignore_sun_vectors_f.set(false); // Overwritten by EEPROM
    attitude_estimator_reset_persistance.set(0);
    attitude_estimator_safety.set(10000);


    _state = gnc::AttitudeEstimatorState();
    _data = gnc::AttitudeEstimatorData();
    _estimate = gnc::AttitudeEstimate();
    _cycle_slip_mtr_cmd = lin::zeros<lin::Vector3f>();
}

void AttitudeEstimator::init()
{
    adcs_cmd_mtr_cmd = FIND_WRITABLE_FIELD(f_vector_t, adcs_cmd.mtr_cmd);
}

void AttitudeEstimator::execute()
{
    _execute();

    /* Cycle slip the MTR command.
     */
    auto const mtr_cmd = adcs_cmd_mtr_cmd->get();
    _cycle_slip_mtr_cmd = {mtr_cmd[0], mtr_cmd[1], mtr_cmd[2]};
}

#ifdef FLIGHT
#if defined(PAN_LEADER)
static constexpr lin::Matrix3x3f D1 = 1.0e-03 * lin::Matrix3x3f {
   0.280962247255082,  -0.491683932696393,  -0.401720651555603,
  -0.721090299605407,   0.441166090209642,   0.305529143258913,
   0.721090299605407,  -0.441166090209642,  -0.305529143258913
};
static constexpr lin::Matrix3x3f D2 = {
  -0.000141173148867,  -0.000031487148399,   0.001398583009120,
  -0.000002076075719,   0.000050863855107,  -0.001078521335830,
   0.000141173148867,   0.000027681009582,  -0.001398583009120
};
static constexpr lin::Vector3f c = 1.0e-04 * lin::Vector3f {
   -0.5108,    0.1698,   -0.3657
};
#elif defined(PAN_FOLLOWER)
static constexpr lin::Matrix3x3f D1 = 1.0e-03 * lin::Matrix3x3f {
   0.279578196775993,  -0.504832412247739,  -0.411409004909227,
  -0.732162703438119,   0.437359951392147,   0.294802752045973,
   0.733546753917209,  -0.434591850433969,  -0.293072688947111
};
static constexpr lin::Matrix3x3f D2 = {
  -0.000144633275065,  -0.000035293287217,   0.001440796548732,
  -0.000002422088338,   0.000050517842487,  -0.001114852660906,
   0.000144633275065,   0.000035293287217,  -0.001440796548732
};
static constexpr lin::Vector3f c = 1.0e-04 * lin::Vector3f {
    0.2716,    0.4158,   -0.6230
};
#else
static_assert(false, "Must define PAN_LEADER or PAN_FOLLOWER");
#endif
#endif

void AttitudeEstimator::_execute()
{
    /* Handle the processing of magnetometer information.
     */
    {
        auto const mag1_functional = adcs_mag1_functional_fp->get();
        auto const mag2_functional = adcs_mag2_functional_fp->get();
        auto const have_functional_magnetometer =
                mag1_functional || mag2_functional;

        if (have_functional_magnetometer)
        {
#ifdef FLIGHT  // These offsets aren't modelled so only do this in flight
            lin::Vector3f const mag1 = adcs_mag1_fp->get() - D1 * _cycle_slip_mtr_cmd - c;
            lin::Vector3f const mag2 = adcs_mag2_fp->get() - D2 * _cycle_slip_mtr_cmd;
#else
            lin::Vector3f const mag1 = adcs_mag1_fp->get();
            lin::Vector3f const mag2 = adcs_mag2_fp->get();
#endif

            if (attitude_estimator_mag_flag_f.get())
            {
                attitude_estimator_b_body_f.set(mag1_functional ? mag1 : mag2);
            }
            else
            {
                attitude_estimator_b_body_f.set(mag2_functional ? mag2 : mag1);
            }
        }

        attitude_estimator_b_valid_f.set(have_functional_magnetometer);
    }

    /* Reset the attitude estimator if the time estimate is invalid, orbit
     * estimate is invalid, or we don't have enough data from the ADCS box to
     * update the estimator.
     */
    {
        auto const should_reset = !time_valid_fp->get() ||
                !orbit_valid_fp->get() || !adcs_gyr_functional_fp->get() ||
                !attitude_estimator_b_valid_f.get() ||
                attitude_estimator_reset_cmd_f.get();
        if (should_reset)
        {
            attitude_estimator_valid_f.set(false);
            attitude_estimator_reset_cmd_f.set(false);
            attitude_estimator_fault.evaluate(true);

            _state = gnc::AttitudeEstimatorState();
            _estimate = gnc::AttitudeEstimate();

            return;
        }
    }

    auto const time_s = time_s_fp->get();
    auto const orbit_pos = orbit_pos_fp->get();
    auto const adcs_gyr = adcs_gyr_fp->get();
    auto const adcs_ssa_valid = adcs_ssa_mode_fp->get() == adcs::SSA_COMPLETE;
    auto const ignore_sun_vectors = attitude_estimator_ignore_sun_vectors_f.get();
    auto const adcs_ssa = [&]() -> lin::Vector3f {
        if (!adcs_ssa_valid || ignore_sun_vectors)
        {
            return lin::nans<lin::Vector3f>();
        }
        auto const s = adcs_ssa_fp->get();
        return s / lin::norm(s);
    }();
    auto const b_body = attitude_estimator_b_body_f.get();

    /* The Current Frobenius Norm */

    float fro_norm= lin::fro(_estimate.P);

    /* Make sure to set the attitude_estimator_reset_persistance to false check this, this is the logic
    that triggers the state field to allow for a reset of attitude_estimator. Once you exceed baseline frobenius norm
    * 1000 (for the safety factor), trigger the persistance state field, accounting for SSA valid and invalid. */
    
    
    if (fro_norm > 3.86e-7 * (attitude_estimator_safety.get()) && !adcs_ssa_valid) {
        attitude_estimator_reset_persistance.set(attitude_estimator_reset_persistance.get() + 1);
    }

    else if (fro_norm > 8.35e-10 * (attitude_estimator_safety.get()) && adcs_ssa_valid) {
        attitude_estimator_reset_persistance.set(attitude_estimator_reset_persistance.get() + 1);
    }

    else {
        attitude_estimator_reset_persistance.set(0);
    }

    auto exceed_persistance = false;

    // This should be based off persistance

    if (attitude_estimator_reset_persistance.get() == 30) {
        exceed_persistance = true;
    }

    // One if statement for no ssa covariance and one if statement for ssa valid covariance 

    if (!adcs_ssa_valid && exceed_persistance) {
        printf(debug_severity::error, "Our code resets: %d\n", static_cast<unsigned int>(0));
        gnc::attitude_estimator_reset(_state, time_s, {0.0f, 0.0f, 0.0f, 1.0f});

    }
    else if (adcs_ssa_valid && exceed_persistance) {
        printf(debug_severity::error, "Our code resets: %d\n", static_cast<unsigned int>(0));
        gnc::attitude_estimator_reset(_state, time_s, orbit_pos, b_body, adcs_ssa);
    }

    /* Attempt to update or initialize the attitude estimator.
     */
    if (_state.is_valid)
    {
        _data = gnc::AttitudeEstimatorData();
        _data.t = time_s;
        _data.r_ecef = orbit_pos;
        _data.b_body = b_body;
        _data.s_body = adcs_ssa;
        _data.w_body = adcs_gyr;

        gnc::attitude_estimator_update(_state, _data, _estimate);
    }
    else
    {
        if (ignore_sun_vectors) 
        {
            gnc::attitude_estimator_reset(
                    _state, time_s, {0.0f, 0.0f, 0.0f, 1.0f});
        }
        else if (adcs_ssa_valid)
        {
            gnc::attitude_estimator_reset(
                    _state, time_s, orbit_pos, b_body, adcs_ssa);
        }
    }

    /* Populate outputs of the attitude estimator if valid.
     */
    attitude_estimator_valid_f.set(_estimate.is_valid);
    attitude_estimator_fault.evaluate(!_estimate.is_valid);
    if (_estimate.is_valid)
    {
        attitude_estimator_q_body_eci_f.set(_estimate.q_body_eci);
        attitude_estimator_p_body_eci_sigma_f.set(
                lin::sqrt(lin::ref<lin::Vector3f>(lin::diag(_estimate.P), 0, 0)));
        attitude_estimator_w_body_f.set(adcs_gyr - _estimate.gyro_bias);
        attitude_estimator_w_bias_body_f.set(_estimate.gyro_bias);
        attitude_estimator_w_bias_body_sigma_f.set(
                lin::sqrt(lin::ref<lin::Vector3f>(lin::diag(_estimate.P), 3, 0)));
        attitude_estimator_L_body_f.set(
                gnc::constant::J_sat * attitude_estimator_w_body_f.get());
    }
}
