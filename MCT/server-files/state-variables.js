//This file holds all of the variables to initialize every point on the gomspace battery and is called by other files that require variable access

module.exports= {
  modes: {battmode: 0, heater: 2, pptmode: 1,},
  counters: {counter_boot: 24, counter_wdt_i2c: 1, wdt_i2c_time_left:3},
  commands: {counter_reset_cmd: false, gs_reboot_cmd:true, gs_reset_cmd:false, heater_cmd:true,pptmode_cmd:1},
  curin: {output1:3, output2: 5, output3: 1},
  curout: {output1:1, output2: 2, output3: 3, output4: 4, output5: 5, output6: 69},
  get_hk: {base: true, override:false, persistence: false, suppress: false, unsignal:false},
  low_batt: {base: true, override: true, persistence: true, suppress: true, unsignal: false},
  output: {output1: true, output2: false, output3: true, output4: true, output5: true, output6: true},
  power_cycle_output: {power_cycle_output1_cmd:true, power_cycle_output2_cmd:true, power_cycle_output3_cmd:true, power_cycle_output4_cmd:true, power_cycle_output5_cmd:true, power_cycle_output6_cmd:true},
  pv: {pv1_cmd:2, pv2_cmd:2, pv3_cmd:2},
  temp: {output1: 32, output2: 2, output3: 31, output4: 32 },
  vboost: {output1:48, output2:10, output3: 9  },
  vbatt: 6000,
  batt_threshold: 8934,
  cursun: 7,
  cursys: 5,
  bootcause: 3
}
