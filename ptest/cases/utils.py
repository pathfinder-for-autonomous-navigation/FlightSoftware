# Contains random utilities used for writing testcases.
import math

class TestCaseFailure(Exception):
    """Raise in case of test case failure."""

def is_lin_vector(var):
    import lin
    return type(var) in {lin.Vector2, lin.Vector3, lin.Vector4}

def to_lin_vector(var):
    import lin
    if type(var) == list:
        _len = len(var)
        if _len == 2:
            var = lin.Vector2(var)
        elif _len == 3:
            var = lin.Vector3(var)
        elif _len == 4:
            var = lin.Vector4(var)
        else:
            raise RuntimeError("Unexpected List Length, can't change into lin Vector")
    else:
        raise RuntimeError("Expected list, can't change into lin Vector")
    return var

def mag_of(vals):
        """
        Returns the magnitude of a list of vals 
        by taking the square root of the sum of the square of the components.
        """

        assert(type(vals) is list)
        return math.sqrt(sum([x*x for x in vals]))

def sum_of_differentials(lists_of_vals):
    """
    Given a list of list of vals, return the sum of all the differentials from one list to the next.

    Returns a val.

    Ex: sum_of_differentials([[1,1,1],[1,2,3],[1,2,2]]) evaluates to 4
    """

    total_diff = [0 for x in lists_of_vals[0]]
    for i in range(len(lists_of_vals) - 1):
        diff = [abs(lists_of_vals[i][j] - lists_of_vals[i+1][j]) for j in range(len(lists_of_vals[i]))]
        total_diff = [diff[x] + total_diff[x] for x in range(len(total_diff))]

    return sum(total_diff)

def str_to_val(field):
        '''
        Automatically detects floats, ints and bools

        Returns a float, int or bool
        '''
        if ',' in field:
            # ret is a list
            list_of_strings = field.split(',')
            list_of_strings = [x for x in list_of_strings if x != '']
            list_of_vals = [str_to_val(x) for x in list_of_strings]
            return list_of_vals
        elif 'nan' in field:
            return float("NAN")
        elif '.' in field:
            return float(field)
        elif field == 'true':
            return True
        elif field == 'false':
            return False
        else:
            return int(field)

class FSWEnum(object):
    """
    Class that encodes flight software enums into Python objects in
    a way that makes them easy to access during testing.
    """

    def __init__(self, arr):
        self.arr = arr
        self._indexed_by_name = {}
        self._indexed_by_num = {}
        for i in range(len(arr)):
            self._indexed_by_name[arr[i]] = i
            self._indexed_by_num[i] = arr[i]

    def __getitem__(self, item):
        if type(item) is str:
            return self._indexed_by_name[item]
        elif type(item) is int:
            return self._indexed_by_num[item]
        else:
            raise AttributeError(f"Cannot access FSWEnum with key: {item}")

    def names(self):
        return self._indexed_by_name.keys()

