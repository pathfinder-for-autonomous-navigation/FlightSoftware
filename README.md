# FlightSoftware

[![Build Status](https://travis-ci.org/pathfinder-for-autonomous-navigation/FlightSoftware.svg?branch=master)](https://travis-ci.org/pathfinder-for-autonomous-navigation/FlightSoftware)
[![Coverage Status](https://coveralls.io/repos/github/pathfinder-for-autonomous-navigation/FlightSoftware/badge.svg?branch=master)](https://coveralls.io/github/pathfinder-for-autonomous-navigation/FlightSoftware?branch=master)

Repository for all Teensy based flight code for Pathfinder for Autonomous Navigation.

## Running tests with Docker (FlightSoftware)

    docker build -t fsw .
    docker run fsw ./tools/run_desktop_flight_tests.sh
    docker run fsw ./tools/run_ground_tests.sh

See the "Tools" directory to see how to run tests locally without Docker.

## Locally running flight software unit tests (quickly)
     pio test -e fsw_native_leader

 
  Change-directory into the ADCS Software folder and work from there.		 ## Compiling flight software for functional testing

  Here are some possible options:

      pio run -e fsw_native_leader (for HOOTL testing)
     pio run -e fsw_teeny35_hitl_leader (for HITL testing with a Teensy 3.5)
     pio run -e fsw_teeny36_hitl_leader (for HITL testing with a Teensy 3.6)
     pio run -e fsw_flight_leader (for HITL testing with pure flight code)

## Testing ADCS software
    Check out `configs/adcs.ini` for a list of available environments.
