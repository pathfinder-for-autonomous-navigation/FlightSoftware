# Usage

```
src/gsw/MCT/dict_generators/generator.py [-h] [-t TELEMETRY] -d DICT -o OUTPUT
```

# Example

Running this from the root directory of FlightSoftware, using Python 3:

```
python src/gsw/MCT/dict_generators/generator.py -t telemetry -d src/gsw/MCT/dict_generators/gomspace.json -o src/gsw/MCT/public/battery.json   
```

# Specifying dictionaries like `gomspace.json`

There are five parameters:
- `key`: Key of top-level dict
- `name`: Name of top-level dict
- `filter`: Filter for searching the `telemetry` file for relevant data
- `keys` : Unit, name, and group data for each key
- `groups`: Names for each group of data

Keys that don't have groups must have names. Names are optional for keys that are contained inside a group. Units are always optional for keys.
