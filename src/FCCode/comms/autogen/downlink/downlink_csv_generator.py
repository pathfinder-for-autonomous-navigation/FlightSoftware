import csv
import math

TYPES = [
    "bool", "int", "state int", "float", "double", "float vector",
    "double vector", "quaternion", "gps time", "temperature"
]
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
 "adcs_faults" : "FaultState::ADCS",
 "gomspace_faults" : "FaultState::Gomspace",
 "propulsion_faults" : "FaultState::Propulsion",
}

FIELDS = []

MAX_INT = 4294967295

MASTER_FIELDS = [
{ "group" : "master", "name" : "master_state",          "type" : "state int",               "min" : 0, "max" : 4 },
{ "group" : "master", "name" : "pan_state",             "type" : "state int",               "min" : 0, "max" : 11 },
{ "group" : "master", "name" : "boot_number",           "type" : "int",        "size" : 32, "min" : 0, "max" : MAX_INT },
{ "group" : "master", "name" : "last_uplink_time",      "type" : "gps time"},
{ "group" : "master", "name" : "was_last_uplink_valid", "type" : "bool" },
{ "group" : "master", "name" : "is_follower",           "type" : "bool" },
]
FIELDS.extend(MASTER_FIELDS)

hat_devices = ["gomspace", "piksi", "quake", "dcdc", "spike_and_hold", "adcs_system", "pressure_sensor",
    "temp_sensor_inner", "temp_sensor_outer", "docking_motor", "docking_switch"]
hat_fields = ["powered_on", "enabled", "is_functional", "error_ignored"]
HAT_FIELDS = []
for device in hat_devices:
    for field in hat_fields:
        HAT_FIELDS.append({ "group" : "hardware", "name" : "hat.at(Devices::{0}.name()).{1}".format(device, field), "type" : "bool" })
FIELDS.extend(HAT_FIELDS)

ADCS_FIELDS = [
{ "group" : "adcs", "name" : "adcs_state",                          "type" : "state int",                 "min" : 0, "max" : 3 },
{ "group" : "adcs", "name" : "cmd_attitude",                        "type" : "quaternion" },
{ "group" : "adcs", "name" : "cur_attitude",                        "type" : "quaternion" },
{ "group" : "adcs", "name" : "cur_ang_rate",                        "type" : "float vector", "size" : 30, "min" : 0, "max" : 2.2 },
{ "group" : "adcs", "name" : "is_sun_vector_determination_working", "type" : "bool" },
{ "group" : "adcs", "name" : "rwa_speeds",                          "type" : "float vector", "size" : 50, "min" : 0, "max" : 0 }, #TODO
{ "group" : "adcs", "name" : "rwa_speed_cmds",                      "type" : "float vector", "size" : 50, "min" : 0, "max" : 0 }, #TODO
{ "group" : "adcs", "name" : "rwa_ramps",                           "type" : "float vector", "size" : 29, "min" : 0, "max" : 310.2 },
{ "group" : "adcs", "name" : "rwa_speeds_rd",                       "type" : "float vector", "size" : 50, "min" : 0, "max" : 0 }, #TODO
{ "group" : "adcs", "name" : "rwa_speed_cmds_rd",                   "type" : "float vector", "size" : 50, "min" : 0, "max" : 0 }, #TODO
{ "group" : "adcs", "name" : "rwa_ramps_rd",                        "type" : "float vector", "size" : 29, "min" : 0, "max" : 310.2 },
{ "group" : "adcs", "name" : "mtr_cmds",                            "type" : "float vector", "size" : 50, "min" : 0, "max" : 0 }, #TODO
{ "group" : "adcs", "name" : "ssa_vec",                             "type" : "float vector", "size" : 21, "min" : 0, "max" : 1 }
]
for x in range(0, 20):
    ADCS_FIELDS.append({ "group" : "adcs", "name" : "ssa_adc_data[{0}]".format(x), "type" : "float", "size" : 8, "min" : 0, "max" : 0 }) #TODO
