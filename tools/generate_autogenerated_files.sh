# Run this file prior to opening a PR, so that your code will not fail
# on the file diff checks in tools/run_ground_tests.sh

# Generate flow_data.cpp
rm -f src/flow_data.cpp
python src/flow_data_generator.py

# Generate `constants` file
rm -f constants
python tools/constant_reporter.py

# Generate `telemetry` file
pio run -e gsw_telem_info_generator
rm -f telemetry
.pio/build/gsw_telem_info_generator/program telemetry
