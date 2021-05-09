#include "OrbitController.hpp"
#include <lin/generators.hpp>
#include <fsw/FCCode/Estimators/rel_orbit_state_t.enum>
#include <fsw/FCCode/Estimators/RelativeOrbitEstimator.hpp>

const constexpr double OrbitController::valve_time_lin_reg_slope;
const constexpr double OrbitController::valve_time_lin_reg_intercept;

// Firing nodes
constexpr double pi = gnc::constant::pi;
static constexpr std::array<double, 3> firing_nodes_far = {pi/3, pi, -pi/3};
static constexpr std::array<double, 18> firing_nodes_near = {pi/18, pi/6, pi*(5/18), pi*(7/18), pi/2, pi*(11/18), 
                                pi*(13/18), pi*(5/6), pi*(17/18), -pi*(17/18),
                                -pi*(5/6), -pi*(13/18), -pi*(11/18), -pi/2,
                                -pi*(7/18), -pi*(5/18), -pi/6, -pi/18};

static constexpr auto gain_factor = static_cast<double>(firing_nodes_near.size()) / firing_nodes_far.size();

OrbitController::OrbitController(StateFieldRegistry &r, unsigned int offset) : 
    TimedControlTask<void>(r, "orbit_control_ct", offset),
    time_fp(FIND_INTERNAL_FIELD(double, time.s)),
    time_valid_fp(FIND_READABLE_FIELD(bool, time.valid)),  
    orbit_valid_fp(FIND_READABLE_FIELD(bool, orbit.valid)),
    rel_orbit_valid_fp(FIND_READABLE_FIELD(unsigned char, rel_orbit.state)),
    pos_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.pos)),
    vel_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.vel)),
    baseline_pos_fp(FIND_READABLE_FIELD(lin::Vector3d, rel_orbit.rel_pos)),
    baseline_vel_fp(FIND_READABLE_FIELD(lin::Vector3d, rel_orbit.rel_vel)),
    attitude_estimator_valid_fp(FIND_READABLE_FIELD(bool, attitude_estimator.valid)),
    q_body_eci_fp(FIND_READABLE_FIELD(lin::Vector4f, attitude_estimator.q_body_eci)),
    sched_valve1_f("orbit.control.valve1", Serializer<unsigned int>(1000)),
    sched_valve2_f("orbit.control.valve2", Serializer<unsigned int>(1000)),
    sched_valve3_f("orbit.control.valve3", Serializer<unsigned int>(1000)),
    sched_valve4_f("orbit.control.valve4", Serializer<unsigned int>(1000)),
    J_ecef_f("orbit.control.J_ecef", Serializer<lin::Vector3d>(0,0.025,10)),
    alpha_f("orbit.control.alpha", Serializer<double>(0,1,10))

{
    add_writable_field(sched_valve1_f);
    add_writable_field(sched_valve2_f);
    add_writable_field(sched_valve3_f);
    add_writable_field(sched_valve4_f);
    add_writable_field(J_ecef_f);
    add_writable_field(alpha_f);
    sched_valve1_f.set(0);
    sched_valve2_f.set(0);
    sched_valve3_f.set(0);
    sched_valve4_f.set(0);
    J_ecef_f.set(lin::zeros<lin::Vector3d>());
    alpha_f.set(0.4);
    dr_smoothed = lin::nans<lin::Vector3d>();
    dv_smoothed = lin::nans<lin::Vector3d>();
}

void OrbitController::init() {
    prop_state_fp = FIND_WRITABLE_FIELD(unsigned int, prop.state);
    prop_cycles_until_firing_fp = FIND_WRITABLE_FIELD(unsigned int, prop.cycles_until_firing);
    max_pressurizing_cycles_fp = FIND_WRITABLE_FIELD(unsigned int, prop.max_pressurizing_cycles);
    ctrl_cycles_per_filling_period_fp = FIND_WRITABLE_FIELD(unsigned int, prop.ctrl_cycles_per_filling);
    ctrl_cycles_per_cooling_period_fp = FIND_WRITABLE_FIELD(unsigned int, prop.ctrl_cycles_per_cooling);
}

