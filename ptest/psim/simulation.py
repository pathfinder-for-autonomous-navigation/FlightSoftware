# Nathan Zimmerberg, Tanishq Aggarwal
# 9.14.2019
# simulation.py
# Class to run a simulation and communicate with the flight computers.

import time, timeit
import platform
import threading
import os
import datetime
import os
from ..gpstime import GPSTime
import psim # woo!

class Simulation(object):
    """
    Full mission simulation, including both spacecraft.
    """
    def __init__(self, is_interactive, devices, seed, testcase, sim_duration, sim_initial_state, is_single_sat_sim):
        """
        Initializes self

        Args:
            devices: Connected Teensy devices that are controllable
            seed(int or None) random number generator seed or None
            print_log: If true, prints logging messages to the console rather than
                       just to a file.
        """
        self.is_interactive = is_interactive
        self.devices = devices
        self.seed = seed
        self.testcase = testcase
        self.sim_duration = sim_duration
        self.sim_initial_state = sim_initial_state
        self.is_single_sat_sim = is_single_sat_sim

        self.log = ""

        self.sim_time = 0

        if self.is_single_sat_sim:
            self.flight_controller = self.devices['FlightController']
        else:
            self.flight_controller_leader = self.devices['FlightControllerLeader']
            self.flight_controller_follower = self.devices['FlightControllerFollower']

        self.add_to_log("Configuring simulation (please be patient)...")
        start_time = timeit.default_timer()
        self.running = True
        self.configure()
        elapsed_time = timeit.default_timer() - start_time
        self.add_to_log("Configuring simulation took %0.2fs." % elapsed_time)

    def start(self):
        '''
        Start the MATLAB simulation. This function is blocking until the simulation begins.
        '''
        self.add_to_log("Starting simulation loop...")
        if self.is_interactive:
            self.sim_thread = threading.Thread(name="Simulation Interface",
                                        target=self.run)
            self.sim_thread.start()
        else:
            self.run()

    def add_to_log(self, msg):
        print(msg)
        self.log += f"[{datetime.datetime.now()}] {msg}\n"

    def configure(self):
        """
        Initialize anything required for the sim.
        """
        raise NotImplementedError

    def update_sensors(self):
        """
        Read sensors from simulation.
        """
        raise NotImplementedError

    def update_dynamics(self):
        """
        Allow simulation to step forward in time and update its
        truth.
        """
        raise NotImplementedError

    def simulate_flight_computers(self):
        """
        For any flight computer simulations within the simulation,
        update their data.

        This is useful if the flight computer has not yet implemented
        certain controllers, but this function should eventually be
        removable from the simulation interface.
        """
        raise NotImplementedError

    def send_actuations_to_simmed_satellites(self):
        """
        Send actuator commands from the real flight computer to the
        simulation so that it can update dynamics.
        """
        raise NotImplementedError

    def run(self):
        """
        Runs the simulation for the time interval specified in start().
        """

        if self.sim_duration != float("inf"):
            num_steps = int(self.sim_duration / self.dt) 
        else:
            num_steps = float("inf")
        step = 0

        start_time = time.time()
        while step < num_steps and self.running:
            self.update_dynamics() # what is the order?
            self.update_sensors() # what is the order?
            self.simulate_flight_computers()

            # Step 3.2. Send sim inputs, read sim outputs from Flight Computer
            self.interact_fc()
            # Step 3.3. Allow test case to do its own meddling with the flight computer.
            self.testcase.run_case()

            # Step 3.4. Step the flight computer forward.
            if self.is_single_sat_sim:
                self.flight_controller.write_state("cycle.start", "true")
            else:
                self.flight_controller_follower.write_state("cycle.start", "true")
                self.flight_controller_leader.write_state("cycle.start", "true")

            self.send_actuations_to_simmed_satellites()

            step += 1
            time.sleep(self.dt - ((time.time() - start_time) % self.dt))

        self.running = False
        self.add_to_log("Simulation ended.")
        self.eng.quit()

    def interact_fc(self):
        if self.is_single_sat_sim:
            self.interact_fc_onesat(self.flight_controller, self.sensor_readings_follower)
        else:
            self.interact_fc_onesat(self.flight_controller_follower, self.sensor_readings_follower)
            self.interact_fc_onesat(self.flight_controller_leader, self.sensor_readings_leader)

    def interact_fc_onesat(self, fc, sensor_readings):
        """
        Exchange simulation state variables with the one of the flight controllers.
        """
        # Step 3.2.2 Send inputs to Flight Controller
        self.write_adcs_estimator_inputs(fc, sensor_readings)
        # Step 3.2.3 Read outputs from previous control cycle
        self.read_adcs_estimator_outputs(fc)
        
        # Step 3.2.4
        self.read_actuators(fc)

    def write_adcs_estimator_inputs(self, flight_controller, sensor_readings):
        """Write the inputs required for ADCS state estimation."""

        # Convert mission time to GPS time
        current_gps_time = GPSTime(self.sim_time)

        # Clean up sensor readings to be in a format usable by Flight Software
        position_ecef = ",".join(["%.9f" % x[0] for x in sensor_readings["position_ecef"]])
        velocity_ecef = ",".join(["%.9f" % x[0] for x in sensor_readings["velocity_ecef"]])
        sat2sun_body = ",".join(["%.9f" % x[0] for x in sensor_readings["sat2sun_body"]])
        magnetometer_body = ",".join(["%.9f" % x[0] for x in sensor_readings["magnetometer_body"]])
        w_body = ",".join(["%.9f" % x[0] for x in sensor_readings["gyro_body"]])

        # Send values to flight software
        flight_controller.write_state("orbit.time", current_gps_time.to_seconds())
        flight_controller.write_state("orbit.pos", position_ecef)
        flight_controller.write_state("orbit.vel", velocity_ecef)
        flight_controller.write_state("adcs_monitor.ssa_vec", sat2sun_body)
        flight_controller.write_state("adcs_monitor.mag1_vec", magnetometer_body)
        flight_controller.write_state("adcs_monitor.gyr_vec", w_body)
    
    def read_adcs_estimator_outputs(self, flight_controller):
        """
        Read and store estimates from the ADCS estimator onboard flight software.

        The estimates are automatically stored in the Flight Controller telemetry log
        by calling read_state.
        """

        flight_Controller.read_state("pan.state")
        flight_Controller.read_state("pan.ccno")
        flight_Controller.read_state("adcs.state")        
        flight_controller.read_state("adcs_monitor.mag1_vec")
        flight_controller.read_state("adcs_monitor.mag2_vec")
        flight_controller.read_state("attitude_estimator.q_body_eci")
        flight_controller.read_state("attitude_estimator.w_body")
        flight_controller.read_state("attitude_estimator.fro_P")

    def read_actuators(self, fc):
        # "no" i dont care about matlab
        self.actuator_commands_follower["mtr_cmd"] = fc.rs("adcs_cmd.mtr_cmd")
        self.actuator_commands_follower["rwa_torque_cmd"] = fc.rs("adcs_cmd.rwa_torque_cmd")

    def stop(self, data_dir):
        """
        Stops a run of the simulation and saves run data to disk.
        """
        self.add_to_log("Stopping simulation...")
        self.running = False
        time.sleep(1) # Wait for logs to finish for the current timestep.

        with open(data_dir + "/simulation_log.txt", "w") as fp:
            fp.write(self.log)


