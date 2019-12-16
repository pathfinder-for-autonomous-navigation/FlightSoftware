#!/bin/bash

set -e # Exit on any error

# Verify compilation and unit tests for CI environment
platformio test -e native_ci -v

# Check for memory mismanagement
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose .pio/build/native_ci/program

# Compile and run functional test for environments
platformio run -e native_ci
platformio run -e native

python -m unittest test.test_dummy_fsw
python -m unittest test.test_real_fsw
