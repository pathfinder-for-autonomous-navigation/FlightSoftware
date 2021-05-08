# Generators:

### 1. [state_variables_generator.py]

This file generates [state-variable.js] from the [flow_data.cpp] file. Run this file and [state-variable.js] will automatically update with the new values
### 2. [subsystem_json_generator.py]

This file generates every subsystem/domain object's json file for MCT. Run this file and every json file in the [MCT\public\subsystems] directory will be replaced with an updated version.

### 3. [mct_setup.sh]

This file runs but `state_variables_generator.py` and `subsystem_json_generator.py`. You must run this from the root FlightSoftware directory


[state_variables_generator.py]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/generators/state_variables_generator.py
[state-variable.js]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/server-files/state-variables.js
[flow_data.cpp]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/src/flow_data.cpp
[subsystem_json_generator.py]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/generators/subsystem_json_generator.py
[MCT\public\subsystems]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/tree/master/MCT/public/subsystems
[mct_setup.sh]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/generators/mct_setup.sh