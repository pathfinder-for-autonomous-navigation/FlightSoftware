import re, sys, json, os
from argparse import ArgumentParser

<<<<<<< HEAD
if __name__ == "__main__":
=======
def get_arguments():
>>>>>>> ec708bd8b98045ab2e0283ade80d25eac08aa882
    parser = ArgumentParser(description='''
    Used to generate large telemetry dictionaries for the MCT software, based on simpler descriptions and the FSW-generated telemetry file.
    ''', prog="src/gsw/MCT/dict_generators/generator.py")

    parser.add_argument('-t', '--telemetry', action='store', help='Location of telemetry file.',
                        default = "telemetry")

    parser.add_argument('-d', '--dict', action='store', help='Location of simple dict describing subsystem telemetry.', required=True)

    parser.add_argument('-o', '--output', action='store', help='Location of where to store output.', required = True)

<<<<<<< HEAD
    args = parser.parse_args()

    dictfile = open(args.dict, 'r')
    subsystem_data = json.load(dictfile)
    dictfile.close()
    key_data = subsystem_data["keys"]
    group_data = subsystem_data["groups"]

=======
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
>>>>>>> ec708bd8b98045ab2e0283ade80d25eac08aa882
    subsystem_dict = {
        "name": subsystem_data["name"],
        "key" : subsystem_data["key"],
        "measurements": {}
    }

<<<<<<< HEAD
    with open(args.telemetry, 'r') as f:
        telem_data = {k: v for k, v in json.load(f)["fields"].items() if k.startswith(subsystem_data["filter"])}

        for key in telem_data:
            key_data.setdefault(key, {})

            val = telem_data[key]
            val["key"] = key

            val["hints"] = {
                "range" : "1"
            }
            key_data[key]["value"] = val

            if "units" in key_data[key]:
                val["units"] = key_data[key]["units"]

=======
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
>>>>>>> ec708bd8b98045ab2e0283ade80d25eac08aa882
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

<<<<<<< HEAD
=======
        # Go back through the collected data and reorganize grouped keys
        # in a way that OpenMCT likes
>>>>>>> ec708bd8b98045ab2e0283ade80d25eac08aa882
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

<<<<<<< HEAD
=======
    # Go back through the collected data and add JSON fields that
    # OpenMCT likes
>>>>>>> ec708bd8b98045ab2e0283ade80d25eac08aa882
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

<<<<<<< HEAD
=======
    # Make output file readable by humans
>>>>>>> ec708bd8b98045ab2e0283ade80d25eac08aa882
    with open(args.output, "w") as f:
        json.dump(subsystem_dict, f, indent=4)
