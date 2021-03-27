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
import psim
import lin 
import json
from ..cases.utils import is_lin_vector, to_lin_vector

class Simulation(object):
    """
    Full mission simulation, including both spacecraft.
    """
    def __init__(self, is_interactive, devices, seed, testcase, sim_duration, 
    sim_initial_state, is_single_sat_sim, _sim_configs, _sim_model, _mapping_file_name, device_config):
        """
        Initializes self

        Args:
            devices: Connected Teensy devices that are controllable
            seed(int or None) random number generator seed or None
            print_log: If true, prints logging messages to the console rather than
                       just to a file.
            _sim_configs = List of psim configs
            _sim_modle = The desired psim model
            _mappings_file_name = The json file containing the mappings of fc to psim statefields
        """
        self.is_interactive = is_interactive
        self.devices = devices
        self.seed = seed
        self.testcase = testcase
        self.sim_duration = sim_duration
        self.sim_initial_state = sim_initial_state
        self.is_single_sat_sim = is_single_sat_sim
        self.sim_configs = _sim_configs
        self.sim_model = _sim_model
        self.mapping_file_name = _mapping_file_name
        self.scrape_emails = scrape_emails
        self.log = ""

        # if the json config has devices, and the string 'autotelem' is somewhere in the dictionary
        if device_config != None and 'autotelem' in str(device_config):
            self.add_to_log('[PTEST-SIM] Autotelem ACTIVE!')
            self.enable_autotelem = True
        else:
            self.add_to_log('[PTEST-SIM] Autotelem INACTIVE!')
            self.enable_autotelem = False

        if self.is_single_sat_sim:
            self.add_to_log('[PTEST-SIM] Singlesat sim!')
            self.flight_controller = self.devices['FlightController']
        elif self.devices:
            self.add_to_log('[PTEST-SIM] Dualsat sim!')
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
        Start the PSim C++ simulation. This function is blocking until the simulation begins.
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

    def read_actuators_send_to_sim(self):
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
            # Step 1. Generate dynamics
            self.update_dynamics()

            # Step 2. Load sensor data into ptest
            self.update_sensors()
                        
            # Step 3.2. Send sim inputs, read sim outputs from Flight Computer
            self.interact_fc()

            # Step 3 Simulate Flight Computers if need be
            self.simulate_flight_computers()

            # Step 3.3. Allow test case to do its own meddling with the flight computer.
            self.testcase.run_case()

            # Step 3.4. Step the flight computer forward.
            if self.is_single_sat_sim:
                self.flight_controller.write_state("cycle.start", "true")
            else:
                self.flight_controller_follower.write_state("cycle.start", "true")
                self.flight_controller_leader.write_state("cycle.start", "true")

            # Step 4. Send telemetry to database
            if self.enable_autotelem:
                if self.is_single_sat_sim:
                    self.flight_controller.dbtelem()
                else:
                    self.flight_controller_follower.dbtelem()
                    self.flight_controller_leader.dbtelem()

            # Step 5. Read the actuators from the flight computer(s) and send to psim
            self.read_actuators_send_to_sim()

            # Step 6. Read incoming uplinks
            for device in self.devices:
                if self.devices[device][device]["scrape_uplinks"]:
                    self.devices[device].scrape_uplink()

            step += 1

        self.running = False
        self.add_to_log("Simulation ended.")

    def interact_fc(self):
        if self.is_single_sat_sim:
            self.interact_fc_onesat(self.flight_controller)
        elif self.devices:
            self.interact_fc_onesat(self.flight_controller_follower)
            self.interact_fc_onesat(self.flight_controller_leader)

    def interact_fc_onesat(self, fc):
        """
        Exchange simulation state variables with the one of the flight controllers.
        """
        # Step 3.2.1 Send inputs to Flight Controller
        self.write_adcs_estimator_inputs(fc)

        # Step 3.2.2 Read outputs from previous control cycle
        self.read_adcs_estimator_outputs(fc)
        
        # Step 3.2.3
        self.read_actuators(fc)
    
    def read_actuators(self, fc):
        raise NotImplementedError

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
        prefix = "lib/common/psim/config/parameters/"
        postfix = ".txt"

        if self.sim_configs == []:
            raise RuntimeError("No simulation configs were provided! Please set the sim_configs property")
        if self.sim_model == None:
            raise RuntimeError("No simulation models were provided! Please set the sim_model property")
        if self.mapping_file_name == "":
            raise RuntimeError("Error. Please set the json mapping file name property")

        configs_list = [prefix + x + postfix for x in self.sim_configs]
        config = psim.Configuration(configs_list)
        
        self.add_to_log("[ sim ] Overwriting Initial Sim Conditions...")

        # get the mutation dict from test case
        initials = self.testcase.sim_ic_map

        # mutate config
        for k,v in initials.items():
            self.add_to_log(f"[ sim ] Set {k} to {v}")            
            if type(v) == list:
                v = to_lin_vector(v)
            config[k] = v

        # construct sim
        self.mysim = psim.Simulation(self.sim_model, config)
        self.dt = self.mysim["truth.dt.ns"]/1e9

        self.sat_names = ['leader','follower']
        if self.is_single_sat_sim:
            self.sat_names = ['leader']

        # Open the correct mapping config file. self.mapping_file_name is a property of the ptest case
        self.mapping_file_name
        fn = 'ptest/psim/mapping_configs/' + self.mapping_file_name
        with open(fn) as json_file:
            self.fc_vs_sim = json.load(json_file)

        # Create sub dictionaries for sensors and actuators
        self.fc_vs_sim_s = self.fc_vs_sim['fc_vs_sim_s']
        self.fc_vs_sim_a = self.fc_vs_sim['fc_vs_sim_a']

        # Create a dictionary of satellite names to FC-to-psim state field mappings
        self.sensors_map = {k:self.fc_vs_sim_s for k in self.sat_names}
        self.actuators_map = {k:self.fc_vs_sim_a for k in self.sat_names}

        # replace sat with the real satellite name in all the mappings
        self.sensors_map =   {k:{f_name:p_name.replace('sat',k) for f_name,p_name in v.items()} for k,v in self.sensors_map.items()}
        self.actuators_map = {k:{f_name:p_name.replace('sat',k) for f_name,p_name in v.items()} for k,v in self.actuators_map.items()}
        
        # Now the above is a dictionary as follows:
        # sensors_map['leader']['fsw_sensor_sf_name'] = 'psim_leader_sensor_sf_name'
        # actuators_map['leader']['fsw_act_sf_name'] = 'psim_leader_act_sf_name'

        # Initialize sensor reading and actuator reading containers
        self.sensor_readings = {k:{} for k in self.sat_names}
        self.actuator_cmds = {k:{} for k in self.sat_names}

        self.fc_to_role_map = {'FlightController':'leader', 'FlightControllerLeader':'leader', 
                               'FlightControllerFollower':'follower'}

    def update_sensors(self):
        '''
        For each active satelite in the sensors_map, get all the psim values from psim
        '''
        
        # iterate across each satellite's mappings
        for role,mappings in self.sensors_map.items():
            # iterate across each fc_sf vs psim_sf pair
            for fc_sf,psim_sf in mappings.items():
                psim_val = self.mysim[psim_sf]
                if(type(psim_val) in {lin.Vector2, lin.Vector3, lin.Vector4}):
                    psim_val = list(psim_val)
                self.sensor_readings[role][fc_sf] = psim_val

    def write_adcs_estimator_inputs(self, fc):
        """Write the inputs required for ADCS state estimation. Per satellite"""

        role = self.fc_to_role_map[fc.device_name]

        mappings = self.sensors_map[role]
        # iterate across each fc_sf vs psim_sf pair
        for fc_sf,psim_sf in mappings.items():
            local = self.sensor_readings[role][fc_sf]
            fc.write_state(fc_sf, local)

    def read_adcs_estimator_outputs(self, flight_controller):
        """
        Read and store estimates from the ADCS estimator onboard flight software.

        The estimates are automatically stored in the Flight Controller telemetry log
        by calling read_state.
        """

        flight_controller.read_state("pan.state")
        flight_controller.read_state("adcs.state")        
        flight_controller.read_state("adcs_monitor.mag1_vec")
        flight_controller.read_state("adcs_monitor.mag2_vec")
        flight_controller.read_state("attitude_estimator.q_body_eci")
        flight_controller.read_state("attitude_estimator.w_body")
        flight_controller.read_state("attitude_estimator.fro_P")

    def update_dynamics(self):
        """
        Allow simulation to step forward in time and update its
        truth.
        """
        self.mysim.step()

    def simulate_flight_computers(self):
        # we're all grown up now, don't need this
        pass 

    def read_actuators(self, fc):
        role = self.fc_to_role_map[fc.device_name]
        for fc_sf in self.actuators_map[role]:
            self.actuator_cmds[role][fc_sf] = fc.smart_read(fc_sf)

        '''
        Uncomment the below sections if you want to mutate the actuators coming out of the flight computer
        for debugging purposes.

        Adjusting the yeet factor (yf) is a direct multiplier to make actuators stronger than they actually are
        Setting it to 0 mutes the actuator. It is useful to temporarily disable actuators for debugging purposes
        to isolate parts of the controller. Largely increasing the strength of actuators is also great to check
        that an actuator really is causing an effect on dynamics (on a much shorter time scale).
        '''

        # mtr_yf = 10
        # self.actuator_cmds[role]["adcs_cmd.mtr_cmd"] = [x*mtr_yf for x in self.actuator_cmds[role]["adcs_cmd.mtr_cmd"]]
        
        # rwa_t_yf = 10
        # self.actuator_cmds[role]["adcs_cmd.rwa_torque_cmd"] = [x*rwa_t_yf for x in self.actuator_cmds[role]["adcs_cmd.rwa_torque_cmd"]]

    def read_actuators_send_to_sim(self):
        
        # send the outputs of the FC to psim        
        # iterate across each satellite's mappings
        for role,mappings in self.actuators_map.items():
            # iterate across each fc_sf vs psim_sf pair
            for fc_sf,psim_sf in mappings.items():
                local = self.actuator_cmds[role][fc_sf]
                if type(local) == list:
                    _len = len(local)
                    if _len == 2:
                        local = lin.Vector2(local)
                    elif _len == 3:
                        local = lin.Vector3(local)
                    elif _len == 4:
                        local = lin.Vector4(local)
                    else:
                        raise RuntimeError("Unexpected List Length, can't change into lin Vector")

                self.mysim[psim_sf] = local
