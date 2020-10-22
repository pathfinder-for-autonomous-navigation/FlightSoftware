#include "OrbitController.hpp"

OrbitController::OrbitController(StateFieldRegistry &r, unsigned int offset) : 
    TimedControlTask<void>(r, "orbit_control_ct", offset),
    time_fp(FIND_READABLE_FIELD(double, orbit.time)),
    pos_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.pos)),
    vel_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.vel)),
    baseline_pos_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.baseline_pos)),
    baseline_vel_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.baseline_vel)),
    sched_valve1_f("orbit.control.valve1", Serializer<unsigned int>(1000)),
    sched_valve2_f("orbit.control.valve2", Serializer<unsigned int>(1000)),
    sched_valve3_f("orbit.control.valve3", Serializer<unsigned int>(1000)),
    sched_valve4_f("orbit.control.valve4", Serializer<unsigned int>(1000))
{
    add_writable_field(sched_valve1_f);
    add_writable_field(sched_valve2_f);
    add_writable_field(sched_valve3_f);
    add_writable_field(sched_valve4_f);
    sched_valve1_f.set(0);
    sched_valve2_f.set(0);
    sched_valve3_f.set(0);
    sched_valve4_f.set(0);
}

void OrbitController::init() {
    prop_planner_state_fp = FIND_READABLE_FIELD(unsigned char, prop.planner.state);
}

void OrbitController::execute() {
    sched_valve1_f.set(0);
    sched_valve2_f.set(0);
    sched_valve3_f.set(0);
    sched_valve4_f.set(0);

    // Collect time, position, velocity, and sun vector data
    double t = time_fp->get();
    lin::Vector3d r = pos_fp->get();
    lin::Vector3d v = vel_fp->get();
    lin::Vector3d sun; // Points from the Earth to the sun
    sun_vector(t, sun);

    // Calculate the normal vector of the satellite's orbital plane
    lin::Vector3d orb_plane = lin::cross(r, v);
    orb_plane = orb_plane / lin::norm(orb_plane);

    // Project the sun vector onto the satellite's orbital plane
    lin::Vector3d proj_sun = sun - ( lin::dot(sun, orb_plane) * orb_plane );

    // Calculate the angle between the satellite's position and the projected sun vector
    double theta = lin::acos( lin::dot(r, proj_sun) / (lin::norm(r) * lin::norm(proj_sun)) );

    // The propulsion system should fire at 60, -60 and 180 degrees
    if ( std::abs(theta) == pi()/3 || theta == pi() ) {
        // Assemble the input Orbit Controller data struct
        gnc::OrbitControllerData data;

        // Default the state struct (a calculation buffer) and actuation struct (output)
        gnc::OrbitControllerState state;
        gnc::OrbitActuation actuation;

        gnc::control_orbit(&state, &data, &actuation);

        // Collect the output of the PD controller
        lin::Vector3d J_ecef = actuation.J_ecef;
    }

}