FIELDS.extend(ADCS_FIELDS)

GOMSPACE_FIELDS = []
for x in range(0, 3):
    GOMSPACE_FIELDS.append({ "group" : "gomspace", "name" : "gomspace_data.vboost[{0}]".format(x),  "type" : "int", "size" : 12, "min" : 0, "max" : 4000 })
GOMSPACE_FIELDS.append(    { "group" : "gomspace", "name" : "gomspace_data.vbatt",                  "type" : "int", "size" : 11, "min" : 5000, "max" : 9000 })
for x in range(0, 3):
    GOMSPACE_FIELDS.append({ "group" : "gomspace", "name" : "gomspace_data.curin[{0}]".format(x),   "type" : "int", "size" : 10, "min" : 0, "max" : 1000 })
GOMSPACE_FIELDS.append(    { "group" : "gomspace", "name" : "gomspace_data.cursun",                 "type" : "int", "size" : 50, "min" : 0, "max" : 0 }) #TODO
GOMSPACE_FIELDS.append(    { "group" : "gomspace", "name" : "gomspace_data.cursys",                 "type" : "int", "size" : 50, "min" : 0, "max" : 0 }) #TODO
for x in range(0, 6):
    GOMSPACE_FIELDS.append({ "group" : "gomspace", "name" : "gomspace_data.curout[{0}]".format(x),  "type" : "int", "size" : 7, "min" : 0, "max" : 1000 })
for x in range(0, 8):
    GOMSPACE_FIELDS.append({ "group" : "gomspace", "name" : "gomspace_data.output[{0}]".format(x),  "type" : "bool" })
GOMSPACE_FIELDS.append(    { "group" : "gomspace", "name" : "gomspace_data.counter_boot",           "type" : "int", "size" : 32, "min" : 0, "max" : MAX_INT })
for x in range(0, 4):
    GOMSPACE_FIELDS.append({ "group" : "gomspace", "name" : "gomspace_data.temp[{0}]".format(x),    "type" : "temperature", "size" : 9 })
GOMSPACE_FIELDS.append(    { "group" : "gomspace", "name" : "gomspace_data.battmode",               "type" : "int", "size" : 50, "min" : 0, "max" : 0 }) #TODO
FIELDS.extend(GOMSPACE_FIELDS)

PROPULSION_FIELDS = []
PROPULSION_FIELDS.append({ "group" : "propulsion", "name" : "propulsion_state",            "type" : "state int",                 "min" : 0, "max" : 5 })
PROPULSION_FIELDS.append({ "group" : "propulsion", "name" : "firing_data.impulse_vector",  "type" : "float vector", "size" : 26, "min" : 0, "max" : 0.005 })
PROPULSION_FIELDS.append({ "group" : "propulsion", "name" : "firing_data.time",            "type" : "gps time",     "size" : 49 })
FIELDS.extend(PROPULSION_FIELDS)

PIKSI_FIELDS = [
{ "group" : "piksi", "name" : "recorded_current_time",              "type" : "gps time"},
{ "group" : "piksi", "name" : "recorded_time_collection_timestamp", "type" : "int",      "size" : 32, "min" : 0, "max" : MAX_INT },
]
FIELDS.extend(PIKSI_FIELDS)

GNC_FIELDS = [
{ "group" : "gnc", "name" : "current_time",                     "type" : "gps time"},
{ "group" : "gnc", "name" : "time_collection_timestamp",        "type" : "int",      "size" : 32, "min" : 0, "max" : MAX_INT },
{ "group" : "gnc", "name" : "has_firing_happened_in_nighttime", "type" : "bool"},
]
FIELDS.extend(GNC_FIELDS)

