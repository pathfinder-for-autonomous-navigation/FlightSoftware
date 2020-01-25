#!/bin/bash

set -e # Exit on any error

# Verify compilation and unit tests for CI environment
platformio test -e gsw_downlink_parser_ci -v

# Check for memory mismanagement
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose .pio/build/gsw_downlink_parser_ci/program

# Compile and run functional test for environments
platformio run -e gsw_downlink_parser
python -m unittest test.test_downlink_parser

# Check that the telemetry info generator compiles, passes functional test, and Valgrind
#platformio run -e telem_info_generator
#python -m unittest test.test_telem_info_generator
#valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose .pio/build/telem_info_generator/program x.json
#rm x.json