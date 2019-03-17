import csv
import math

TYPES = ["bool", "int", "state int", "float vector", "double vector", "quaternion", "gps time"]
MAX_INT = 4294967295

## Other satellite data
FIELDS = [
    {"name" : "uplink_number",             "type" : "state int", "min" : 0, "max" : MAX_INT},
    {"name" : "other_satellite_position",  "type" : "double vector", "size" : 45, "min" : 6400, "max" : 7200},
    {"name" : "other_satellite_velocity",  "type" : "double vector", "size" : 45, "min" : 8000, "max" : 12000},
    {"name" : "other_satellite_timestamp", "type" : "gps time" },
]

## Constant parameters
for x in range(0,5):
    FIELDS.append({"name" : "constant_{0}_id".format(x),  "type" : "state int",                 "min" : 1, "max" : 19})
    FIELDS.append({"name" : "constant_{0}_val".format(x), "type" : "int",       "size" : 32,    "min" : 0, "max" : MAX_INT})

## State parameters
FIELDS.append({"name" : "master_state", "type" : "state int", "min" : 0, "max" : 4  })
FIELDS.append({"name" : "pan_state",    "type" : "state int", "min" : 0, "max" : 11 })
FIELDS.append({"name" : "is_follower",  "type" : "bool" })

## HAT parameters
hat_devices = [
    "gomspace", "piksi", "quake", "dcdc", "spike_and_hold", "adcs_system",
    "pressure_sensor", "temp_sensor_inner", "temp_sensor_outer",
    "docking_motor", "docking_switch"
]
for device in hat_devices:
    FIELDS.append({"name" : "fc_hat_{0}".format(device), "type" : "bool" })
adcs_hat_devices = [
    "gyroscope", "magnetometer_1", "magnetometer_2", "magnetorquer_x", "magnetorquer_y", "magnetorquer_z",
    "motorpot", "motor_x", "motor_y", "motor_z", "adc_motor_x", "adc_motor_y", "adc_motor_z",
    "ssa_adc_1", "ssa_adc_2", "ssa_adc_3", "ssa_adc_4", "ssa_adc_5"
]
for device in adcs_hat_devices:
    FIELDS.append({"name" : "adcs_hat_{0}".format(device), "type" : "bool" })

## If in safe hold or standby, error ignore parameters
# Safe hold error ignores
FIELDS.append({"name": "vbatt_ignored",                        "type": "bool" })
FIELDS.append({"name": "cannot_pressurize_outer_tank_ignored", "type": "bool" })
FIELDS.append({"name": "all_magnetometers_faulty_ignore", "type": "bool"})
FIELDS.append({"name": "all_ssa_faulty_ignore", "type": "bool"})
FIELDS.append({"name": "motor_x_faulty_ignore", "type": "bool"})
FIELDS.append({"name": "motor_y_faulty_ignore", "type": "bool"})
FIELDS.append({"name": "motor_z_faulty_ignore", "type": "bool"})
# Standby error ignores
FIELDS.append({"name": "ignore_destabilized", "type": "bool" })
FIELDS.append({"name": "ignore_overpressure", "type": "bool" })


## Actuate actuators
FIELDS.append({ "name" : "adcs_state",         "type" : "state int", "min" : 0, "max" : 3 })
FIELDS.append({ "name" : "command_adcs",       "type" : "bool" })
FIELDS.append({ "name" : "adcs_frame",         "type" : "state int", "min" : 0, "max" : 1 })
FIELDS.append({ "name" : "adcs_attitude",      "type" : "quaternion" })

FIELDS.append({ "name" : "command_propulsion", "type" : "bool"})
FIELDS.append({ "name" : "firing_vector",      "type" : "float vector", "size" : 26, "min" : 0, "max" : 0.005 })
FIELDS.append({ "name" : "firing_time",        "type" : "gps time"})

FIELDS.append({ "name" : "docking_motor_mode", "type" : "bool" })

## Actuate power cycles and resets
resettables = ["piksi", "quake", "dcdc", "spike_and_hold"]
for resettable in resettables:
    FIELDS.append({ "name" : "reset_{0}".format(resettable),     "type" : "bool" })
cyclables = ["gomspace", "piksi", "quake", "adcs_system", "spike_and_hold"]
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

    print "Uplink:"
    print "Number of fields: " + str(len(FIELDS))
    print "Full data size: " + str(int(math.ceil(size / 8))) + " bytes"
    compressed_size = sum([field["size"] for field in FIELDS])
    print "Compressed data size: " + str(int(math.ceil(
        compressed_size / 8))) + " bytes"
    print "Compression ratio: " + str(100 - 100 *
                                      (compressed_size + 0.0) / size)
    print "--------------------------------"


get_statistics()