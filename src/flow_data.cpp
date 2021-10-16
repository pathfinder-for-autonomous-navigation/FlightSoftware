/**
 * WARNING: THIS FILE IS AUTOGENERATED. ALL CHANGES WILL BE OVERWRITTEN.
 */

#include "flow_data.hpp"

const std::vector<DownlinkProducer::FlowData> PAN::flow_data = {
{1, true, {"pan.state", "pan.deployed", "pan.sat_designation", "pan.bootcount", "pan.deployment.elapsed", "pan.cycle_no"}},
{2, true, {"time.valid", "orbit.valid", "attitude_estimator.ignore_sun_vectors", "time.gps", "orbit.pos", "orbit.vel", "attitude_estimator.valid", "attitude_estimator.q_body_eci", "attitude_estimator.L_body", "rel_orbit.state"}},
{3, false, {"orbit.pos_sigma", "orbit.vel_sigma"}},
{4, false, {"rel_orbit.pos", "rel_orbit.vel"}},
{5, false, {"rel_orbit.rel_pos", "rel_orbit.rel_vel", "rel_orbit.rel_pos_sigma", "rel_orbit.rel_vel_sigma"}},
{6, false, {"attitude_estimator.w_bias_body", "attitude_estimator.p_body_eci_sigma_f", "attitude_estimator.w_bias_sigma_body", "attitude_estimator.mag_flag", "attitude_estimator.b_valid", "attitude_estimator.b_body"}},
{7, true, {"adcs_monitor.functional_fault.base", "piksi_fh.dead.base", "adcs_monitor.wheel1_fault.base", "adcs_monitor.wheel2_fault.base", "adcs_monitor.wheel3_fault.base", "adcs_monitor.wheel_pot_fault.base", "prop.overpressured.base", "prop.pressurize_fail.base", "prop.tank2_temp_high.base", "prop.tank1_temp_high.base", "gomspace.low_batt.base", "gomspace.get_hk.base", "attitude_estimator.fault.base"}},
{8, true, {"gomspace.vbatt", "gomspace.cursun", "gomspace.cursys", "gomspace.temp.output1", "gomspace.temp.output2", "gomspace.temp.output3", "gomspace.temp.output4"}},
{9, false, {"gomspace.vboost.output1", "gomspace.vboost.output2", "gomspace.vboost.output3", "gomspace.curin.output1", "gomspace.curin.output2", "gomspace.curin.output3"}},
{10, false, {"gomspace.curout.output1", "gomspace.curout.output2", "gomspace.curout.output3", "gomspace.curout.output4", "gomspace.curout.output5", "gomspace.curout.output6", "gomspace.output.output1", "gomspace.output.output2", "gomspace.output.output3", "gomspace.output.output4", "gomspace.output.output5", "gomspace.output.output6"}},
{11, false, {"gomspace.counter_boot", "gomspace.bootcause", "gomspace.battmode", "gomspace.pptmode"}},
{12, false, {"gomspace.power_cycle_output1_cmd", "gomspace.power_cycle_output2_cmd", "gomspace.power_cycle_output3_cmd", "gomspace.power_cycle_output4_cmd", "gomspace.power_cycle_output5_cmd", "gomspace.power_cycle_output6_cmd", "gomspace.piksi_off"}},
{13, true, {"adcs.state", "prop.state", "radio.state", "piksi.state", "qfh.state"}},
{14, true, {"adcs_monitor.havt_device0", "adcs_monitor.havt_device1", "adcs_monitor.havt_device2", "adcs_monitor.havt_device3", "adcs_monitor.havt_device4", "adcs_monitor.havt_device5", "adcs_monitor.havt_device6", "adcs_monitor.havt_device7", "adcs_monitor.havt_device8", "adcs_monitor.havt_device9", "adcs_monitor.havt_device10", "adcs_monitor.havt_device11", "adcs_monitor.havt_device12", "adcs_monitor.havt_device13", "adcs_monitor.havt_device14", "adcs_monitor.havt_device15", "adcs_monitor.havt_device16", "adcs_monitor.havt_device17"}},
{15, true, {"adcs_monitor.rwa_speed_rd.x", "adcs_monitor.rwa_speed_rd.y", "adcs_monitor.rwa_speed_rd.z", "adcs_monitor.rwa_torque_rd.x", "adcs_monitor.rwa_torque_rd.y", "adcs_monitor.rwa_torque_rd.z", "adcs_monitor.ssa_mode"}},
{16, false, {"prop.tank1.valve_choice", "prop.tank2.pressure", "prop.tank2.temp", "prop.tank1.temp", "prop.num_prop_firings"}},
{17, true, {"attitude.pointer_vec1_current", "attitude.pointer_vec1_desired", "attitude.pointer_vec2_current", "attitude.pointer_vec2_desired"}},
{18, true, {"radio.err", "radio.last_comms_ccno"}},
{19, true, {"adcs_monitor.mag1_vec.x", "adcs_monitor.mag1_vec.y", "adcs_monitor.mag1_vec.z", "adcs_monitor.mag2_vec.x", "adcs_monitor.mag2_vec.y", "adcs_monitor.mag2_vec.z", "adcs_monitor.gyr_vec.x", "adcs_monitor.gyr_vec.y", "adcs_monitor.gyr_vec.z", "adcs_monitor.ssa_vec"}},
{20, false, {"adcs_monitor.speed_rd_flag", "adcs_monitor.torque_rd_flag", "adcs_monitor.mag1_vec_flag", "adcs_monitor.mag2_vec_flag", "adcs_monitor.gyr_vec_flag", "adcs_monitor.gyr_temp_flag"}},
{21, false, {"piksi.pos", "piksi.vel", "piksi.baseline_pos", "piksi.fix_error_count", "piksi.time", "piksi.microdelta"}},
{22, false, {"dcdc.SpikeDock", "dcdc.ADCSMotor"}},
{23, false, {"adcs_monitor.ssa_voltage0", "adcs_monitor.ssa_voltage1", "adcs_monitor.ssa_voltage2", "adcs_monitor.ssa_voltage3", "adcs_monitor.ssa_voltage4", "adcs_monitor.ssa_voltage5", "adcs_monitor.ssa_voltage6", "adcs_monitor.ssa_voltage7", "adcs_monitor.ssa_voltage8", "adcs_monitor.ssa_voltage9"}},
{24, false, {"adcs_monitor.ssa_voltage10", "adcs_monitor.ssa_voltage11", "adcs_monitor.ssa_voltage12", "adcs_monitor.ssa_voltage13", "adcs_monitor.ssa_voltage14", "adcs_monitor.ssa_voltage15", "adcs_monitor.ssa_voltage16", "adcs_monitor.ssa_voltage17", "adcs_monitor.ssa_voltage18", "adcs_monitor.ssa_voltage19"}},
{25, true, {"docksys.docked"}},
{26, true, {"orbit.control.valve1", "orbit.control.valve2", "orbit.control.valve3", "orbit.control.valve4", "orbit.control.J_ecef", "orbit.control.alpha", "orbit.control.num_near_field_nodes"}},
{27, false, {"prop.cycles_until_firing", "prop.sched_intertank1", "prop.sched_intertank2"}},
{28, false, {"adcs_cmd.rwa_speed_cmd", "adcs_cmd.rwa_torque_cmd", "adcs_cmd.mtr_cmd"}},
{29, false, {"timing.adcs_commander.avg_wait", "timing.adcs_commander.num_lates", "timing.adcs_controller.avg_wait", "timing.adcs_controller.num_lates", "timing.estimators.avg_wait", "timing.estimators.num_lates", "timing.adcs_monitor.avg_wait", "timing.adcs_monitor.num_lates", "timing.attitude_controller.avg_wait", "timing.attitude_controller.num_lates", "timing.clock_ct.avg_wait", "timing.clock_ct.num_lates", "timing.dcdc_ct.avg_wait", "timing.dcdc_ct.num_lates", "timing.debug.avg_wait", "timing.debug.num_lates"}},
{30, false, {"timing.docking_ct.avg_wait", "timing.docking_ct.num_lates", "timing.downlink_ct.avg_wait", "timing.downlink_ct.num_lates", "timing.eeprom_ct.avg_wait", "timing.eeprom_ct.num_lates", "timing.gomspace_rd.avg_wait", "timing.gomspace_rd.num_lates", "timing.mission_ct.avg_wait", "timing.mission_ct.num_lates", "timing.piksi.avg_wait", "timing.piksi.num_lates", "timing.quake.avg_wait", "timing.quake.num_lates", "timing.uplink_ct.avg_wait", "timing.uplink_ct.num_lates"}},
{31, false, {"dcdc.ADCSMotor_cmd", "dcdc.SpikeDock_cmd", "dcdc.disable_cmd", "dcdc.reset_cmd"}},
{32, false, {"adcs_cmd.rwa_mode", "adcs_cmd.rwa_speed_filter", "adcs_cmd.rwa_ramp_filter", "adcs_cmd.mtr_mode", "adcs_cmd.mtr_limit", "adcs_cmd.ssa_voltage_filter", "adcs_cmd.mag1_mode", "adcs_cmd.mag2_mode", "adcs_cmd.imu_mag_filter", "adcs_cmd.imu_gyr_filter", "adcs_cmd.imu_gyr_temp_filter", "adcs_cmd.imu_gyr_temp_pwm", "adcs_cmd.imu_gyr_temp_desired"}},
{33, false, {"detumble_safety_factor", "docking_timeout_limit", "fault_handler.enabled", "trigger_dist.close_approach", "trigger_dist.docking"}},
{34, false, {"downlink.shift_id1", "downlink.shift_id2", "downlink.toggle_id"}},
{35, false, {"gomspace.heater", "gomspace.heater_cmd"}},
{36, false, {"gomspace.wdt_i2c_time_left", "gomspace.counter_wdt_i2c"}},
{37, false, {"gomspace.gs_reboot_cmd", "gomspace.gs_reset_cmd", "gomspace.heater_cmd", "gomspace.pv1_cmd", "gomspace.pv2_cmd", "gomspace.pv3_cmd", "gomspace.pptmode_cmd", "gomspace.counter_reset_cmd"}},
{38, false, {"adcs_cmd.havt_disable0", "adcs_cmd.havt_disable1", "adcs_cmd.havt_disable2", "adcs_cmd.havt_disable3", "adcs_cmd.havt_disable4", "adcs_cmd.havt_disable5", "adcs_cmd.havt_disable6", "adcs_cmd.havt_disable7", "adcs_cmd.havt_disable8", "adcs_cmd.havt_disable9", "adcs_cmd.havt_disable10", "adcs_cmd.havt_disable11", "adcs_cmd.havt_disable12", "adcs_cmd.havt_disable13", "adcs_cmd.havt_disable14", "adcs_cmd.havt_disable15", "adcs_cmd.havt_disable16", "adcs_cmd.havt_disable17", "adcs_cmd.havt_disable18"}},
{39, false, {"adcs_cmd.havt_reset0", "adcs_cmd.havt_reset1", "adcs_cmd.havt_reset2", "adcs_cmd.havt_reset3", "adcs_cmd.havt_reset4", "adcs_cmd.havt_reset5", "adcs_cmd.havt_reset6", "adcs_cmd.havt_reset7", "adcs_cmd.havt_reset8", "adcs_cmd.havt_reset9", "adcs_cmd.havt_reset10", "adcs_cmd.havt_reset11", "adcs_cmd.havt_reset12", "adcs_cmd.havt_reset13", "adcs_cmd.havt_reset14", "adcs_cmd.havt_reset15", "adcs_cmd.havt_reset16", "adcs_cmd.havt_reset17", "adcs_cmd.havt_reset18"}},
{40, false, {"radio.max_transceive", "radio.max_wait"}},
{41, false, {"piksi_fh.no_cdpgs_max_wait", "piksi_fh.cdpgs_delay_max_wait", "piksi_fh.enabled"}},
{42, false, {"prop.max_venting_cycles", "prop.ctrl_cycles_per_closing", "prop.max_pressurizing_cycles", "prop.threshold_firing_pressure", "prop.ctrl_cycles_per_filling", "prop.ctrl_cycles_per_cooling"}},
{43, false, {"docksys.is_turning", "docksys.config_cmd", "docksys.step_angle", "docksys.step_delay", "docksys.dock_config"}}
};