ADCS_HISTORY_FIELDS = []
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "gyro_history",              "type" : "float vector", "size" : 50, "buf_size" : 10, "min" : -2.2, "max" : 2.2 }) #TODO
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "gyro_fast_history",         "type" : "float vector", "size" : 50, "buf_size" : 10, "min" : 0, "max" : 0 }) #TODO
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "attitude_cmd_history",      "type" : "quaternion" , "buf_size" : 10 })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "attitude_history",          "type" : "quaternion" , "buf_size" : 10 })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "attitude_fast_history",     "type" : "quaternion" , "buf_size" : 10 })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "rate_history",              "type" : "float vector", "size" : 30, "buf_size" : 10, "min" : -2.2, "max" : 2.2 })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "rate_fast_history",         "type" : "float vector", "size" : 30, "buf_size" : 10, "min" : -2.2, "max" : 2.2 })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "spacecraft_L_history",      "type" : "float vector", "size" : 50, "buf_size" : 10, "min" : 0, "max" : 0 }) #TODO
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "spacecraft_L_fast_history", "type" : "float vector", "size" : 50, "buf_size" : 10, "min" : 0, "max" : 0 }) #TODO
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "magnetometer_history",      "type" : "float vector", "size" : 29, "buf_size" : 10, "min" : 0, "max" : 0.005 })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "rwa_ramp_cmd_history",      "type" : "float vector", "size" : 29, "buf_size" : 10, "min" : -310.2, "max" : 310.2 })
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "mtr_cmd_history",           "type" : "float vector", "size" : 50, "buf_size" : 10, "min" : 0, "max" : 0 }) #TODO
ADCS_HISTORY_FIELDS.append({ "group" : "adcs_history", "name" : "ssa_vector_history",        "type" : "float vector", "size" : 21, "buf_size" : 10, "min" : 0, "max" : 1 })
FIELDS.extend(ADCS_HISTORY_FIELDS)

PIKSI_HISTORY_FIELDS = []
PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "iar_history",                     "type" : "int", "size" : 32, "buf_size" : 10, "min" : 0, "max" : MAX_INT })
PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "nsats_history",                   "type" : "int", "size" : 5,  "buf_size" : 10,  "min" : 0, "max" : 30 })
PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "recorded_position_history",       "type" : "double vector", "size" : 45, "buf_size" : 10, "min" : 6400, "max" : 7200 })
PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "recorded_velocity_history",       "type" : "double vector", "size" : 51, "buf_size" : 10, "min" : 8000, "max" : 12000 })
PIKSI_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "recorded_position_other_history", "type" : "double vector", "size" : 45, "buf_size" : 10, "min" : 6400, "max" : 7200 })
FIELDS.extend(PIKSI_HISTORY_FIELDS)

PROPULSION_HISTORY_FIELDS = []
PROPULSION_HISTORY_FIELDS.append({ "group" : "propulsion_history", "name" : "tank_pressure_history",          "type" : "float", "size" : 10, "buf_size" : 10, "min" : 0, "max" : 100 })
PROPULSION_HISTORY_FIELDS.append({ "group" : "propulsion_history", "name" : "inner_tank_temperature_history", "type" : "temperature" ,       "buf_size" : 10 })
PROPULSION_HISTORY_FIELDS.append({ "group" : "propulsion_history", "name" : "outer_tank_temperature_history", "type" : "temperature" ,       "buf_size" : 10 })
FIELDS.extend(PROPULSION_HISTORY_FIELDS)

GNC_HISTORY_FIELDS = []
GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "position_history",       "type" : "double vector", "size" : 45, "buf_size" : 10, "min" : 6400, "max" : 7200 })
GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "velocity_history",       "type" : "double vector", "size" : 51, "buf_size" : 10, "min" : 8000, "max" : 12000 })
GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "position_other_history", "type" : "double vector", "size" : 45, "buf_size" : 10, "min" : 6400, "max" : 7200 })
GNC_HISTORY_FIELDS.append({ "group" : "piksi_history", "name" : "velocity_other_history", "type" : "double vector", "size" : 51, "buf_size" : 10, "min" : 8000, "max" : 12000 })
FIELDS.extend(GNC_HISTORY_FIELDS)

