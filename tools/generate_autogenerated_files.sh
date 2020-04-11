# Run this file prior to opening a PR, so that your code will not fail
# on the file diff checks in tools/run_ground_tests.sh

# Generate `telemetry` file
pio run -e gsw_telem_info_generator
.pio/build/gsw_telem_info_generator/program telemetry

# Generate `constants` file
python tools/constant_reporter.py

# Generate flow_data.cpp
python src/flow_data_generator.py
