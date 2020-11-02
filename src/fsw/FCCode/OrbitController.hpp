#pragma once

#include <algorithm>

#include <common/GPSTime.hpp>
#include <common/constant_tracker.hpp>
#include <environment.hpp>
#include <gnc/constants.hpp>
#include <gnc/orbit_controller.hpp>
#include "ClockManager.hpp"
#include "PropController.hpp"
#include "psim/truth/transform_direction.hpp"

#include "TimedControlTask.hpp"

class OrbitController : public TimedControlTask<void>
{
public:
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
     * Returns the time in seconds when the satellitee reaches its next firing point
     * This is just an estimation for the sake of prop planning. It's not a very accurate
     * number.
     */
    double time_till_node(double theta, lin::Vector3d pos, lin::Vector3d vel);

    /**
     * Calculate impulse
     */
    lin::Vector3d calculate_impulse(double t, lin::Vector3d r, lin::Vector3d v, lin::Vector3d dr, lin::Vector3d dv);

    /**
     * Schedule valves
     */
    void schedule_valves(lin::Vector3d J_ecef);

    // Firing nodes
    double pi = gnc::constant::pi;
    double firing_nodes[3] = {pi/3, pi, -pi/3};

    // Prop Controller
    PropController prop_controller;

    // Input statefields for time, position, velocity, and baseline
    // position/velocity. In ECEF
    const ReadableStateField<double>* const time_fp;
    const ReadableStateField<lin::Vector3d>* const pos_fp;
    const ReadableStateField<lin::Vector3d>* const vel_fp;
    const ReadableStateField<lin::Vector3d>* const baseline_pos_fp;
    const ReadableStateField<lin::Vector3d>* const baseline_vel_fp;

    // Converts from ECI to body frame
    ReadableStateField<lin::Vector4f>* q_body_eci_fp;

    // Outputs
    ReadableStateField<unsigned char>* prop_planner_state_fp;
    WritableStateField<unsigned int>* prop_cycles_until_firing_fp;
    WritableStateField<unsigned int> sched_valve1_f;
    WritableStateField<unsigned int> sched_valve2_f;
    WritableStateField<unsigned int> sched_valve3_f;
    WritableStateField<unsigned int> sched_valve4_f;

protected:
    // TODO: Add GNC components.
};