class Enums(object):
    mission_states = FSWEnum([
        "startup",
        "detumble",
        "initialization_hold",
        "standby",
        "follower",
        "leader",
        "follower_close_approach",
        "leader_close_approach",
        "docking",
        "docked",
        "safehold",
        "manual"
    ])

    prop_states = FSWEnum([
        "disabled",
        "idle",
        "await_pressurizing",
        "pressurizing",
        "venting",
        "firing",
        "await_firing",
        "handling_fault",
        "manual"
    ])

    adcs_states = FSWEnum([
        "manual",
        "startup",
        "limited",
        "zero_torque",
        "zero_L",
        "detumble",
        "point_manual",
        "point_standby",
        "point_docking"
    ])

    radio_states = FSWEnum([
        "wait",
        "transceive",
        "read",
        "write",
        "config",
        "disabled"
    ])

    qfh_states = FSWEnum([
        "unfaulted",
        "forced_standby",
        "powercycle_1",
        "powercycle_2",
        "powercycle_3",
        "safehold"
    ])

    sat_designations = FSWEnum([
        "undecided",
        "leader",
        "follower"
    ])

    piksi_modes = FSWEnum([
        "spp",
        "fixed_rtk",
        "float_rtk",
        "no_fix",
        "sync_error",
        "nsat_error",
        "crc_error",
        "time_limit_error",
        "data_error",
        "no_data_error",
        "dead"
    ])
    
    rwa_modes = FSWEnum([
        "RWA_DISABLED",
        "RWA_SPEED_CTRL",
        "RWA_ACCEL_CTRL"
    ])

    imu_modes = FSWEnum([
        "IMU_MAG_NORMAL",
        "IMU_MAG_CALIBRATE"
    ])

    mtr_modes = FSWEnum([
        "MTR_DISABLED",
        "MTR_ENABLED"
    ])

    ssa_modes = FSWEnum([
        "SSA_FAILURE",
        "SSA_IN_PROGRESS",
        "SSA_COMPLETE",
    ])

    # copied from havt_devices.hpp
    havt_devices = FSWEnum([
        "IMU_GYR",
        "IMU_MAG1",
        "IMU_MAG2",
        "MTR1",
        "MTR2",
        "MTR3",
        "RWA_POT",
        "RWA_WHEEL1",
        "RWA_WHEEL2",
        "RWA_WHEEL3",
        "RWA_ADC1",
        "RWA_ADC2",
        "RWA_ADC3",
        "SSA_ADC1",
        "SSA_ADC2",
        "SSA_ADC3",
        "SSA_ADC4",
        "SSA_ADC5",
        "GYRO_HEATER"])
    havt_length = len(havt_devices.arr)

    def __getitem__(self, key):
        key_associations = {
            "pan.state" : self.mission_states,
            "prop.state" : self.prop_states,
            "adcs.state" : self.adcs_states,
            "radio.state" : self.radio_states,
            "sat.designation" : self.sat_designations,
            "piksi.state" : self.piksi_modes,
            "adcs_monitor.ssa_mode" : self.ssa_modes,
        }
        return key_associations[key]

