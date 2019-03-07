import csv

TYPES = ["bool", "int", "float", "double", "float vector", "double vector", "quaternion", "gps time", "temperature"]
GROUPS = {
 "master" : "State::Master",
 "hardware" : "State::Hardware",
 "gomspace" : "State::Gomspace",
 "propulsion" : "State::Propulsion",
 "adcs" : "State::ADCS",
 "quake" : "State::Quake",
 "gnc" : "State::GNC",
 "piksi" : "State::Piksi",
 "adcs_history" : "StateHistory::ADCS",
 "propulsion_history" : "StateHistory::Propulsion",
 "gnc_history" : "StateHistory::GNC",
 "piksi_history" : "StateHistory::Piksi",
}

FIELDS = []


MASTER_FIELDS = [
{ "group" : "master", "name" : "master_state",          "type" : "int",        "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "master", "name" : "pan_state",             "type" : "int",        "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "master", "name" : "boot_number",           "type" : "int",        "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "master", "name" : "last_uplink_time",      "type" : "gps time",   "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "master", "name" : "was_last_uplink_valid", "type" : "bool",       "size" : 1},
{ "group" : "master", "name" : "is_follower",           "type" : "bool",       "size" : 1},
]
FIELDS.extend(MASTER_FIELDS)

hat_devices = ["gomspace", "piksi", "quake", "dcdc", "spike_and_hold", "adcs_system", "pressure_sensor",
    "temp_sensor_inner", "temp_sensor_outer", "docking_motor", "docking_switch"]
hat_fields = ["powered_on", "enabled", "is_functional", "error_ignored"]
HAT_FIELDS = []
for device in hat_devices:
    for field in hat_fields:
        HAT_FIELDS.append({ "group" : "hardware", "name" : "hat.at(Devices::{0}.name()).{1}".format(device, field), "type" : "bool", "size": 1})
FIELDS.extend(HAT_FIELDS)

