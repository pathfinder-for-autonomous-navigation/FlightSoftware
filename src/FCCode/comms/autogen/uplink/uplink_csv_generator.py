import csv
import math

TYPES = ["bool", "int", "state int", "float vector", "double vector", "quaternion", "gps time"]
MAX_INT = 4294967295

FIELDS = [
    {"name" : "uplink_number",                      "type" : "state int", "min" : 0, "max" : MAX_INT},
    {"name" : "uplink_timestamp",                   "type" : "gps time"},
    {"name" : "other_satellite_position",           "type" : "double vector", "size" : 45, "min" : 6400, "max" : 7200},
    {"name" : "other_satellite_position_timestamp", "type" : "gps time" },
    {"name" : "other_satellite_velocity",           "type" : "double vector", "size" : 45, "min" : 8000, "max" : 12000},
    {"name" : "other_satellite_velocity_timestamp", "type" : "gps time" },
]

for x in range(0,3):
    FIELDS.append({"name" : "constant_{0}_id".format(x),  "type" : "state int",                 "min" : 0, "max" : 100})
    FIELDS.append({"name" : "constant_{0}_val".format(x), "type" : "int",       "size" : 32,    "min" : 0, "max" : MAX_INT})

FIELDS.append({"name" : "master_state", "type" : "state int", "min" : 0, "max" : 4  })
FIELDS.append({"name" : "pan_state",    "type" : "state int", "min" : 0, "max" : 11 })
FIELDS.append({"name" : "is_follower",  "type" : "bool" })

hat_devices = [
    "gomspace", "piksi", "quake", "dcdc", "spike_and_hold", "adcs_system",
    "pressure_sensor", "temp_sensor_inner", "temp_sensor_outer",
    "docking_motor", "docking_switch"
]
for device in hat_devices:
    FIELDS.append({"name" : "hat_{0}_error_ignored".format(device), "type" : "bool" })
adcs_hat_devices = [
    "gyroscope", "magnetometer", "magnetorquer_x", "magnetorquer_y", "magnetorquer_z",
    "motorpot", "motor_x", "motor_y", "motor_z", "motor_x_adc", "motor_y_adc", "motor_z_adc", 
    "ssa_adc_1", "ssa_adc_2", "ssa_adc_3", "ssa_adc_4", "ssa_adc_5"
]
for device in adcs_hat_devices:
    FIELDS.append({"name" : "adcs_hat_{0}_is_functional".format(device), "type" : "bool" })

FIELDS.append({ "name" : "adcs_state",         "type" : "state int", "min" : 0, "max" : 3 })
FIELDS.append({ "name" : "command_adcs",       "type" : "bool" })
FIELDS.append({ "name" : "adcs_frame",         "type" : "state int", "min" : 0, "max" : 3 })
FIELDS.append({ "name" : "adcs_attitude",      "type" : "quaternion" })

FIELDS.append({ "name" : "command_propulsion", "type" : "bool"})
FIELDS.append({ "name" : "firing_vector",      "type" : "float vector", "size" : 26, "min" : 0, "max" : 0.005 })
FIELDS.append({ "name" : "firing_time",        "type" : "gps time"})

FIELDS.append({ "name" : "docking_motor_mode", "type" : "bool" })

resettables = ["piksi", "quake"]
for resettable in resettables:
    FIELDS.append({ "name" : "reset_{0}".format(resettable),     "type" : "bool" })
cyclables = ["gomspace", "piksi", "quake", "dcdc", "adcs_system"]
for cyclable in cyclables:
    FIELDS.append({ "name" : "power_cycle_{0}".format(cyclable), "type" : "bool" })

# Auto-add field sizes
for field in FIELDS:
    if field["type"] == "quaternion":
        field["size"] = 29
    elif field["type"] == "gps time":
        field["size"] = 49
    elif field["type"] == "bool":
        field["size"] = 1
    elif field["type"] == "state int":
        if "max" not in field or "min" not in field:
            print "field {0} has undefined limits".format(field["name"])
            continue
        ints = field['max'] - field['min'] + 1
        field['size'] = int(math.ceil(math.log(ints, 2)))
        field['type'] = 'int'
    elif field["type"] not in TYPES:
        print "field {0} has undefined size".format(field['name'])

# Write fields to CSV
with open('uplink_fields.csv', 'wb') as output_file:
    dict_writer = csv.DictWriter(output_file, FIELDS[0].keys())
    dict_writer.writeheader()
    dict_writer.writerows(FIELDS)

#############
# Just for fun: size of uncompressed data
#############
def get_statistics():
    size = 0
    for field in FIELDS:
        if field["type"] == "bool":
            size += 8
        if field["type"] == "int" or field["type"] == "state int":
            size += 32
        if field["type"] == "float vector":
            size += (8 * 4 * 3)
        if field["type"] == "double vector":
            size += (8 * 8 * 3)
        if field["type"] == "quaternion":
            size += (8 * 4 * 4)
        if field["type"] == "gps time":
            size += (32 + 32)
        if field["type"] == "temperature":
            size += 16

    print "Number of fields: " + str(len(FIELDS))
    print "Full data size: " + str(int(math.ceil(size / 8))) + " bytes"
    compressed_size = sum([field["size"] for field in FIELDS])
    print "Compressed data size: " + str(int(math.ceil(
        compressed_size / 8))) + " bytes"
    print "Compression ratio: " + str(100 - 100 *
                                      (compressed_size + 0.0) / size)

get_statistics()