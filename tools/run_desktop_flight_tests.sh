#!/bin/bash

set -e # Exit on any error

# Verify compilation and unit tests for CI environment
platformio test -e fsw_native_ci -v

# Compile and run functional test for environments
platformio run -e fsw_native_ci
python -m unittest test.test_dummy_fsw

platformio run -e fsw_native_leader
python -m unittest test.test_real_fsw
