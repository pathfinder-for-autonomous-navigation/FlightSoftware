#pragma once

#include <algorithm>

#include <common/GPSTime.hpp>
#include <common/constant_tracker.hpp>
#include <environment.hpp>
#include <gnc/constants.hpp>
#include <gnc/orbit_controller.hpp>

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
     * Returns the angle between the current position and the next firing point
     */
    double time_till_node(double theta, lin::Vector3d pos, lin::Vector3d vel);

    // Firing nodes
    double pi = gnc::constant::pi;
    double firing_nodes[3] = {pi/3, pi/2, -pi/3};

    // Input statefields for time, position, velocity, and baseline
    // position/velocity. In ECEF
    const ReadableStateField<double>* const time_fp;
    const ReadableStateField<lin::Vector3d>* const pos_fp;
    const ReadableStateField<lin::Vector3d>* const vel_fp;
    const ReadableStateField<lin::Vector3d>* const baseline_pos_fp;
    const ReadableStateField<lin::Vector3d>* const baseline_vel_fp;

    // Outputs
    ReadableStateField<unsigned char>* prop_planner_state_fp;
    WritableStateField<unsigned int> sched_valve1_f;
    WritableStateField<unsigned int> sched_valve2_f;
    WritableStateField<unsigned int> sched_valve3_f;
    WritableStateField<unsigned int> sched_valve4_f;

protected:
    // TODO: Add GNC components.
};
