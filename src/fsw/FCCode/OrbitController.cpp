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
}

void OrbitController::execute() {

    // Collect time, position, velocity, and sun vector data
    double t = time_fp->get();
    lin::Vector3d r = pos_fp->get();
    lin::Vector3d v = vel_fp->get();
    lin::Vector3d dr = baseline_pos_fp->get();
    lin::Vector3d dv = baseline_vel_fp->get();
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

    // Schedule the valves for firing soon
    if (time_till_firing_cc <= delta_time && prop_cycles_until_firing_fp->get() == 0) {
        prop_cycles_until_firing_fp->set(time_till_firing_cc);
    }

    // Check if the satellite is at a firing point
    if ( std::find(firing_nodes.begin(), firing_nodes.end(), theta) != firing_nodes.end() ) {

        // Collect the output of the PD controller and get the needed impulse
        lin::Vector3d J_ecef = calculate_impulse(t, r, v, dr, dv);

        // Communicate desired impulse to the prop controller.
        schedule_valves(J_ecef, t);

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

unsigned int impulse_to_time(double impulse) {
    double time = 0.024119 * impulse + 7.0092e-05;
    return static_cast<unsigned int>(time);
}

void schedule_valves(lin::Vector3d J_ecef, double t) {
    // Transform the impulse from ecef frame to the eci frame
    lin::Vector4d q_ecef_eci;
    gnc::env::earth_attitude(t, q_ecef_eci);
    lin::Vector4d q_eci_ecef;
    gnc::utl::quat_conj(q_ecef_eci, q_eci_ecef);

    lin::Vector3d J_eci;
    gnc::utl::rotate_frame(q_eci_ecef, J_ecef, J_eci);

    // Transform the impulse from eci frame to the body frame of the spacecraft
    lin::Vector4f q_body_eci = q_body_eci_fp->get();
    lin::Vector4f q_eci_body;
    gnc::utl::quat_conj(q_ecef_eci, q_eci_ecef);

    lin::Vector3d J_body;
    gnc::utl::rotate_frame(q_eci_body, J_eci, J_body);

    // Define the unit vectors that give the directions the prop system would fire in. 
    lin::Vector3d thruster1 = { 0.6534, -0.3822, -0.6534};
    lin::Vector3d thruster2 = { 0.5391,  0.6472,  0.5391};
    lin::Vector3d thruster3 = {-0.6534, -0.3822,  0.6534};
    lin::Vector3d thruster4 = {-0.5391,  0.6472, -0.5391};

    // Calculate a linear combination of these direction vectors to get the impulse on each thruster
    lin::Matrix3x4d thrust_matrix = {
        thruster1(0), thruster2(0), thruster3(0), thruster4(0),
        thruster1(1), thruster2(1), thruster3(1), thruster4(1),
        thruster1(2), thruster2(2), thruster3(2), thruster4(2),
    };

    // Solve the linear system (thrust_matrix)*x=(impulse vector). The general solution will have one degree of freedom,
    // so we must also minimimize the norm of x.
    // See top of page 4: https://faculty.math.illinois.edu/~mlavrov/docs/484-spring-2019/ch4lec4.pdf
    
    // Solve A * A^T * w = Y
    lin::Matrix3x3d AAT = thrust_matrix * lin::transpose(thrust_matrix);
    lin::Matrix3x3d Q, R;
    lin::qr(AAT, Q, R);
    lin::Vector3d W;
    lin::backward_sub(R, W, (lin::transpose(Q) * J_body).eval());

    // Solve x = A^T * w
    lin::Vector4d x = lin::transpose(thrust_matrix) * W;

    // Translate the impulse values into the times the valves must stay open
    unsigned int time1 = impulse_to_time(x(0));
    unsigned int time2 = impulse_to_time(x(1));
    unsigned int time3 = impulse_to_time(x(2));
    unsigned int time4 = impulse_to_time(x(3));

    // Set valves
    sched_valve1_f(time1);
    sched_valve2_f(time2);
    sched_valve3_f(time3);
    sched_valve4_f(time4);
}