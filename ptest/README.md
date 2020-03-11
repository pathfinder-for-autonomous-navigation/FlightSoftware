# psim Python Interface

This folder will house all python code in this repository. Functionality to be
supported by Python may include, but is not limited to:

 * Running a simulation in real time with one or two satellites.
 * Real time graphing utilities and other data visulation tools to be used in
   conjunction with a real time simulation. To use this utility run

       python -m ptest.plotter -d /path/to/data/file

   after running the installation steps below. The `/path/to/data/file` needs to be
   produced by the simulation.

 * Hardware/Sim interface for SHITLs.

# Running unit tests

From the root flight software folder, run

    python -m pytest ptest

# Installing

To install the Python MATLAB engine, you must:

 1. Install Python 3.6 - it's what we'll be using as it's the newest version
    officially supported by MATLAB.
 2. Create and activate a virtualenv **within the root FlightSoftware directory**.
    - For Mac and Linux: `python3 -m virtualenv venv; source venv/bin/activate`
    - For Windows: `python -m virtualenv venv`, followed by `venv\Scripts\activate`.
 3. Install the requirements: `pip install -r requirements.txt`
 4. If on Mac, separately install `readline` via `pip`.
 5. I found that I required MATLAB R2019b in order for the simulation to work. Make sure you
    have this version.
 6. Set MATLAB's `pyversion` variable to link with the newly installed Python 3.6.
    You set and check the Python interpretter used by MATLAB by entering
    something like the following in MATLAB's terminal. Replace `PATH_TO_FLIGHT_SOFTWARE` with 
    the absolute path to this repository. For windows, use `PATH_TO_FLIGHT_SOFTWARE/python/venv/Scripts/python`.

        >> pyversion PATH_TO_FLIGHT_SOFTWARE/python/venv/bin/python3
        >> pyversion

            version: '3.6'
            executable: '/Users/tanishqaggarwal/Documents/pan/repositories/psim/python/venv/bin/python3'
            library: ''
            home: '/Users/tanishqaggarwal/Documents/pan/repositories/psim/python/venv/bin/..'
            isloaded: 0

 7. Install MATLAB for this repository. With the venv activated from the FlightSoftware directory, run the following on macOS:

         cd /Applications/MATLAB_R2019b.app/extern/engines/python
         python setup.py install --prefix="PATH_TO_FLIGHT_SOFTWARE/venv"

      or on Linux:

         cd /PATH_TO_MATLAB/R2019b/extern/engines/python
         python3 setup.py install --prefix="PATH_TO_FLIGHT_SOFTWARE/venv"

      or on Windows:

         cd C:\Program Files\MATLAB\R2019b\extern\engines\python
         python setup.py install --prefix="PATH_TO_FLIGHT_SOFTWARE/venv"


# Configuring Simulation

**Running with Teensy in the loop**

Open the config file:

    psim/python/ptest/configs

Change the port to the COM port Teensy is connected to

Run the main script:

    python -m ptest.run_simulation -c ptest/configs/fc_only_teensy.json

**Running with software only**

This only works for Mac and Linux for now.

Edit `ptest/configs/fc_only.json` so that the `binary_filepath` for the Flight Controller points to a binary built
for the Flight Software. You can find these binaries [here](https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/releases).

# Running simulation
1. Activate the virtualenv as done in step 2 of the Installation above. You do not need to reinstall the venv.
    - For Mac and Linux: `source venv/bin/activate`
    - For Windows: `venv\Scripts\activate`

2. Then, run the main script. On Windows you must run

       python -m ptest.run_simulation -c ptest/configs/fc_only_teensy.json -t TESTCASENAME

      since psim for Windows currently only supports connections to actual Teensy devices. On Mac you may run

       python -m ptest.run_simulation -c ptest/configs/fc_only_native.json -t TESTCASENAME

      To connect to a desktop binary. Recommended testcase names are `EmptySimCase` and `EmptyCase`.

3. If you'd like to run more complex configurations, see the examples in the folder `ptest/configs`.
