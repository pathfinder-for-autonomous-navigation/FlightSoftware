# Nathan Zimmerberg, Tanishq Aggarwal
# 9.14.2019
# Shihao Cao, Kyle Krol
# 04.12.2021
# psim_case.py
# Class to run a simulation and communicate with the flight computers.

import time, timeit
import platform
import threading
import os
import datetime
import os
from ...gpstime import GPSTime
import psim
import lin 
import json
from ..utils import is_lin_vector, to_lin_vector,Enums
import math
from .ptest_case import PTestCase
from .single_sat_case import SingleSatCase

class PSimCase(PTestCase):
    """Base class for testcases running with a PSim simulation.

    Attributes:
        psim_config_override
        psim_config_prefix
        psim_config_suffix
        psim_configs
    """

    def __init__(self, *args, **kwargs):
        """
        """
        super(PSimCase, self).__init__(*args, **kwargs)

        self.psim_config_overrides = dict()
        self.psim_config_prefix = 'lib/common/psim/config/parameters/'
        self.psim_config_suffix = '.txt'
        self.psim_configs = [
            'truth/base',
            'sensors/base'
        ]
        
        self.psim_mode = None
        self.is_single_sat_sim = None

        if isinstance(self, SingleSatCase):
            self.is_single_sat_sim = True
            from psim.sims import SingleAttitudeOrbitGnc
            self.psim_model = SingleAttitudeOrbitGnc
        else:
            self.is_single_sat_sim = False
            from psim.sims import DualAttitudeOrbitGnc
            self.psim_model = DualAttitudeOrbitGnc

        
    def setup(self, *args, **kwargs):
        """
        """
        super(PSimCase, self).setup(*args, **kwargs)

        configs = [self.psim_config_prefix + config + self.psim_config_suffix for config in self.psim_configs]
        config = psim.Configuration(configs)
        for name, param in self.psim_config_overrides.items():
            config[name] = param

        self.__sim = self.psim_model(config)

        """
        Initializes self

        Args:
            devices: Connected Teensy devices that are controllable
            seed(int or None) random number generator seed or None
            print_log: If true, prints logging messages to the console rather than
                       just to a file.
            _sim_configs = List of psim configs
            _sim_modle = The desired psim model
        """

        '''
        If this member variable is true, then we will attempt to populate a 
        set of sensor validity fields based off of psim, as well as just purely
        setting them to be "working just fine"
        '''
        self.mock_sensor_validity = True

        # # if the json config has devices, and the string 'autotelem' is somewhere in the dictionary
        # if self.device_config != None and 'autotelem' in str(self.device_config):
        #     print('[PTEST-SIM] Autotelem ACTIVE!')
        #     self.enable_autotelem = True
        # else:
        #     print('[PTEST-SIM] Autotelem INACTIVE!')
        #     self.enable_autotelem = False

        # if self.is_single_sat_sim:
        #     print('[PTEST-SIM] Singlesat sim!')
        #     self.flight_controller = self.devices['FlightController']
        # elif self.devices:
        #     print('[PTEST-SIM] Dualsat sim!')
        #     self.flight_controller_leader = self.devices['FlightControllerLeader']
        #     self.flight_controller_follower = self.devices['FlightControllerFollower']

        print("Configuring simulation (please be patient)...")
        start_time = timeit.default_timer()
        self.running = True
        self.configure()
        elapsed_time = timeit.default_timer() - start_time
        print("Configuring simulation took %0.2fs." % elapsed_time)

    def cycle(self, *args, **kwargs):
        """
        """
        super(PSimCase, self).cycle(*args, **kwargs)
        
        for device in self.devices:
            self.read_actuators(device)
        
        # Step 5. Read the actuators from the flight computer(s) and send to psim
        self.read_actuators_send_to_sim()

        # Step 1. Generate dynamics
        self.update_dynamics()

        # Step 2. Load sensor data from psim into ptest
        self.update_sensors()
                    
        # Step 3.1 Mock sensor validity flags and states if requested
        if self.mock_sensor_validity:
            for device in self.devices:
                self.mock_piksi_state(device)
                self.mock_adcs_havt(device)
                self.mock_ssa_mode(device)

        for device in self.devices:
            self.transfer_piksi_time(device)

        ### BEGIN SECTION OF CODE FOR STATEFIELDS THAT ARE EASY TRANSFERS

        # Step 3.2. Send sim inputs, read sim outputs from Flight Computer
        for device in self.devices:
            self.write_adcs_estimator_inputs(device)

        # Step 3 Simulate Flight Computers if need be
        self.simulate_flight_computers()

        # # Step 3.3. Allow test case to do its own meddling with the flight computer.
        # self.testcase.run_case()

        # # Step 3.4. Step the flight computer forward.
        # if self.is_single_sat_sim:
        #     self.flight_controller.write_state("cycle.start", "true")
        # else:
        #     self.flight_controller_follower.write_state("cycle.start", "true")
        #     self.flight_controller_leader.write_state("cycle.start", "true")

        # # Step 4. Send telemetry to database
        # if self.enable_autotelem:
        #     if self.is_single_sat_sim:
        #         self.flight_controller.dbtelem()
        #     else:
        #         self.flight_controller_follower.dbtelem()
        #         self.flight_controller_leader.dbtelem()

        # # Step 6. Read incoming uplinks
        # for device in self.devices:
        #     if self.devices[device].scrape:
        #         self.devices[device].scrape_uplink()


    def rs_psim(self, name: str):
        '''
        Read a psim state field with <name>, log to datastore, and return the python value
        '''
        ret = self.__sim[name]

        stripped = ret
        if type(ret) in {lin.Vector2, lin.Vector3, lin.Vector4}:
            ret = list(ret)
            stripped = str(ret).strip("[]").replace(" ","")+","
        
        packet = {}
        
        packet["t"] = int(self.__sim["truth.t.ns"]/1e9/1e3) # t: number of ms since sim start
        packet["field"] = name
        packet["val"] = str(stripped)
        packet["time"] = str(datetime.datetime.now())

        # log to datastore
        for d in self.devices:
            d.datastore.put(packet)

        return ret

    def print_rs_psim(self, name):
        '''
        Read a psim state field with <name>, log to datastore, print to console and return the python value
        '''
        ret = self.psim_rs(name)
        self.logger.put(f"{name} is {ret}")
        
    def configure(self):
        self.actuator_commands_follower = {}
        prefix = "lib/common/psim/config/parameters/"
        postfix = ".txt"

        self.dt = self.__sim["truth.dt.ns"]/1e9

        self.sat_names = ['leader','follower']
        if self.is_single_sat_sim:
            self.sat_names = ['leader']
            
        self.fc_vs_sim = {
            "fc_vs_sim_s":{
                "piksi.pos": "truth.sat.orbit.r.ecef",
                "piksi.vel": "truth.sat.orbit.v.ecef",
                "adcs_monitor.ssa_vec":  "truth.sat.environment.s.body",
                "adcs_monitor.mag1_vec": "truth.sat.environment.b.body",
                "adcs_monitor.gyr_vec": "truth.sat.attitude.w",
                "adcs_monitor.rwa_speed_rd": "truth.sat.wheels.w"
            },
            "fc_vs_sim_a":{
                "adcs_cmd.mtr_cmd": "truth.sat.magnetorquers.m",
                "adcs_cmd.rwa_torque_cmd": "truth.sat.wheels.t"
            }
        }

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
                psim_val = self.__sim[psim_sf]
                if(type(psim_val) in {lin.Vector2, lin.Vector3, lin.Vector4}):
                    psim_val = list(psim_val)
                self.sensor_readings[role][fc_sf] = psim_val

    def mock_piksi_state(self, fc_device):
        '''
        Lets piksi state to be populated with either spp for fixed rtk
        based on a psim flag value
        '''
        fc_name = fc_device.device_name
        psim_sat_name = self.fc_to_role_map[fc_name]
        try:
            cdgps_active = self.__sim["sensors."+psim_sat_name+".cdgps.valid"]
        except RuntimeError:
            # sim does not support this field, probably a single sat setup
            cdgps_active = 0

        fsw_piksi_state = -1

        if cdgps_active == 0:
            fsw_piksi_state = Enums.piksi_modes['spp']
        else:
            fsw_piksi_state = Enums.piksi_modes['fixed_rtk']

        fc_device.write_state('piksi.state', fsw_piksi_state)

    def mock_adcs_havt(self, fc_device):
        '''
        Set the gyro, and both mags to be "working"
        '''
        fc_device.write_state('adcs_monitor.havt_device0', True)
        fc_device.write_state('adcs_monitor.havt_device1', True)
        fc_device.write_state('adcs_monitor.havt_device2', True)

    def mock_ssa_mode(self, fc_device):
        '''
        Lets ssa mode to be populated with either SSA_COMPLETE OR SSA_FAILURE
        based on a psim flag value        
        '''
        fc_name = fc_device.device_name
        psim_sat_name = self.fc_to_role_map[fc_name]
        ssa_vec = self.__sim[f"sensors.{psim_sat_name}.sun_sensors.s"]
        fsw_ssa_mode = -1

        # if any of ssa_vec is nan
        if any([math.isnan(x) for x in ssa_vec]):
            fsw_ssa_mode = Enums.ssa_modes['SSA_FAILURE']
        else:
            fsw_ssa_mode = Enums.ssa_modes['SSA_COMPLETE']

        fc_device.write_state('adcs_monitor.ssa_mode', fsw_ssa_mode)

    def transfer_piksi_time(self, fc_device):
        psim_time_ns = self.__sim['truth.t.ns']
        python_time = GPSTime(psim_time_ns)

        fc_device.write_state('piksi.time', python_time.to_list())

    def write_adcs_estimator_inputs(self, fc):
        """Write the inputs required for ADCS state estimation. Per satellite"""

        role = self.fc_to_role_map[fc.device_name]

        mappings = self.sensors_map[role]
        # iterate across each fc_sf vs psim_sf pair
        for fc_sf,psim_sf in mappings.items():
            local = self.sensor_readings[role][fc_sf]
            fc.write_state(fc_sf, local)

    def update_dynamics(self):
        """
        Allow simulation to step forward in time and update its
        truth.
        """
        self.__sim.step()

    def simulate_flight_computers(self):
        """
        For any flight computer simulations within the simulation,
        update their data.

        This is useful if the flight computer has not yet implemented
        certain controllers, but this function should eventually be
        removable from the simulation interface.
        """
        # we're all grown up now, don't need this
        pass 

    def read_actuators_send_to_sim(self):
        """
        Send actuator commands from the real flight computer to the
        simulation so that it can update dynamics.
        """

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

                self.__sim[psim_sf] = local
    
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