void OrbitController::execute() {

    // Applies exponential smoothing if relative orbit estimate is valid
    // Sets smoothed values to nans if relative orbit estimate is invalid
    double alpha = alpha_f.get();
    lin::Vector3d dr = baseline_pos_fp->get();
    lin::Vector3d dv = baseline_vel_fp->get();

    if (rel_orbit_valid_fp->get()) {
        if (!lin::all(lin::isfinite(dr_smoothed))) {
            dr_smoothed = dr;
        } else {
            dr_smoothed = alpha * dr + (1.0 - alpha) * dr_smoothed;
        }
        if (!lin::all(lin::isfinite(dv_smoothed))) {
            dv_smoothed = dv;
        } else {
            dv_smoothed = alpha * dv + (1.0 - alpha) * dv_smoothed;
        }
    } else {
        dr_smoothed = lin::nans<lin::Vector3d>();
        dv_smoothed = lin::nans<lin::Vector3d>();
    }

    // If we don't have all the information we need, don't calculate a firing
    if (!time_valid_fp->get() || !orbit_valid_fp->get() || !rel_orbit_valid_fp->get() ||
            !attitude_estimator_valid_fp->get()) {
        sched_valve1_f.set(0);
        sched_valve2_f.set(0);
        sched_valve3_f.set(0);
        sched_valve4_f.set(0);
        return;
    }

    // Collect time, position, velocity
    double t = time_fp->get();
    lin::Vector3d r = pos_fp->get();
    lin::Vector3d v = vel_fp->get();

    // Convert the velocity to ECEF0 coordinates
    lin::Vector3f w_earth;
    gnc::env::earth_angular_rate(t, w_earth);
    v = v - lin::cross(w_earth, r);

    // Get the sun vector in ECI and convert it to ECEF coordinate
    lin::Vector3d sun_eci;
    gnc::env::sun_vector(t, sun_eci);

    lin::Vector4d q_ecef_eci;
    gnc::env::earth_attitude(t, q_ecef_eci);

    lin::Vector3d sun_ecef;
    gnc::utl::rotate_frame(q_ecef_eci, sun_eci, sun_ecef);

    // Calculate the normal vector of the satellite's orbital plane
    lin::Vector3d orb_plane = lin::cross(r, v);
    orb_plane = orb_plane / lin::norm(orb_plane);

    // Project the sun vector onto the satellite's orbital plane
    lin::Vector3d proj_sun = sun_ecef - ( lin::dot(sun_ecef, orb_plane) * orb_plane );

    // Calculate the angle between the satellite's position and the projected sun vector
    double theta = lin::atan2( lin::norm(lin::cross(proj_sun, r)), lin::dot(proj_sun, r) );

    // If the satellite is within a certain delta time/cc from the next firing point, then the 
    // propulsion system should get ready to fire soon.

    // Get the time until the satellite reaches the next firing node in control cycles
    double time_till_firing = time_till_node(theta, r, v);
    double time_till_firing_cc = time_till_firing * 1000 / PAN::control_cycle_time_ms;

    // Schedule the valves for firing soon if the prop system is idle
    if (time_till_firing_cc <= (prop_min_cycles_needed() + 10) && static_cast<prop_state_t>(prop_state_fp->get()) == prop_state_t::idle) {
        prop_cycles_until_firing_fp->set(time_till_firing_cc);
    }

    // Check if the satellite is around a firing point and the prop system is ready to fire
    // and if the time and orbit data is valid
    if ( time_till_firing_cc < 20 && static_cast<prop_state_t>(prop_state_fp->get()) == prop_state_t::await_firing) {

        // Collect the output of the PD controller and get the needed impulse
        lin::Vector3d J_ecef = calculate_impulse(t, r, v, dr_smoothed, dv_smoothed);

        // Save J_ecef to statefield
        J_ecef_f.set(J_ecef);

        // Transform the impulse from ecef frame to the eci frame
        lin::Vector4d q_eci_ecef;
        gnc::utl::quat_conj(q_ecef_eci, q_eci_ecef);
        lin::Vector3d J_eci;
        gnc::utl::rotate_frame(q_eci_ecef, J_ecef, J_eci);

        // Transform the impulse from eci frame to the body frame of the spacecraft
        lin::Vector4f q_body_eci = q_body_eci_fp->get();
        lin::Vector3d J_body;
        gnc::utl::rotate_frame(lin::cast<double>(q_body_eci).eval(), J_eci, J_body);

        // Communicate desired impulse to the prop controller.
        schedule_valves(J_body);

    }

}