class BootUtil(object):
    """
    Utility for bootin satellite to a desired state.
    """

    def __init__(self, flight_controller, logger, state, fast_boot, one_day_ccno, _suppress_faults, skip_startup=False):
        """
        Initializes the booter.

        Args:
        - Testcase: Testcase object for which booter will be run. This
          is used to access the flight controller.
        - State: Desired boot state.
        - Fast boot: If true, the satellite will not
          walk through the state machine, but rather immediately force
          the satellite state to a particular value.
        - Skip startup: If true, skips the deployment wait sequence.
          This option is redundant if fast=True.
        """

        if state not in ["manual", "startup", "initialization_hold", "standby", "leader", "follower","detumble"]:
            raise ValueError("Invalid boot state specified.")

        self.flight_controller = flight_controller
        self.logger = logger
        self.desired_boot_state = state
        self.fast_boot = fast_boot
        self.one_day_ccno = one_day_ccno
        self.skip_startup = skip_startup
        self.finished = False
        self.suppress_faults = _suppress_faults
        
    def setup_boot(self):
        """
        Sets up initial conditions for the boot based on the desired boot stage.
        """

        mission_state_names = list(Enums.mission_states.names())

        # Booting to the nominal states requires detumbling.
        nominal_states = mission_state_names
        nominal_states.remove('manual')
        nominal_states.remove('safehold')
        nominal_states.remove('initialization_hold')

        if self.desired_boot_state in nominal_states:
            # Number of cycles for which the satellite will be in a deployment hold. This
            # is an item that is configured on Flight Software.
            self.deployment_hold_length = self.one_day_ccno // (24 * 2)
            self.elapsed_deployment = int(self.flight_controller.read_state("pan.deployment.elapsed"))

            # Number of cycles for which we expect the satellite to be in detumble
            self.max_detumble_cycles = 1000

        if self.suppress_faults:
            self.logger.put("[TESTCASE] Suppressing Faults!")

            # Prevent faults from mucking up the state machine.
            self.flight_controller.write_state("gomspace.low_batt.suppress", "true")
            self.logger.put("Suppressing gomspace.low_batt")
            
            self.flight_controller.write_state("fault_handler.enabled", "false")
            self.logger.put("Turning off fault_handler")

            # Suppress Prop Faults
            self.flight_controller.write_state("prop.overpressured.suppress", "true")
            self.logger.put("Suppressing turning off overpressued")

            self.flight_controller.write_state("prop.tank2_temp_high.suppress", "true")
            self.logger.put("Suppressing Tank2 temp high ")

            self.flight_controller.write_state("prop.tank1_temp_high.suppress", "true")
            self.logger.put("Suppressing Tank1 temp high")
    
            # Prevent ADCS faults from causing transition to initialization hold
            self.flight_controller.write_state("adcs_monitor.functional_fault.suppress", "true")
            self.logger.put("Suppressing adcs_monitor.functional_fault")

            self.flight_controller.write_state("adcs_monitor.wheel1_fault.suppress", "true")
            self.logger.put("Suppressing adcs_monitor.wheel1_fault")

            self.flight_controller.write_state("adcs_monitor.wheel2_fault.suppress", "true")
            self.logger.put("Suppressing adcs_monitor.wheel2_fault")

            self.flight_controller.write_state("adcs_monitor.wheel3_fault.suppress", "true")
            self.logger.put("Suppressing adcs_monitor.wheel3_fault")

            self.flight_controller.write_state("adcs_monitor.wheel_pot_fault.suppress", "true")
            self.logger.put("Suppressing adcs_monitor.wheel_pot_fault")

        self.logger.put(f"Waiting for the satellite to boot to {self.desired_boot_state}.")

        if self.fast_boot:
            self.flight_controller.write_state("pan.state", Enums.mission_states[self.desired_boot_state])
            self.logger.put(f"[TESTCASE] Fast Boot: Setting MissionMode to {self.desired_boot_state} state.")
            self.finished = True

    def run_boot_sequence(self):
        satellite_state = Enums.mission_states[\
            int(self.flight_controller.read_state("pan.state"))]
        true_elapsed = self.flight_controller.read_state("pan.deployment.elapsed")

        if not hasattr(self, "boot_stage"):
            self.boot_stage = "startup"

        if self.skip_startup and self.desired_boot_state == "standby"  and not self.finished:
            self.logger.put("[TESTCASE] Skipping deployment hold as it is requested by user.")
            self.flight_controller.write_state("pan.deployed", "true")
            self.boot_stage = "detumble"

        if self.boot_stage == 'startup':
            self.logger.put("")
            self.logger.put("[TESTCASE] Entering startup state.")

            if not satellite_state == "startup":
                raise TestCaseFailure(f"Satellite was not in state {self.boot_stage}.")
            self.boot_stage = 'deployment_hold'
            self.logger.put("[TESTCASE] Waiting for the deployment period to be over.")
            if self.desired_boot_state == "startup":
                self.finished = True
                self.logger.put("[TESTCASE] Successfully entered startup. Now in startup state.")

        elif self.boot_stage == 'deployment_hold' and not self.finished:
            if self.elapsed_deployment == self.deployment_hold_length:
                if satellite_state == "detumble":
                    self.logger.put("[TESTCASE] Deployment period is over. Entering detumble state.")
                    self.boot_stage = 'detumble'
                    self.num_detumble_cycles = 0
                    if self.desired_boot_state == "detumble":
                        self.logger.put("[TESTCASE] Successfully entered deployment. Now in detumble state.")
                        self.finished = True
                elif satellite_state == "initialization_hold" and self.desired_boot_state != "initalization_hold":
                    raise TestCaseFailure("Satellite went to initialization hold instead of detumble.")
                else:
                    raise TestCaseFailure(f"Satellite failed to exit deployment wait period. \
                        Satellite state is {satellite_state}. Elapsed deployment period was {true_elapsed}.")
            else:
                self.elapsed_deployment += 1

        elif self.boot_stage == 'detumble' and not self.finished:
            if self.num_detumble_cycles >= self.max_detumble_cycles or satellite_state == "standby":
                if satellite_state == "standby":
                    self.logger.put("[TESTCASE] Successfully detumbled. Now in standby state.")
                    self.boot_stage = 'standby'
                else:
                    raise TestCaseFailure("Satellite failed to exit detumble.")
            else:
                self.num_detumble_cycles += 1

        elif self.boot_stage == "standby" and not self.finished:
            if self.desired_boot_state == "standby":
                self.logger.put("[TESTCASE] Finished boot to standby state.")
            else:
                # Desired boot state is either leader or follower
                self.flight_controller.write_state("pan.state", Enums.mission_states[self.desired_boot_state])
                self.logger.put(f"[TESTCASE] Finished boot to {self.desired_boot_state} state.")
                self.boot_stage = self.desired_boot_state
            self.finished = True

        return self.boot_stage

    def finished_boot(self):
        if self.finished:
            return True
        else: 
            return self.run_boot_sequence() == self.desired_boot_state
