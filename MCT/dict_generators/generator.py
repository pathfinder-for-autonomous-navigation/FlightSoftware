import re, sys, json, os
from argparse import ArgumentParser

def get_arguments():
    parser = ArgumentParser(description='''
    Used to generate large telemetry dictionaries for the MCT software, based on simpler descriptions and the FSW-generated telemetry file.
    ''', prog="src/gsw/MCT/dict_generators/generator.py")

    parser.add_argument('-t', '--telemetry', action='store', help='Location of telemetry file.',
                        default = "telemetry")

    parser.add_argument('-d', '--dict', action='store', help='Location of simple dict describing subsystem telemetry.', required=True)

    parser.add_argument('-o', '--output', action='store', help='Location of where to store output.', required = True)

    return parser.parse_args()

if __name__ == "__main__":
    args = get_arguments()

    # Read in input JSON file.
    dictfile = open(args.dict, 'r')
    subsystem_data = json.load(dictfile)
    dictfile.close()

    # Key data consists of all individual data items.
    # See the format of gomspace.json for an example.
    key_data = subsystem_data["keys"]

    # Group data names groups that will contain several keys
    # See the format of gomspace.json for an example.
    group_data = subsystem_data["groups"]

    # Base-level data structure that we will populatee
    subsystem_dict = {
        "name": subsystem_data["name"],
        "key" : subsystem_data["key"],
        "measurements": {}
    }

    # Pull in FSW-produced telemetry info for the sake of getting value bounds on each key
    with open(args.telemetry, 'r') as f:
        # Find items in telemetry info that start with the name of the subsystem whose telemetry we're currently
        # processing
        telem_data = {k: v for k, v in json.load(f)["fields"].items() if k.startswith(subsystem_data["filter"])}

        # Extract data from the telemetry info file for each key that was matched, and put the
        # extracted data into subsystem dict.
        for key in telem_data:
            key_data.setdefault(key, {})

            # Get all data for the current key from the telemetry info
            val = telem_data[key]
            val["key"] = key
            val["hints"] = {
                "range" : "1"
            }

            # Store the data as part of the data that we collect on the key.
            key_data[key]["value"] = val

            # Get data from input JSON file and use it to populate:

            # 1. Units
            if "units" in key_data[key]:
                val["units"] = key_data[key]["units"]

            # 2. The group of the key
            if "group" in key_data[key]:
                group = key_data[key]["group"]
                group_name = group_data[group]["name"]
                group_expanded = subsystem_dict["measurements"].setdefault(group, {
                    "key" : group,
                    "name" : group_name,
                    "values" : []
                })
                group_expanded["values"].append(val)
                subsystem_dict["measurements"][group] = group_expanded

        # Go back through the collected data and reorganize grouped keys
        # in a way that OpenMCT likes
        for key in key_data:
            if "group" in key_data[key]: continue

            key_name = key_data[key]["name"]
            key_val = key_data[key]["value"]
            key_val["key"] = "value"

            measurement = {
                key : {
                    "name" : key_name,
                    "values" : [key_val]
                }
            }
            subsystem_dict["measurements"].update(measurement)

    # Go back through the collected data and add JSON fields that
    # OpenMCT likes
    measurements = []
    for measurement in subsystem_dict["measurements"]:
        data = subsystem_dict["measurements"][measurement]
        time_value = {
            "key": "utc",
            "source": "timestamp",
            "name": "Timestamp",
            "format": "utc",
            "hints": {
                "domain": 1
            }
        }
        data["values"].append(time_value)
        data["key"] = measurement
        measurements.append(data)
    subsystem_dict["measurements"] = measurements

    # Make output file readable by humans
    with open(args.output, "w") as f:
        json.dump(subsystem_dict, f, indent=4)
