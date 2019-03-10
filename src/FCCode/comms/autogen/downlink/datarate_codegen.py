FIELDS = []

ADCS_HISTORY_FIELDS = []
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "gyro_history",              })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "gyro_fast_history",         })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "attitude_cmd_history",      })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "attitude_history",          })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "attitude_fast_history",     })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "rate_history",              })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "rate_fast_history",         })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "spacecraft_L_history",      })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "spacecraft_L_fast_history", })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "magnetometer_history",      })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "rwa_ramp_cmd_history",      })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "mtr_cmd_history",           })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "ssa_vector_history",        })
FIELDS.extend(ADCS_HISTORY_FIELDS)

PIKSI_HISTORY_FIELDS = []
PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "recorded_position_history",       })
PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "recorded_velocity_history",       })
PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "recorded_position_other_history", })
FIELDS.extend(PIKSI_HISTORY_FIELDS)

PROPULSION_HISTORY_FIELDS = []
PROPULSION_HISTORY_FIELDS.append({ "group" : "propulsion_history", "name" : "tank_pressure_history",          })
PROPULSION_HISTORY_FIELDS.append({ "group" : "propulsion_history", "name" : "inner_tank_temperature_history", })
PROPULSION_HISTORY_FIELDS.append({ "group" : "propulsion_history", "name" : "outer_tank_temperature_history", })
FIELDS.extend(PROPULSION_HISTORY_FIELDS)

GNC_HISTORY_FIELDS = []
GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "position_history",       })
GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "velocity_history",       })
GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "position_other_history", })
GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "velocity_other_history", })
FIELDS.extend(GNC_HISTORY_FIELDS)