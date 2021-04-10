# How to write a ptest/psim interface mappings config file

Please define a json like the one below.
You must provide two dictionaries.
"fc_vs_sim_s" is must be a dictionary between flight software statefield names and the
corresponding psim statefield names. Replace leader or follower with 'sat'.
Please similarly define "fc_vs_sim_a" which is a dictionary of the mappings for actuators.
    
"fc_vs_sim_s":{
    "orbit.pos": "truth.sat.orbit.r.ecef",
    "orbit.vel": "truth.sat.orbit.v.ecef",
    "adcs_monitor.ssa_vec":  "truth.sat.environment.s.body",
    "adcs_monitor.mag1_vec": "truth.sat.environment.b.body",
    "adcs_monitor.gyr_vec": "truth.sat.attitude.w",
    "adcs_monitor.rwa_speed_rd": "truth.sat.wheels.w"
},
"fc_vs_sim_a":{
    "adcs_cmd.mtr_cmd": "truth.sat.magnetorquers.m",
    "adcs_cmd.rwa_torque_cmd": "truth.sat.wheels.t"
}