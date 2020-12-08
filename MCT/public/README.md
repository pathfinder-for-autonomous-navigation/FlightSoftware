#Telemetry and OpenMCT plugins info

Telemetry is stored as a JSON object with three basic fields:
1. name - name of the telemetry object (satellite, battery, etc.)
2. key - the accessible key value for the telemetry object that is used by the plugins to access its data
3. measurements - all of the telemetry points on the given telemetry object. 

There are five parameters:
- `key`: Key of top-level dict
- `name`: Name of top-level dict
- `filter`: Filter for searching the `telemetry` file for relevant data
- `keys` : Unit, name, and group data for each key
- `groups`: Names for each group of data

Keys that don't have groups must have names. Names are optional for keys that are contained inside a group, but the key must be 'value'. Units are always optional for keys.

**Plugins**
The purpose of plugins is to allow openMCT to process our own types of telemetry and display telemetry points and objects in the object tree.
![Object Tree](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/addmct/src/gsw/MCT/images/object-tree.png)

There are 4 main functions and variables in each plugin:

1. The getter function at the top of each plugin uses HTTP to import the respective JSON file for the plugin to parse. It is used every time information from the JSON object is required by the other functions.

2. The plugin function at the bottom of each plugin file is installed by openmct and called in index.html, as this file is the bridge between the telemetry file and openMCT.

- The plugin function calls addRoot(openmct) and addType(openmct) which are both openMCT pre defined functions that display an object in the object tree and define the object as a new type for openmct to display.

3. The plugin function uses the **object provider variable** to define the specific name and key fro openmct to display

4. The plugin function uses the **composition provider variable** to assign each defined telemetry point in the JSON object file to the main telemetry object. (ex. setting the key from battery_voltage to sat.battery_voltage)

**more info on plugins and specifically the generic-plugin in the main MCT README**

**look at the main MCT README for new Domain Object/Subsystem instillation**