ADCS_FIELDS = [
{ "group" : "adcs", "name" : "adcs_state",                       "type" : "int",          "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "cmd_attitude",                     "type" : "quaternion",   "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "cur_attitude",                        "type" : "quaternion",   "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "cur_angular_rate",                    "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "is_propulsion_pointing_active",       "type" : "bool", "size" : 1},
{ "group" : "adcs", "name" : "is_sun_vector_determination_working", "type" : "bool", "size" : 1},
{ "group" : "adcs", "name" : "rwa_speeds",             "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "rwa_speed_cmds",           "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "rwa_ramps",             "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "rwa_speeds_rd",           "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "rwa_speed_cmds_rd",          "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "rwa_ramps_rd",            "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "mtr_cmds",             "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "ssa_vec",              "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "gyro_data",             "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "mag_data",             "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
{ "group" : "adcs", "name" : "ssa_adc_data",            "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 },
]
FIELDS.extend(ADCS_FIELDS)

GOMSPACE_FIELDS = []
for x in range(0, 3):
    GOMSPACE_FIELDS.append({ "group" : "gomspace", "name" : "gomspace_data.vboost[{0}]".format(x),  "type" : "int", "size" : 11, "min" : 0, "max" : 0 })
GOMSPACE_FIELDS.append(    { "group" : "gomspace", "name" : "gomspace_data.vbatt",                  "type" : "int", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 3):
    GOMSPACE_FIELDS.append({ "group" : "gomspace", "name" : "gomspace_data.curin[{0}]".format(x),   "type" : "int", "size" : 11, "min" : 0, "max" : 0 })
GOMSPACE_FIELDS.append(    { "group" : "gomspace", "name" : "gomspace_data.cursun",                 "type" : "int", "size" : 11, "min" : 0, "max" : 0 })
GOMSPACE_FIELDS.append(    { "group" : "gomspace", "name" : "gomspace_data.cursys",                 "type" : "int", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 6):
    GOMSPACE_FIELDS.append({ "group" : "gomspace", "name" : "gomspace_data.curout[{0}]".format(x),  "type" : "int", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 8):
    GOMSPACE_FIELDS.append({ "group" : "gomspace", "name" : "gomspace_data.output[{0}]".format(x),  "type" : "bool", "size" : 1})
GOMSPACE_FIELDS.append(    { "group" : "gomspace", "name" : "gomspace_data.counter_boot",           "type" : "int", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 4):
    GOMSPACE_FIELDS.append({ "group" : "gomspace", "name" : "gomspace_data.temp[{0}]".format(x),    "type" : "temperature", "size" : 11 })
GOMSPACE_FIELDS.append(    { "group" : "gomspace", "name" : "gomspace_data.battmode",               "type" : "int", "size" : 11, "min" : 0, "max" : 0 })
FIELDS.extend(GOMSPACE_FIELDS)

PROPULSION_FIELDS = []
PROPULSION_FIELDS.append({ "group" : "propulsion", "name" : "propulsion_state",            "type" : "int",          "size" : 11, "min" : 0, "max" : 0 })
PROPULSION_FIELDS.append({ "group" : "propulsion", "name" : "delta_v_available",           "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
PROPULSION_FIELDS.append({ "group" : "propulsion", "name" : "is_firing_planned",           "type" : "bool",         "size" : 1 })
PROPULSION_FIELDS.append({ "group" : "propulsion", "name" : "is_firing_planned_by_uplink", "type" : "bool",         "size" : 1 })
PROPULSION_FIELDS.append({ "group" : "propulsion", "name" : "firing_data.impulse_vector",  "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
PROPULSION_FIELDS.append({ "group" : "propulsion", "name" : "firing_data.time",            "type" :  "gps time",    "size" : 11 })
FIELDS.extend(PROPULSION_FIELDS)

PIKSI_FIELDS = [
{ "group" : "piksi", "name" : "recorded_current_time",                      "type" : "gps time", "size" : 11 },
{ "group" : "piksi", "name" : "recorded_current_time_collection_timestamp", "type" : "int",      "size" : 32 },
]
FIELDS.extend(PIKSI_FIELDS)

GNC_FIELDS = [
{ "group" : "gnc", "name" : "current_time",                      "type" : "gps time", "size" : 11 },
{ "group" : "gnc", "name" : "current_time_collection_timestamp", "type" : "int",      "size" : 32, "min" : 0, "max" : 0 },
{ "group" : "gnc", "name" : "has_firing_happened_in_nighttime",  "type" : "bool",     "size" : 1 },
]
FIELDS.extend(GNC_FIELDS)

ADCS_HISTORY_FIELDS = []
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "attitude_cmd_history[{0}]".format(x),      "type" : "quaternion", "size" : 11 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "attitude_history[{0}]".format(x),        "type" : "quaternion", "size" : 11 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "attitude_fast_history[{0}]".format(x),  "type" : "quaternion", "size" : 11 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "rate_history[{0}]".format(x),           "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "rate_fast_history[{0}]".format(x),      "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "spacecraft_L_history[{0}]".format(x),   "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "spacecraft_L_fast_history[{0}]".format(x), "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "gyro_history[{0}]".format(x),      "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "gyro_fast_history[{0}]".format(x),   "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "magnetometer_history[{0}]".format(x),    "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "rwa_ramp_cmd_history[{0}]".format(x),    "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "mtr_cmd_history[{0}]".format(x),         "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "ssa_vector_history[{0}]".format(x),       "type" : "float vector", "size" : 11, "min" : 0, "max" : 0 })
FIELDS.extend(ADCS_HISTORY_FIELDS)

PROPULSION_HISTORY_FIELDS = []
for x in range(0, 20):
    PROPULSION_HISTORY_FIELDS.append({ "group" : "propulsion_history", "name" : "tank_pressure_history[{0}]".format(x),          "type" : "float", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    PROPULSION_HISTORY_FIELDS.append({ "group" : "propulsion_history", "name" : "inner_tank_temperature_history[{0}]".format(x), "type" : "float", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    PROPULSION_HISTORY_FIELDS.append({ "group" : "propulsion_history", "name" : "outer_tank_temperature_history[{0}]".format(x), "type" : "float", "size" : 11, "min" : 0, "max" : 0 })
FIELDS.extend(PROPULSION_HISTORY_FIELDS)

PIKSI_HISTORY_FIELDS = []
for x in range(0, 20):
    PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "recorded_position_history[{0}]".format(x), "type" : "double vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "recorded_velocity_history[{0}]".format(x), "type" : "double vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "recorded_position_other_history[{0}]".format(x), "type" : "double vector", "size" : 11, "min" : 0, "max" : 0 })
FIELDS.extend(PIKSI_HISTORY_FIELDS)

GNC_HISTORY_FIELDS = []
for x in range(0, 20):
    GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "position_history[{0}]".format(x),       "type" : "double vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "velocity_history[{0}]".format(x),       "type" : "double vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "position_other_history[{0}]".format(x), "type" : "double vector", "size" : 11, "min" : 0, "max" : 0 })
for x in range(0, 20):
    GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "velocity_other_history[{0}]".format(x), "type" : "double vector", "size" : 11, "min" : 0, "max" : 0 })
FIELDS.extend(GNC_HISTORY_FIELDS)

# Fully qualify each field's name and group
for field in FIELDS:
    field["group"] = GROUPS[field["group"]]
    field["name"] = field["group"] + "::" + field["name"]

# Write fields to CSV
import csv
keys = FIELDS[0].keys()
with open('downlink_fields.csv', 'wb') as output_file:
    dict_writer = csv.DictWriter(output_file, keys)
    dict_writer.writeheader()
    dict_writer.writerows(FIELDS)