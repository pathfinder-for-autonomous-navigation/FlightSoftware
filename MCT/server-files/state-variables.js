//This file holds all of the variables to initialize every point on the gomspace battery and is called by other files that require variable access

module.exports= {
    gomspace: {
		batt_threshold: 8934,
		low_batt: {
			base: true,
			override: true,
			persistence: true,
			suppress: true,
			unsignal: false
		},
		get_hk: {
			base: true,
			override: false,
			persistence: false,
			suppress: false,
			unsignal: false
		},
		vbatt: 6600,
		cursun: 7,
		cursys: 5,
		vboost: {
			output1: 48,
			output2: 10,
			output3: 9
		},
		curin: {
			output1: 3,
			output2: 5,
			output3: 1
		},
		curout: {
			output1: 1,
			output2: 2,
			output3: 3,
			output4: 4,
			output5: 5,
			output6: 69
		},
		output: {
			output1: true,
			output2: false,
			output3: true,
			output4: true,
			output5: true,
			output6: true
		},
		counter_boot: 24,
		temp: {
			output1: 32,
			output2: 2,
			output3: 31,
			output4: 32
		},
		bootcause: 3,
		battmode: 0,
		pptmode: 1,
		power_cycle_output1_cmd: true,
		power_cycle_output2_cmd: true,
		power_cycle_output3_cmd: true,
		power_cycle_output4_cmd: true,
		power_cycle_output5_cmd: true,
		power_cycle_output6_cmd: true,
		heater: 2,
		heater_cmd: true,
		wdt_i2c_time_left: 3,
		counter_wdt_i2c: 1,
		gs_reboot_cmd: true,
		gs_reset_cmd: false,
		pv1_cmd: 2,
		pv2_cmd: 2,
		pv3_cmd: 2,
		pptmode_cmd: 1,
		counter_reset_cmd: false
	}
}
