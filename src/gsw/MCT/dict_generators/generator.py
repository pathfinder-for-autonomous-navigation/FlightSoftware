import re, sys, json, os
from argparse import ArgumentParser

if __name__ == "__main__":
    parser = ArgumentParser(description='''
    Used to generate large telemetry dictionaries for the MCT software, based on simpler descriptions and the FSW-generated telemetry file.
    ''', prog="src/gsw/MCT/dict_generators/generator.py")

    parser.add_argument('-t', '--telemetry', action='store', help='Location of telemetry file.',
                        default = "telemetry")

    parser.add_argument('-d', '--dict', action='store', help='Location of simple dict describing subsystem telemetry.', required=True)

    parser.add_argument('-o', '--output', action='store', help='Location of where to store output.', required = True)

    args = parser.parse_args()

    dictfile = open(args.dict, 'r')
    subsystem_data = json.load(dictfile)
    dictfile.close()
    key_data = subsystem_data["keys"]
    group_data = subsystem_data["groups"]

    subsystem_dict = {
        "name": subsystem_data["name"],
        "key" : subsystem_data["key"],
        "measurements": {}
    }

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

    with open(args.output, "w") as f:
        json.dump(subsystem_dict, f, indent=4)
