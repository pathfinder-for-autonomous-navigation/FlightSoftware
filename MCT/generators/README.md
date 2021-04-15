# Generators:

### 1. [state_variables_generator.py]

This file generates [state-variable.js] from the [flow_data.cpp] file. Run this file and [state-variable.js] will automatically update with the new values
### 2. [subsystem_json_generator.py]

This file generates every subsystem/domain object's json file for MCT. Run this file and every json file in the [MCT\public\subsystems] directory will be replaced with an updated version.

### 3. [mct_es_config.sh]

This file will configure your local elasticsearch file to enable CORS for mct to store its persistence data within the database. If you run this file, it will determine your operating system and then will add the configuration lines to the elasticsearch.yml config file on your computer depending on ur OS (If the configuration is already present it will not readd them) RUn with `./mct_elasticsearch_configuration.sh` - if you encounter permissions errors with the file run ```chmod u+x mct_elasticsearch_configuration.sh``` in the directory containing the file (```MCT/generators```)


[state_variables_generator.py]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/generators/state_variables_generator.py
[state-variable.js]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/server-files/state-variables.js
[flow_data.cpp]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/src/flow_data.cpp
[subsystem_json_generator.py]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/generators/subsystem_json_generator.py
[MCT\public\subsystems]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/tree/master/MCT/public/subsystems
[mct_es_config.sh]: https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/blob/master/MCT/generators/mct_es_config.sh