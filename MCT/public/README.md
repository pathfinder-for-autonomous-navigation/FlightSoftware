# Telemetry JSON and OpenMCT plugins info
---

#### Telemetry is stored as a JSON object with three basic fields:
1. name - name of the telemetry object (satellite, battery, etc.)
2. key - the accessible key value for the telemetry object that is used by the plugins to access its data
3. measurements - all of the telemetry points on the given telemetry object. 

##### There are five parameters:
- `key`: Key of top-level dict
- `name`: Name of top-level dict
- `filter`: Filter for searching the `telemetry` file for relevant data
- `keys` : Unit, name, and group data for each key
- `groups`: Names for each group of data

Keys that don't have groups must have names. Names are optional for keys that are contained inside a group, but the key must be 'value'. Units are always optional for keys.


**More info on plugin usage and for new Domain Object/Subsystem instillation in the [main MCT README]**

[main MCT README]: (https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/README.md)