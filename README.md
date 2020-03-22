# FlightSoftware

[![Build Status](https://travis-ci.org/pathfinder-for-autonomous-navigation/FlightSoftware.svg?branch=master)](https://travis-ci.org/pathfinder-for-autonomous-navigation/FlightSoftware)
[![Coverage Status](https://coveralls.io/repos/github/pathfinder-for-autonomous-navigation/FlightSoftware/badge.svg?branch=master)](https://coveralls.io/github/pathfinder-for-autonomous-navigation/FlightSoftware?branch=master)

Repository for all Teensy based flight code for Pathfinder for Autonomous Navigation.

## Running tests with Docker (FlightSoftware)

    docker build -t fsw .
    docker run fsw ./tools/run_desktop_flight_tests.sh
    docker run fsw ./tools/run_ground_tests.sh

See the "Tools" directory to see how to run tests locally without Docker.

## Working with ADCS Software

Change-directory into the ADCS Software folder and work from there.
