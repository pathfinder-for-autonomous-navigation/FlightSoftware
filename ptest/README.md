# Running unit tests

From the **root flight software folder**, run

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
    the absolute path to this repository. For windows, use `PATH_TO_FLIGHT_SOFTWARE/venv/Scripts/python`.
    For Mac and Linux, use: `PATH_TO_FLIGHT_SOFTWARE/venv/bin/python`.

    Example Linux Output (Matlab Terminal):

         >> pyversion '/home/shihao/Code/PAN/FlightSoftware/venv/bin/python'
         >> pyversion

               version: '3.6'
            executable: '/home/shihao/Code/PAN/FlightSoftware/venv/bin/python'
               library: 'libpython3.6m.so.1.0'
                  home: '/home/shihao/Code/PAN/FlightSoftware/venv'
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

    python -m ptest runsim -c ptest/configs/fc_only_teensy.json

**Running with software only**

This only works for Mac and Linux for now.

Edit `ptest/configs/fc_only.json` so that the `pio_target` for the Flight Controller points to the desired flight
software binary.

# Running simulation
1. Activate the virtualenv as done in step 2 of the Installation above. You do not need to reinstall the venv.
    - For Mac and Linux: `source venv/bin/activate`
    - For Windows: `venv\Scripts\activate`

2. Then, run the main script. On Windows you must run

       python -m ptest runsim -c ptest/configs/fc_only_teensy.json -t TESTCASENAME

      since psim for Windows currently only supports connections to actual Teensy devices. On Mac you may run

       python -m ptest runsim -c ptest/configs/fc_only_native.json -t TESTCASENAME

      To connect to a desktop binary. Recommended testcase names are `EmptySimCase` and `EmptyCase`.

3. If you'd like to run more complex configurations, see the examples in the folder `ptest/configs`.