GOMSPACE_FAULTS_FIELDS = []
for x in range(0, 20):
    GOMSPACE_FAULTS_FIELDS.append({ "group" : "gomspace_faults", "name" : "fault_bits[i]", "type" : "bool" })
FIELDS.extend(GOMSPACE_FAULTS_FIELDS)

PROPULSION_FAULTS_FIELDS = []
PROPULSION_FAULTS_FIELDS.append({ "group" : "propulsion_faults", "name" : "overpressure_event",    "type" : "gps time" })
PROPULSION_FAULTS_FIELDS.append({ "group" : "propulsion_faults", "name" : "overpressure_event_id", "type" : "int", "size" : 2, "min" : 0, "max" : 3 })
PROPULSION_FAULTS_FIELDS.append({ "group" : "propulsion_faults", "name" : "destabilization_event", "type" : "gps time" })
FIELDS.extend(PROPULSION_FAULTS_FIELDS)

# Fully qualify each field's name and group
for field in FIELDS:
    field["group_nickname"] = field["group"]
    field["group"] = GROUPS[field["group"]]
    field["name"] = field["group"] + "::" + field["name"]

# Auto-add field sizes
for field in FIELDS:
    if field["type"] == "quaternion":
        field["size"] = 29
    elif field["type"] == "gps time":
        field["size"] = 49
    elif field["type"] == "bool":
        field["size"] = 1
    elif field["type"] == "temperature":
        field["size"] = 9
    elif field["type"] == "state int":
        if "max" not in field or "min" not in field:
            print "field {0} has undefined limits".format(field["name"])
            continue
        ints = field['max'] - field['min'] + 1
        field['size'] = int(math.ceil(math.log(ints, 2)))
        field['type'] = "int"
    elif field["type"] not in TYPES:
        print "field {0} has undefined size".format(field['name'])

# Update buffer sizes for non-history fields
for field in FIELDS:
    if "history" not in field["name"]:
        field["buf_size"] = 1

# Write fields to CSV
with open('downlink_fields.csv', 'wb') as output_file:
    dict_writer = csv.DictWriter(output_file, FIELDS[0].keys())
    dict_writer.writeheader()
    dict_writer.writerows(FIELDS)

#############
# Just for fun: size of uncompressed data
#############
def get_statistics():
    size = 0
    for field in FIELDS:
        field["total_size"] = field["buf_size"] * field["size"]
        if field["type"] == "bool":
            size += field["buf_size"] * 8
        if field["type"] == "int" or field["type"] == "state int":
            size += field["buf_size"] * 32
        if field["type"] == "float":
            size += field["buf_size"] * (8 * 4)
        if field["type"] == "double":
            size += field["buf_size"] * (8 * 8)
        if field["type"] == "float vector":
            size += field["buf_size"] * (8 * 4 * 3)
        if field["type"] == "double vector":
            size += field["buf_size"] * (8 * 8 * 3)
        if field["type"] == "quaternion":
            size += field["buf_size"] * (8 * 4 * 4)
        if field["type"] == "gps time":
            size += field["buf_size"] * (32 + 32)
        if field["type"] == "temperature":
            size += field["buf_size"] * 16

    print "Number of fields: " + str(len(FIELDS))
    print "Full data size: " + str(int(math.ceil(size / 8))) + " bytes"
    compressed_size = sum([field["total_size"] for field in FIELDS])
    print "Compressed data size: " + str(int(math.ceil(compressed_size / 8))) + " bytes"
    print "Compression ratio: " + str(100 - 100 * (compressed_size + 0.0) / size)
    print "Required # of packets: " + str(
        int(math.ceil(compressed_size / 8.0 / 70)))

get_statistics()
