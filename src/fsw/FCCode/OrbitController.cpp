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
    prop_planner_state_fp = FIND_READABLE_FIELD(unsigned char, prop.planner.state); // isn't used by prop controller... might have to delete
    prop_cycles_until_firing_fp = FIND_WRITABLE_FIELD(unsigned int, prop.cycles_until_firing);
}

void OrbitController::execute() {

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

    // If the satellite is within a certain delta time/cc from the next firing point, then the 
    // propulsion system should get ready to fire soon.
    double delta_time = prop_controller.min_cycles_needed() + 10;

    // Get the time until the satellite reaches the next firing node in control cycles
    double time_till_firing = time_till_node(theta, r, v);
    double time_till_firing_cc = time_till_firing / ClockManager::control_cycle_size;

    // If the satellite is about to approach a firing point and the prop state is idle, 
    // then schedule the valves for firing
    // How do I access prop state? What I did here prob isn't right
    if (time_till_firing_cc <= delta_time && prop_controller.check_current_state(prop_state_t::idle)) {
        prop_cycles_until_firing_fp->set(time_till_firing_cc);
    }

    // Check if the satellite is at a firing point
    if ( std::find(firing_nodes.begin(), firing_nodes.end(), theta) != firing_nodes.end() ) {

        // Collect the output of the PD controller
        lin::Vector3d J_ecef = calculate_impulse(t, r, v, baseline_pos_fp->get(), baseline_vel_fp->get());

        // Communicate desired impulse to the prop controller. Need to talk to kyle and/or tanishq about this and replace the 0s
        sched_valve1_f.set(0);
        sched_valve2_f.set(0);
        sched_valve3_f.set(0);
        sched_valve4_f.set(0);

    }

}

double OrbitController::time_till_node(double theta, lin::Vector3d pos, lin::Vector3d vel) {
    // Calculate angular velocity (w = v/r)
    lin::Vector3d ang_vel = lin::norm(vel)/lin::norm(pos);

    // Calculate the times until each node (theta_node = theta_now + w*t)
    double min_time = std::numeric_limits<double>::max()
    for (int i=0; i < firing_nodes.size(); i++) {
        double time_til_node = (firing_nodes[i] - theta) / ang_vel;
        if (time_til_node > 0 && time_til_node < min_time) {
            min_time = time_til_node;
        }
    }

    // Return the smallest positive time
    return min_time;
}

lin::Vector3d OrbitController::calculate_impulse(double t, lin::Vector3d r, lin::Vector3d v, lin::Vector3d dr, lin::Vector3d dv) {
    // Assemble the input Orbit Controller data struct
    gnc::OrbitControllerData data;
    data.t = time_fp->get();
    data.r_ecef = pos_fp->get();
    data.v_ecef = vel_fp->get();
    data.dr_ecef = baseline_pos_fp->get();
    data.dv_ecef = baseline_vel_fp->get();

    // Default the state struct (a calculation buffer) and actuation struct (output)
    gnc::OrbitControllerState state;
    gnc::OrbitActuation actuation;

    gnc::control_orbit(&state, &data, &actuation);

    // Collect the output of the PD controller
    return actuation.J_ecef;
}