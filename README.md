# FlightSoftware

[![Build Status](https://travis-ci.org/pathfinder-for-autonomous-navigation/FlightSoftware.svg?branch=master)](https://travis-ci.org/pathfinder-for-autonomous-navigation/FlightSoftware)
[![Coverage Status](https://coveralls.io/repos/github/pathfinder-for-autonomous-navigation/FlightSoftware/badge.svg?branch=master)](https://coveralls.io/github/pathfinder-for-autonomous-navigation/FlightSoftware?branch=master)

Repository for all flight software, ground software, and test infastructure for Pathfinder for Autonomous Navigation.
PAN's simulation software lives in [psim](https://github.com/pathfinder-for-autonomous-navigation/psim).

## Running tests with Docker (FlightSoftware)
    docker build -t fsw .
    docker run fsw ./tools/run_desktop_flight_tests.sh
    docker run fsw ./tools/run_common_software_tests.sh
    docker run fsw ./tools/run_ground_tests.sh
    docker run fsw ./tools/verify_adcs_builds.sh
    docker run fsw ./tools/verify_teensy_builds.sh

See the "Tools" directory to see how to run tests locally without Docker, and for many
other useful items.

## Locally running flight software unit tests (quickly)
    pio test -e fsw_native_leader
  
You can modify the .ini files in configs/ to alter the pattern match against unit tests
in order to run unit tests selectively.

## Compiling and loading flight software for functional testing

Here are some possible options:

    pio run -e fsw_native_leader                 (for HOOTL testing)
    pio run -e fsw_teeny35_hitl_leader -t upload (for HITL testing with a Teensy 3.5)
    pio run -e fsw_teeny36_hitl_leader -t upload (for HITL testing with a Teensy 3.6)
    pio run -e fsw_flight_leader -t upload       (for HITL testing with pure flight code)

The binaries end up in `.pio/build`.

## Running functional tests with `ptest`

Install `ptest` via the guide in the README in [ptest](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/tree/master/ptest), and then run from this directory, and with a virtual environment activated:

      python -m ptest runsim -c CONFIGFILE -t TESTCASE

## Testing ADCS software

Check out `configs/adcs.ini` for a list of available environments.
