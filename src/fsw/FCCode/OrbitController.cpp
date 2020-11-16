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
    prop_cycles_until_firing_fp = FIND_WRITABLE_FIELD(unsigned int, prop.cycles_until_firing);
    q_body_eci_fp = FIND_READABLE_FIELD(lin::Vector4f, attitude_estimator.q_body_eci);
    max_pressurizing_cycles_fp = FIND_WRITABLE_FIELD(unsigned int, prop.max_pressurizing_cycles);
    ctrl_cycles_per_filling_period_fp = FIND_WRITABLE_FIELD(unsigned int, prop.ctrl_cycles_per_filling);
    ctrl_cycles_per_cooling_period_fp = FIND_WRITABLE_FIELD(unsigned int, prop.ctrl_cycles_per_cooling);
}

void OrbitController::execute() {

    // Collect time, position, velocity, and sun vector data
    double t = time_fp->get();
    lin::Vector3d r = pos_fp->get();
    lin::Vector3d v = vel_fp->get();
    lin::Vector3d dr = baseline_pos_fp->get();
    lin::Vector3d dv = baseline_vel_fp->get();
    lin::Vector3d sun; // Points from the Earth to the sun
    gnc::env::sun_vector(t, sun);

    // Calculate the normal vector of the satellite's orbital plane
    lin::Vector3d orb_plane = lin::cross(r, v);
    orb_plane = orb_plane / lin::norm(orb_plane);

    // Project the sun vector onto the satellite's orbital plane
    lin::Vector3d proj_sun = sun - ( lin::dot(sun, orb_plane) * orb_plane );

    // Calculate the angle between the satellite's position and the projected sun vector
    double theta = lin::atan2( lin::norm(lin::cross(proj_sun, r)), lin::dot(proj_sun, r) );

    // If the satellite is within a certain delta time/cc from the next firing point, then the 
    // propulsion system should get ready to fire soon.
    double delta_time = prop_min_cycles_needed() + 10;

    // Get the time until the satellite reaches the next firing node in control cycles
    double time_till_firing = time_till_node(theta, r, v);
    double time_till_firing_cc = time_till_firing / PAN::control_cycle_time;

    // Schedule the valves for firing soon
    if (time_till_firing_cc <= delta_time && prop_cycles_until_firing_fp->get() == 0) {
        prop_cycles_until_firing_fp->set(time_till_firing_cc);
    }

    // Check if the satellite is at a firing point
    if ( std::find(std::begin(firing_nodes), std::end(firing_nodes), theta) != std::end(firing_nodes) ) {

        // Collect the output of the PD controller and get the needed impulse
        lin::Vector3d J_ecef = calculate_impulse(t, r, v, dr, dv);

        // Transform the impulse from ecef frame to the eci frame
        lin::Vector4d q_ecef_eci;
        gnc::env::earth_attitude(t, q_ecef_eci);
        lin::Vector4d q_eci_ecef;
        gnc::utl::quat_conj(q_ecef_eci, q_eci_ecef);

        lin::Vector3d J_eci;
        gnc::utl::rotate_frame(q_eci_ecef, J_ecef, J_eci);

        // Transform the impulse from eci frame to the body frame of the spacecraft
        lin::Vector4f q_body_eci = q_body_eci_fp->get();
        lin::Vector4d q_body_eci_d = { (double)q_body_eci(0), (double)q_body_eci(1), 
            (double)q_body_eci(2), (double)q_body_eci(3) }; // cast to double

        lin::Vector3d J_body;
        gnc::utl::rotate_frame(q_body_eci_d, J_eci, J_body);

        // Communicate desired impulse to the prop controller.
        schedule_valves(J_body);

    }

}

double OrbitController::time_till_node(double theta, lin::Vector3d pos, lin::Vector3d vel) {
    // Calculate angular velocity (w = v/r)
    double ang_vel = lin::norm(vel)/lin::norm(pos);

    // Calculate the times until each node (theta_node = theta_now + w*t)
    double min_time = std::numeric_limits<double>::max();
    for (size_t i=0; i < sizeof(firing_nodes); i++) {
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
    data.t = t;
    data.r_ecef = r;
    data.v_ecef = v;
    data.dr_ecef = dr;
    data.dv_ecef = dv;

    gnc::control_orbit(state, data, actuation);

    // Collect the output of the PD controller
    return actuation.J_ecef;
}

unsigned int OrbitController::impulse_to_time(double impulse) {
    double time = 0.024119 * impulse + 7.0092e-05;
    int time_ms = time * 1000;
    return time_ms;
}

void OrbitController::schedule_valves(lin::Vector3d J_body) {

    double a = J_body(0);
    double b = J_body(1);
    double c = J_body(2);

    // Minimum norm solution of x4, not necessarily positive
    double x4 = -1 * (1/152280838494) * ((70618085000 * c)-(87226380000 * b)+(70618085000 * a)); 

    // Calculate the impulses on each of the other thrusters
    double x1 = (0.4026550706*c) + (1.16788310856*a) - (1.30821559393*b) + (1.69335426478*x4);
    double x2 = (0.92747171211*c) + (0.92747171211*a) + x4;
    double x3 = (1.16788310856*c) + (0.4026550706*a) - (1.30821559393*b) + (1.69335426478*x4);

    // Check that none of the impulses are negative
    if (x1<0 || x2<0 || x3<0 || x4<0) {
        // Get the values of the particular solution
        double x1_p = (0.4026550706*c) + (1.16788310856*a) - (1.30821559393*b);
        double x2_p = (0.92747171211*c) + (0.92747171211*a);
        double x3_p = (1.16788310856*c) + (0.4026550706*a) - (1.30821559393*b); 
        
        // Find the next minimum value of x4 that would make x1,x2,x3>0
        x4 = -1 * std::min(x1_p, std::min(x2_p, x3_p));

        // Recalculate  x1,x2,x3
        x1 = (0.4026550706*c) + (1.16788310856*a) - (1.30821559393*b) + (1.69335426478*x4);
        x2 = (0.92747171211*c) + (0.92747171211*a) + x4;
        x3 = (1.16788310856*c) + (0.4026550706*a) - (1.30821559393*b) + (1.69335426478*x4);
    }

    // Translate the impulse values into the times the valves must stay open
    unsigned int time1 = impulse_to_time(x1);
    unsigned int time2 = impulse_to_time(x2);
    unsigned int time3 = impulse_to_time(x3);
    unsigned int time4 = impulse_to_time(x4);

    // Set valves
    sched_valve1_f.set(time1);
    sched_valve2_f.set(time2);
    sched_valve3_f.set(time3);
    sched_valve4_f.set(time4);
}

unsigned int OrbitController::prop_min_cycles_needed() {
    return max_pressurizing_cycles_fp->get() *
               (ctrl_cycles_per_filling_period_fp->get() +
                ctrl_cycles_per_cooling_period_fp->get()) +
           4;
}