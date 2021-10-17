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
import lin 
import json
from ..utils import is_lin_vector, to_lin_vector,Enums
import math
from .ptest_case import PTestCase
from .single_sat_case import SingleSatCase

class PSimCase(PTestCase):
    """Base class for testcases running with a PSim simulation.

    Attributes:
        psim_config_override Dictionary mapping PSim configuration parameter
                             names to override values.
        psim_config_prefix Prefix path to the PSim configuration files. This
                           defaults to 'lib/common/psim/config/parameters' and
                           shouldn't need to be changed.
        psim_config_suffix Suffix to the PSim configuration files. This defaults
                           to '.txt' and shouldn't need to be changed.
        psim_configs Set of PSim configuration files.
    """
    def __init__(self, *args, **kwargs):
        super(PSimCase, self).__init__(*args, **kwargs)

        self.psim_config_overrides = dict()
        self.psim_config_prefix = 'lib/common/psim/config/parameters/'
        self.psim_config_suffix = '.txt'
        self.psim_configs = [
            'truth/base',
            'sensors/base'
        ]

        self.__is_single_sat = isinstance(self, SingleSatCase)

        self.mock_sensors = True

    def setup(self, *args, **kwargs):
        super(PSimCase, self).setup(*args, **kwargs)

        from psim import Configuration, Simulation
        from psim.sims import DualAttitudeOrbitGnc, SingleAttitudeOrbitGnc

        # Load the configuration and override the requested configuration
        # values.
        configs = [
            self.psim_config_prefix + config + self.psim_config_suffix
            for config in self.psim_configs
        ]
        config = Configuration(configs)
        for name, param in self.psim_config_overrides.items():
            config[name] = param

        # Choose the simulation model depending on whether we have one or two
        # spacecraft.
        psim_model = SingleAttitudeOrbitGnc if self.__is_single_sat else DualAttitudeOrbitGnc

        # Initialize the simulation.
        self.__sim = Simulation(psim_model, config)

        # Generate the pairing between flight controllers and satellite names.
        if self.__is_single_sat:
            self.__fc_satellite_pairs = [("leader", self.flight_controller)]
        else:
            self.__fc_satellite_pairs = [
                ("leader", self.flight_controller_leader),
                ("follower", self.flight_controller_follower)
            ]

    def cycle(self, *args, **kwargs):
        super(PSimCase, self).cycle(*args, **kwargs)

        def load_to_fc(fc, sensors):
            for name, value in sensors.items():
                fc.write_state(name, value)

        # Read actuator data from the flight computer(s) and send it to PSim.
        for satellite, fc in self.__fc_satellite_pairs:
            self.__actuators(fc, satellite)

        # Step PSim forward in time.
        self.__sim.step()

        # Load sensor data from PSim back into the flight computer.
        if self.mock_sensors:
            threads = list()
            for satellite, fc in self.__fc_satellite_pairs:
                sensors = self.__poll_sensors(satellite)
                thread = threading.Thread(target=load_to_fc, daemon=True, args=(fc, sensors))
                threads.append(thread)
                thread.start()

            # Wait for all state field transactions to be complete.
            for thread in threads:
                thread.join()

    def __actuators(self, fc, satellite):
        """Reads actuator outputs from a flight computer into the simulation for
        the specified satellite name.

        Currently, we're only looking at the magnetorquer and wheel commands.
        """
        self.__sim[f"truth.{satellite}.wheels.t"] = lin.Vector3(fc.smart_read("adcs_cmd.rwa_torque_cmd"))
        self.__sim[f"truth.{satellite}.magnetorquers.m"] = lin.Vector3(fc.smart_read("adcs_cmd.mtr_cmd"))

    def __poll_sensors(self, satellite):
        """Polls sensor data from the simulation for the specified satellite and
        returns a key value mapping of flight computer state field to sensor
        values.
        """
        sensors = dict()

        # Simulate the Piksi.
        #
        # The Piksi can be in one of three states: we can have no GPS or CDGPS
        # data, we can have only GPS data, or we can have both CDGPS and GPS
        # data. All cases are handled here.
        gps_is_valid = self.__sim[f"sensors.{satellite}.gps.valid"]

        if gps_is_valid:
            if self.__sim.get(f"sensors.{satellite}.cdgps.valid"):
                sensors["piksi.state"] = Enums.piksi_modes["fixed_rtk"]
                sensors["piksi.baseline_pos"] = list(self.__sim[f"sensors.{satellite}.cdgps.dr"])
            else:
                sensors["piksi.state"] = Enums.piksi_modes["spp"]

            sensors["piksi.time"] = GPSTime(self.__sim['truth.t.ns']).to_list()
            sensors["piksi.microdelta"] = 0
            sensors["piksi.pos"] = list(self.__sim[f"sensors.{satellite}.gps.r"])
            sensors["piksi.vel"] = list(self.__sim[f"sensors.{satellite}.gps.v"])
        else:
            sensors["piksi.state"] = Enums.piksi_modes["no_fix"]

        # Simulate the gyroscope.
        #
        # Here, we simply see if the gyroscope data is valid, mock the ADCS
        # HAVT, and pass along the reading if applicable.
        gyroscope_is_valid = self.__sim[f"sensors.{satellite}.gyroscope.valid"]

        sensors["adcs_monitor.havt_device0"] = gyroscope_is_valid
        if gyroscope_is_valid:
            sensors["adcs_monitor.gyr_vec"] = list(self.__sim[f"sensors.{satellite}.gyroscope.w"])

        # Simulate the magnetometer
        #
        # Here, we simply see if the magnetometer data is valid, mock the ADCS
        # HAVT, and pass along the reading if applicable.
        magnetometer_is_valid = self.__sim[f"sensors.{satellite}.magnetometer.valid"]

        sensors["adcs_monitor.havt_device2"] = magnetometer_is_valid
        if magnetometer_is_valid:
            sensors["adcs_monitor.mag2_vec"] = list(self.__sim[f"sensors.{satellite}.magnetometer.b"])

        # Simulate the sun sensors
        #
        # Here we check if the sun vector is valid, mock the sun sensor mode,
        # and pass along the reading if applicable.
        sun_sensors_are_valid = self.__sim[f"sensors.{satellite}.sun_sensors.valid"]

        if sun_sensors_are_valid:
            sensors["adcs_monitor.ssa_mode"] = Enums.ssa_modes['SSA_COMPLETE']
            sensors["adcs_monitor.ssa_vec"] = list(self.__sim[f"sensors.{satellite}.sun_sensors.s"])
        else:
            sensors["adcs_monitor.ssa_mode"] = Enums.ssa_modes['SSA_FAILURE']

        # Simulate wheel speeds
        #
        # As wheel speeds are required for magnetometer data offsets now,
        # wheel speeds must be simmed
        sensors['adcs_monitor.rwa_speed_rd'] = list(self.__sim[f'truth.{satellite}.wheels.w'])

        return sensors

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
        ret = self.rs_psim(name)
        self.logger.put(f"{name} is {ret}")

    def ws_psim(self, name, value):
        '''
        Write to a psim state field.
        '''
        if type(value) == list:
            linTypes = [lin.Vector2, lin.Vector3, lin.Vector4]
            self.__sim[name] = linTypes[len(list)](value)
        else:
            self.__sim[name] = value