class CppSimulation(Simulation):
    # good god i hate matlab

    def configure(self):
        self.actuator_commands_follower = {}
        # self.actuator_commands_follower["mtr_cmd"] = None
        self.mysim = psim.SimulationRunner 

    def update_sensors(self):
        psimsim.get()
        raise NotImplementedError

    def update_dynamics(self):
        """
        Allow simulation to step forward in time and update its
        truth.
        """
        raise NotImplementedError

    def simulate_flight_computers(self):
        # we're all grown up now, don't need this
        pass 

    def send_actuations_to_simmed_satellites(self):
        # send the outputs of the FC to psim
        self.mysim["adcs_cmd.mtr_cmd"] = self.flight
        raise NotImplementedError

    pass


class MatlabSimulation(Simulation):
    def configure(self):
        import matlab.engine

        self.eng = matlab.engine.start_matlab()
        path = os.path.join(
            os.path.dirname(os.path.abspath(__file__)), "../../lib/common/psim/MATLAB")
        self.eng.addpath(path, nargout=0)

        if ((platform.system() == 'Darwin' and not os.path.exists("geograv_wrapper.mexmaci64"))
            or (platform.system() == 'Linux' and not os.path.exists("geograv_wrapper.mexa64"))
            or (platform.system() == 'Windows' and not os.path.exists("geograv_wrapper.mexw64"))
        ):
            self.eng.install(nargout=0)
        self.eng.config(nargout=0)
        self.eng.generate_mex_code(nargout=0)
        self.eng.eval("global const", nargout=0)

        self.main_state = self.eng.initialize_main_state(self.seed, self.sim_initial_state, nargout=1)
        self.computer_state_follower, self.computer_state_leader = self.eng.initialize_computer_states(self.sim_initial_state, nargout=2)

        self.eng.workspace['const']['dt'] = self.flight_controller.smart_read("pan.cc_ms") * 1e6  # Control cycle time in nanoseconds.
        self.dt = self.eng.workspace['const']['dt'] * 1e-9 # In seconds

    def update_sensors(self):
        self.sensor_readings_follower = self.eng.sensor_reading(self.main_state['follower'],self.main_state['leader'], nargout=1)
        self.sensor_readings_leader = self.eng.sensor_reading(self.main_state['leader'],self.main_state['follower'], nargout=1)

    def update_dynamics(self):
        self.main_state_promise = self.eng.main_state_update(self.main_state, nargout=1, background=True)

    def simulate_flight_computers(self):
        self.computer_state_follower, self.actuator_commands_follower = \
            self.eng.update_FC_state(self.computer_state_follower,self.sensor_readings_follower, nargout=2)
        self.computer_state_leader, self.actuator_commands_leader = \
            self.eng.update_FC_state(self.computer_state_leader,self.sensor_readings_leader, nargout=2)

    def send_actuations_to_simmed_satellites(self):
        self.main_state = self.main_state_promise.result()
        self.main_state['follower'] = self.eng.actuator_command(self.actuator_commands_follower,self.main_state['follower'], nargout=1)
        self.main_state['leader'] = self.eng.actuator_command(self.actuator_commands_leader,self.main_state['leader'], nargout=1)
