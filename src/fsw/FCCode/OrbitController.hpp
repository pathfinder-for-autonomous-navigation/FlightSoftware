#pragma once

#include <common/GPSTime.hpp>
#include <common/constant_tracker.hpp>
#include <gnc/environment.hpp>
#include <gnc/utilities.hpp>
#include <gnc/constants.hpp>
#include <gnc/orbit_controller.hpp>
#include "TimedControlTask.hpp"
#include <fsw/FCCode/prop_state_t.enum>
#include <common/constant_tracker.hpp>

class OrbitController : public TimedControlTask<void>
{
public:
    // Linear regression on dataset from empirical test: ThrustVectorTest_2018_Nov_11_15h37m36s_300Firings_293K_refVac
    TRACKED_CONSTANT_SC(double, valve_time_lin_reg_slope, 0.024119);
    TRACKED_CONSTANT_SC(double, valve_time_lin_reg_intercept, 7.0092e-05);

    OrbitController(StateFieldRegistry &registry, unsigned int offset);

    /**
     * @brief Collect prop planner state's state field.
     */
    void init();

    /** 
    * Compute firing and set prop planner's desired state. 
    */
    void execute() override;

    /**
     * Returns the time in seconds when the satellite reaches its next firing point
     * This is just an estimation for the sake of prop planning. It's not a very accurate
     * number. It calculates the angular velocity w and uses the eqn firing_node = wt+theta
     * to find the time until each firing node. It then returns the smallest positive time.
     */
    double time_till_node(double theta, const lin::Vector3d &pos, const lin::Vector3d &vel);

    /**
     * Calculate impulse needs to rendezvous with leader. Uses a PD controller to return
     * impulse in ECEF reference frame.
     */
    lin::Vector3d calculate_impulse(double t, const lin::Vector3d &r, const lin::Vector3d &v, 
        const lin::Vector3d &dr, const lin::Vector3d &dv);

    /*
     * Convert the impulse of a thruster to the time the valve should be open in ms
     */
    unsigned int impulse_to_time(double impulse);

    /**
     * Calculates the time each valve should open to deliver a given impulse. 
     * The impulse must be in the body frame of the satellite
     */
    void schedule_valves(lin::Vector3d J_body);

    // Firing nodes
    double pi = gnc::constant::pi;
    double firing_nodes[3] = {pi/3, pi, -pi/3};

    // Orbit Controller
    gnc::OrbitControllerData data;
    gnc::OrbitControllerState state;
    gnc::OrbitActuation actuation;

    // Prop system 
    unsigned int prop_min_cycles_needed();
    WritableStateField<unsigned int>* prop_state_fp;
    WritableStateField<unsigned int>* max_pressurizing_cycles_fp;
    WritableStateField<unsigned int>* ctrl_cycles_per_filling_period_fp;
    WritableStateField<unsigned int>* ctrl_cycles_per_cooling_period_fp;

    // Input statefields for time, position, velocity, and baseline
    // position/velocity in ECEF
    const InternalStateField<double>* const time_fp;
    const ReadableStateField<bool> *const time_valid_fp;
    const ReadableStateField<bool> *const orbit_valid_fp;
    const ReadableStateField<unsigned char> *const rel_orbit_valid_fp;
    const ReadableStateField<lin::Vector3d>* const pos_fp;
    const ReadableStateField<lin::Vector3d>* const vel_fp;
    const ReadableStateField<lin::Vector3d>* const baseline_pos_fp;
    const ReadableStateField<lin::Vector3d>* const baseline_vel_fp;

    // Converts from ECI to body frame
    ReadableStateField<bool> const *const attitude_estimator_valid_fp;
    ReadableStateField<lin::Vector4f> const *const q_body_eci_fp;

    // Outputs
    WritableStateField<unsigned int>* prop_cycles_until_firing_fp;
    WritableStateField<unsigned int> sched_valve1_f;
    WritableStateField<unsigned int> sched_valve2_f;
    WritableStateField<unsigned int> sched_valve3_f;
    WritableStateField<unsigned int> sched_valve4_f;

};
