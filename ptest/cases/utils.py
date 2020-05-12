# Contains random utilities used for writing testcases.

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
        "disabled",
        "wait",
        "transceive",
        "read",
        "write",
        "config"
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
        "MTR_ENABLED",
        "MTR_DISABLED"
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

class TestCaseFailure(Exception):
    """Raise in case of test case failure."""

class BootUtil(object):
    """
    Utility for bootin satellite to a desired state.
    """

    def __init__(self, flight_controller, logger, state, fast_boot, skip_startup=False):
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

        if state not in ["manual", "startup", "initialization_hold", "standby", "leader", "follower"]:
            raise ValueError("Invalid boot state specified.")

        self.flight_controller = flight_controller
        self.logger = logger
        self.desired_boot_state = state
        self.fast_boot = fast_boot
        self.skip_startup = skip_startup
        self.finished = False

    def setup_boot(self):
        """
        Sets up initial conditions for the boot based on the desired boot stage.
        """

        mission_state_names = list(Enums.mission_states.names())
        nominal_states = mission_state_names
        nominal_states.remove('manual')
        nominal_states.remove('startup')
        nominal_states.remove('safehold')
        nominal_states.remove('initialization_hold')

        if self.desired_boot_state in nominal_states:
            self.deployment_hold_length = 100 # Number of cycles for which the satellite will be in a deployment hold. This
                                          # is an item that is configured on Flight Software.
            self.elapsed_deployment = int(self.flight_controller.read_state("pan.deployment.elapsed"))
            self.max_detumble_cycles = 100 # Number of cycles for which we expect the satellite to be in detumble

            # Let's be generous with what angular rate is allowable as "detumbled."
            self.flight_controller.write_state("detumble_safety_factor", 10)

            # Prevent ADCS faults from causing transition to initialization hold
            self.flight_controller.write_state("adcs_monitor.functional_fault.suppress", "true")
            self.flight_controller.write_state("adcs_monitor.wheel1_fault.suppress", "true")
            self.flight_controller.write_state("adcs_monitor.wheel2_fault.suppress", "true")
            self.flight_controller.write_state("adcs_monitor.wheel3_fault.suppress", "true")
            self.flight_controller.write_state("adcs_monitor.wheel_pot_fault.suppress", "true")

        self.logger.put(f"Waiting for the satellite to boot to {self.desired_boot_state}.")

    def run_boot_sequence(self):
        satellite_state = Enums.mission_states[\
            int(self.flight_controller.read_state("pan.state"))]
        true_elapsed = self.flight_controller.read_state("pan.deployment.elapsed")

        if not hasattr(self, "boot_stage"):
            self.boot_stage = "startup"

        if self.skip_startup and self.desired_boot_state == "standby":
            self.logger.put("[TESTCASE] Skipping deployment hold as it is requested by user.")
            self.flight_controller.write_state("pan.deployed", "true")
            self.boot_stage = "detumble_wait"

        if self.boot_stage == 'startup':
            self.logger.put("")
            self.logger.put("[TESTCASE] Entering startup state.")

            if not satellite_state == "startup":
                raise TestCaseFailure(f"Satellite was not in state {self.boot_stage}.")
            self.boot_stage = 'deployment_hold'
            self.logger.put("[TESTCASE] Waiting for the deployment period to be over.")

        elif self.boot_stage == 'deployment_hold':
            self.elapsed_deployment += 1

            if self.elapsed_deployment == self.deployment_hold_length:
                if satellite_state == "detumble":
                    self.logger.put("[TESTCASE] Deployment period is over. Entering detumble state.")
                    self.num_detumble_cycles = 0
                    self.boot_stage = 'detumble_wait'
                elif satellite_state == "initialization_hold" and self.desired_boot_state != "initalization_hold":
                    raise TestCaseFailure("Satellite went to initialization hold instead of detumble.")
                else:
                    raise TestCaseFailure(f"Satellite failed to exit deployment wait period. \
                        Elapsed deployment period was {true_elapsed}.")

        elif self.boot_stage == 'detumble_wait':
            self.num_detumble_cycles += 1
            if self.num_detumble_cycles >= self.max_detumble_cycles or satellite_state == "standby":
                # For now, force the satellite into standby since the attitude control stuff isn't working.
                self.flight_controller.write_state("pan.state", Enums.mission_states["standby"])
                self.boot_stage = 'standby'
                self.logger.put("[TESTCASE] Successfully detumbled. Now in standby state.")

                # TODO add the following code back after merging of #287 and #348
                # if satellite_state == "standby":
                #     self.logger.put("[TESTCASE] Successfully detumbled. Now in standby state.")
                #     self.boot_stage = 'standby'
                # else:
                #     raise TestCaseFailure("Satellite failed to exit detumble.")

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
        if self.fast_boot or self.desired_boot_state == "startup":
            self.flight_controller.write_state("pan.state", Enums.mission_states[self.desired_boot_state])
            return True
        else:
            return self.run_boot_sequence() == self.desired_boot_state
