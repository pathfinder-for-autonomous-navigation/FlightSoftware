# FlightSoftware

![HOOTL Tests](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/actions/workflows/hootl.yml/badge.svg?branch=master)
![Unit Tests](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/actions/workflows/unit_tests.yml/badge.svg?branch=master)

Repository for all flight software, ground software, and test infastructure for Pathfinder for Autonomous Navigation.
PAN's simulation software lives in [psim](https://github.com/pathfinder-for-autonomous-navigation/psim).

## Building and compiling this repo

1. Clone the repository:

        git clone --recursive git@github.com:pathfinder-for-autonomous-navigation/FlightSoftware.git
        cd FlightSoftware

2. Create a virtual environment, install requirements, and install `psim`:

        python -m venv venv
        source venv/bin/activate
        pip install --upgrade pip wheel
        pip install -r requirements.txt
        pip install -e lib/common/psim

## Locally running flight software unit tests (quickly)

    pio test -e fsw_native_leader
  
You can modify the .ini files in configs/ to alter the pattern match against unit tests
in order to run unit tests selectively.

## Compiling and loading flight software for functional testing

Here are some possible options:

    pio run -e fsw_native_leader                 (for HOOTL testing)
    pio run -e fsw_teensy35_hitl_leader -t upload (for HITL testing with a Teensy 3.5)
    pio run -e fsw_teensy36_hitl_leader -t upload (for HITL testing with a Teensy 3.6)
    pio run -e fsw_flight_leader -t upload       (for HITL testing with pure flight code)

The binaries end up in `.pio/build`.

## Running functional tests with `ptest`

Install `ptest` via the guide in the README in [ptest](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/tree/master/ptest), and then run from this directory, and with a virtual environment activated:

      python -m ptest runsim -c CONFIGFILE -t TESTCASE

## Testing ADCS software

Check out `configs/adcs.ini` for a list of available environments.
