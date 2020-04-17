#!/bin/bash

set -e # Exit on any error

# Verify compilation and unit tests for CI environment
platformio test -e gsw_downlink_parser_ci -v

# Compile and run functional test for environments
#platformio run -e gsw_downlink_parser
#python -m unittest test.test_downlink_parser
# TEMPORARILY DISABLED DOWNLINK PARSER TESTING until we have better tools
# for writing and updating this test when the flows change.

# Check that the telemetry info generator compiles, passes functional test, and Valgrind
platformio run -e gsw_telem_info_generator
python -m unittest test.test_telem_info_generator
.pio/build/gsw_telem_info_generator/program expected_telemetry

# Ensure that produced telemetry info and telemetry info uploaded by the user are the same.
diff expected_telemetry telemetry

# Ensure that produced telemetry flows and the telemetry flows uploaded by the user are the same.
mv src/flow_data.cpp src/uploaded_flow_data.cpp
python src/flow_data_generator.py
diff src/flow_data.cpp src/uploaded_flow_data.cpp

# Ensure that constants across code and constants file uploaded by the user are the same.
mv constants uploaded_constants
python tools/constant_reporter.py
diff constants uploaded_constants

# Cleanup Directory
rm expected_telemetry
rm uploaded_constants