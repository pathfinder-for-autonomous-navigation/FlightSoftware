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

def suppress_faults(fc, logger):
    """Supress faults associated on a given flight controller.
    """
    fc.write_state("fault_handler.enabled", "false")
    logger.put("Turning off fault_handler")

    fc.write_state("piksi_fh.enabled", "false")
    logger.put("Turning off piksi_fh")

    faults = [
        "gomspace.low_batt",
        "gomspace.get_hk.base",
        "prop.overpressured",
        "prop.tank2_temp_high",
        "prop.tank1_temp_high",
        "adcs_monitor.functional_fault",
        "adcs_monitor.wheel1_fault",
        "adcs_monitor.wheel2_fault",
        "adcs_monitor.wheel3_fault",
        "adcs_monitor.wheel_pot_fault",
        "attitude_estimator.fault"
    ]
    for fault in faults:
        fc.write_state(fault + ".suppress", "true")
        logger.put(f"Supressing {fault}")

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
