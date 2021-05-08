"""Utility functions specific to full mission cases.

Full mission cases here refers to those intended to be used for mission
rehearsals.
"""

fc_fields = [
    # Core state information
    "pan.state",
    "pan.bootcount",
    "pan.cycle_no",
    "pan.deployment.elapsed",
    # Other state machine states
    "adcs.state",
    "piksi.state",
    "radio.state",
    "prop.state",
    # Estimator information
    "time.valid",
    "time.gps",
    "orbit.valid",
    "orbit.pos",
    "orbit.vel",
    "rel_orbit.state",
    "rel_orbit.rel_pos",
    "rel_orbit.rel_vel",
    "attitude_estimator.valid",
    "attitude_estimator.q_body_eci",
    "attitude_estimator.L_body",
    "attitude_estimator.w_bias_body",
    # Attitude control commands
    "adcs_cmd.mtr_cmd",
    "adcs_cmd.rwa_torque_cmd",
    # PSim sensor debugging information
    #    "piksi.time"
    #    "adcs_monitor.ssa_vec",
    #    "adcs_monitor.mag1_vec",
    #    "adcs_monitor.gyr_vec",
    #    "adcs_monitor.ssa_mode",
    "dcdc.SpikeDock",

    "prop.cycles_until_firing",
    "prop.sched_valve1",
    "prop.sched_valve2",
    "prop.sched_valve3",
    "prop.sched_valve4",

    #prop errors
    "prop.pressurize_fail.base",
    "prop.overpressured.base",
    "prop.tank2_temp_high.base",
    "prop.tank1_temp_high.base"
]

psim_fields_per_satellite = [
    "truth.{}.attitude.w",
    "truth.{}.attitude.L",
    "truth.{}.wheels.t",
    "truth.{}.wheels.w",
    "truth.{}.orbit.r.ecef",
    "truth.{}.orbit.v.ecef",
    "sensors.{}.gyroscope.w.bias"
]

psim_fields = [
    "truth.t.ns",
    "truth.dt.ns"
]


def log_fc_data(fc):
    for field in fc_fields:
        fc.smart_read(field)


def log_psim_data(case, *satellites):
    """Logs a collection of PSim associated with a testcase for the satellite(s)
    specified.
    """
    for satellite in satellites:
        for field in psim_fields_per_satellite:
            case.rs_psim(field.format(satellite))

    for field in psim_fields:
        case.rs_psim(field)