double OrbitController::time_till_node(double theta, const lin::Vector3d &pos, const lin::Vector3d &vel) {


    // Collect Relative Orbit Estimator State
    unsigned char rel_orbit_state=rel_orbit_valid_fp->get();

    // Calculate angular velocity (w = v/r)
    double ang_vel = lin::norm(vel)/lin::norm(pos);

    // Calculate the times until each node (theta_node = theta_now + w*t)
    auto next_node = [&](auto const &arr) -> double {
        auto min_time = std::numeric_limits<double>::max();
        for (auto const node : arr) {
            auto const time_til_node = (node - theta) / ang_vel;
            if (time_til_node > 0 && time_til_node < min_time) {
                min_time = time_til_node;
            }
        }
        return min_time;
    };

    return (rel_orbit_state == static_cast<unsigned char>(rel_orbit_state_t::estimating)) ? 
            next_node(firing_nodes_near) : next_node(firing_nodes_far);
}

lin::Vector3d OrbitController::calculate_impulse(double t, const lin::Vector3d &r, const lin::Vector3d &v, 
    const lin::Vector3d &dr, const lin::Vector3d &dv) {

    // Collects Relative Orbit Estimator state
    unsigned char rel_orbit_state=rel_orbit_valid_fp->get();

    // Assemble the input Orbit Controller data struct
    data.t = t;
    data.r_ecef = r;
    data.v_ecef = v;
    data.dr_ecef = dr;
    data.dv_ecef = dv;

    // Sets Orbit Controller gains depending on whether satellites are in near or far-field 
    data.p = gnc::constant::K_p;
    data.d = gnc::constant::K_d;
    data.energy_gain = gnc::constant::K_e;   // Energy gain                   (J)
    data.h_gain = gnc::constant::K_h;        // Angular momentum gain         (kg m^2/sec)
    
    if (rel_orbit_state==static_cast<unsigned char>(rel_orbit_state_t::estimating)) {
        data.p /= gain_factor;  
        data.d /= gain_factor;
        data.energy_gain /= gain_factor;
        data.h_gain /= gain_factor;
    }

    gnc::control_orbit(state, data, actuation);

    // Collect the output of the PD controller
    return actuation.J_ecef;

}

unsigned int OrbitController::impulse_to_time(double impulse) {
    double time = valve_time_lin_reg_slope * impulse + valve_time_lin_reg_intercept;
    int time_ms = time * 1000;
    return time_ms;
}

void OrbitController::schedule_valves(lin::Vector3d J_body) {

    double a = J_body(0);
    double b = J_body(1);
    double c = J_body(2);

    // Minimum norm solution of x4, not necessarily positive
    double x4 = -1 * (1/152280838494) * ((70618085000 * c)-(87226380000 * b)+(70618085000 * a)); 
    if (x4<0){
        x4=0;
    }

    // Calculate the impulses on each of the other thrusters
    double x1 = (0.4026550706*c) + (1.16788310856*a) - (1.30821559393*b) + (1.69335426478*x4);
    double x2 = (0.92747171211*c) + (0.92747171211*a) + x4;
    double x3 = (1.16788310856*c) + (0.4026550706*a) - (1.30821559393*b) + (1.69335426478*x4);

    // Check that none of the impulses are negative
    if (x1<0 || x2<0 || x3<0) {
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
    printf(debug_severity::error, "x1, x2 is: %f, %f\n", x1, x2);

    // Translate the impulse values into the times the valves must stay open and set valves
    sched_valve1_f.set(impulse_to_time(x1));
    sched_valve2_f.set(impulse_to_time(x2));
    sched_valve3_f.set(impulse_to_time(x3));
    sched_valve4_f.set(impulse_to_time(x4));

}

unsigned int OrbitController::prop_min_cycles_needed() {
    return max_pressurizing_cycles_fp->get() *
               (ctrl_cycles_per_filling_period_fp->get() +
                ctrl_cycles_per_cooling_period_fp->get()) +
           4;
}
