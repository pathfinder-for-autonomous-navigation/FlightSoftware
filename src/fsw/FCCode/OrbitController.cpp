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
    double theta = lin::atan2( lin::cross(proj_sun, r), lin::dot(proj_sun, r) );

    // If the satellite is within a certain time from the next firing point, then the 
    // propulsion system should get ready to fire soon.
    double delta_time = 5 * 60; // 5 minutes for now. Need to talk to Athena.

    if (time_till_node(theta, r, v) <= delta_time) {
        // Somehow tell the prop system to get ready - talk to Athena
    }

    // Check if the staellite is at a firing point
    if ( std::find(firing_nodes.begin(), firing_nodes.end(), theta) != firing_nodes.end() ) {
        // Assemble the input Orbit Controller data struct
        gnc::OrbitControllerData data;
        data.t = t;
        data.r_ecef = r;
        data.v_ecef = v;
        data.dr_ecef = baseline_pos_fp->get(); // Need to check if this is right...
        data.dv_ecef = baseline_vel_fp->get();

        // Default the state struct (a calculation buffer) and actuation struct (output)
        gnc::OrbitControllerState state;
        gnc::OrbitActuation actuation;

        gnc::control_orbit(&state, &data, &actuation);

        // Collect the output of the PD controller
        lin::Vector3d J_ecef = actuation.J_ecef;

        // Communicate desired impulse to the prop controller - talk to athena
    }

}

double OrbitController::time_till_node(double theta, lin::Vector3d pos, lin::Vector3d vel) {
    // Calculate angular velocity (w = v/r)
    lin::Vector3d ang_vel = lin::norm(vel)/lin::norm(pos);

    // Calculate the times until each node (theta_node = theta_now + w*t)
    double t1 = ( pi()/3 - theta) / ang_vel;
    double t2 = ( pi()/2 - theta) / ang_vel;
    double t3 = ( -pi()/3 - theta) / ang_vel;
    double times[3] = {t1, t2, t3};

    // Return the shortest positive time
    double min_time = 100*pi();
    for (int i=0; i<times.size(); i++){
        double time = times[i];
        if (time > 0 && time < min_time){
            min_time = time;
        }
    }

    return min_time;
}
