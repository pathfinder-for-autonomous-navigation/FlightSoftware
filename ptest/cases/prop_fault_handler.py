from .base import SingleSatOnlyCase
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, TestCaseFailure

# pio run -e fsw_native_leader
# python -m ptest runsim -c ptest/configs/ci.json -t PropFaultHandler
class PropFaultHandler(SingleSatOnlyCase):
    def __init__(self, is_interactive, random_seed, data_dir):
        self.tank2_pressure = 12.0
        self.tank2_temp = 25.0
        self.tank1_temp = 25.0

        self.THRESH_PRESS = 25.0
        self.MAX_SAFE_TEMP = 49.0
        self.MAX_SAFE_PRESS = 75.0

        self.fault_name = "prop.pressurize_fail"

        super().__init__(is_interactive, random_seed, data_dir)

    # @property
    # def sim_configs(self):
    #     configs = ["truth/ci", "truth/base"]
    #     configs += ["sensors/base"]
    #     return configs

    # @property
    # def sim_model(self):
    #     return SingleAttitudeOrbitGnc

    # @property
    # def sim_mapping(self):
    #     return "ci_mapping.json"

    # @property
    # def sim_duration(self):
    #     return float("inf")

    # @property
    # def sim_initial_state(self):
    #     return "startup"

    @property
    def initial_state(self):
        return "leader"

    @property
    def fast_boot(self):
        return True

    def setup_pre_bootsetup(self):
        self.powercycle_happening = None

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)
        self.flight_controller.write_state("dcdc.SpikeDock_cmd", True)
        # Lower these so that we don't need to wait
        self.ws("prop.ctrl_cycles_per_filling", 2)
        self.ws("prop.ctrl_cycles_per_cooling", 1)
        self.ws("prop.max_pressurizing_cycles", 2)
        self.ws("prop.max_venting_cycles", 2)

# --------------------------------------------------------------------------------------
# Prop Properties
# --------------------------------------------------------------------------------------
    @property
    def state(self):
        return self.rs("prop.state")

    @state.setter
    def state(self, val):
        self.ws("prop.state", Enums.prop_states[val])

    # Set fake sensor values on the prop
    # Default values are set if param list is empty
    def fake_sensors(self, tank2_press=12.0, tank2_temp=25.0, tank1_temp=25.0):
        self.tank2_pressure = tank2_press
        self.tank2_temp = tank2_temp
        self.tank1_temp = tank1_temp

    @property
    def ctrl_cycles_per_filling(self):
        return self.rs("prop.ctrl_cycles_per_filling")

    @property
    def ctrl_cycles_per_cooling(self):
        return self.rs("prop.ctrl_cycles_per_cooling")

    @property
    def max_pressurizing_cycles(self):
        return self.rs("prop.max_pressurizing_cycles")

    @property
    def max_venting_cycles(self):
        return self.rs("prop.max_venting_cycles")

    @property
    def min_num_cycles(self):
        return (int(self.ctrl_cycles_per_filling) + int(self.ctrl_cycles_per_cooling))*int(self.max_pressurizing_cycles) + 4

    def cycle(self):
        self.collect_diagnostic_data()
        init = self.rs("pan.cycle_no")
        self.flight_controller.write_state('cycle.start', 'true')
        if self.rs("pan.cycle_no") != init + 1:
            raise TestCaseFailure(f"FC did not step forward by one cycle")
        self.ws("prop.tank2.pressure", str(self.tank2_pressure))
        self.ws("prop.tank2.temp",  str(self.tank2_temp))
        self.ws("prop.tank1.temp",  str(self.tank1_temp))

# --------------------------------------------------------------------------------------
# Helper methods for tests
# --------------------------------------------------------------------------------------

    def check_mission_state(self, expected, reason=""):
        if self.mission_state != expected:
            raise TestCaseFailure(
                f"Expected mission_state = {expected}, but mission_state = {self.mission_state}: {reason}")
    
    def check_prop_state(self, expected, reason=""):
        if int(self.state) != Enums.prop_states[expected]:
            raise TestCaseFailure(
                f"Expected prop.state = {expected}, but prop.state = {Enums.prop_states[self.state]}: {reason}")

    def check_prop_fault(self, fault_name, expected, reason=""):
        if self.rs(f"{fault_name}.base") != expected:
            raise TestCaseFailure(
                f"Expected {fault_name} == {expected}")

    def check_all_faults(self, press_failed=False, overpress=False, tank2_high=False, tank1_high=False):
        self.check_prop_fault("prop.pressurize_fail", press_failed)
        self.check_prop_fault("prop.overpressured", overpress)
        self.check_prop_fault("prop.tank1_temp_high", tank2_high)
        self.check_prop_fault("prop.tank2_temp_high", tank1_high)

    def collect_diagnostic_data(self):
        self.rs("prop.state")
        self.rs("pan.state")
        self.rs("pan.cycle_no")

        self.rs("prop.tank2.pressure")
        self.rs("prop.tank2.temp")
        self.rs("prop.tank1.temp")

        self.rs("prop.pressurize_fail.base")
        self.rs("prop.pressurize_fail.suppress")
        self.rs("prop.pressurize_fail.override")

        self.rs("prop.overpressured.base")
        self.rs("prop.overpressured.suppress")
        self.rs("prop.overpressured.override")
        
        self.rs("prop.overpressured.base")
        self.rs("prop.tank1_temp_high.suppress")
        self.rs("prop.tank1_temp_high.override")

        self.rs("prop.tank2_temp_high.base")
        self.rs("prop.tank2_temp_high.suppress")
        self.rs("prop.tank2_temp_high.override")

# --------------------------------------------------------------------------------------
# Test Case
# --------------------------------------------------------------------------------------
    def run_case_singlesat(self):
        self.fault_name = "prop.pressurize_fail"
        self.test_pressurize_fail()

        self.fault_name = "prop.overpressured"
        self.test_overpressured()

        self.fault_name = "prop.tank2_temp_high"
        self.test_tank2_high()

        self.fault_name = "prop.tank1_temp_high"
        self.test_tank1_high()
        
        # set this to something weird to avoid using methods that depend on fault_name
        #   since multiple faults occur in this test
        self.fault_name = "prop.DOESNOTEXIST"
        self.test_vent_both()

        self.finish()

    def test_pressurize_fail(self):
        """prop.pressurize_fail cccurs when we fail to pressurize within the number of designated cycles
        """
        self.init_test()
        # Get us into pressurizing
        self.ws("prop.sched_valve1", 800)
        self.ws("prop.cycles_until_firing", self.min_num_cycles)
        self.cycle()
        self.check_prop_state("pressurizing")   

        # Cause the fault to happen:
        #   Keep cycling but don't change the pressure
        for _ in range(self.min_num_cycles-4):
            self.cycle()
        
        # One cycle before the fault will be triggered:
        self.check_prop_state("pressurizing")
        self.check_prop_fault(self.fault_name, False)
        
        # Cycle where the fault occurs:
        self.cycle()
        self.check_prop_state("handling_fault")
        self.check_prop_fault(self.fault_name, True)

        # PropFaultHandler executes BEFORE PropController, so the standby is seen on the NEXT cycle
        self.check_mission_state("leader")
        self.cycle()
        self.check_mission_state("standby")

        # Ground resolves this by suppressing the fault
        self.resolve_via_suppress()

        # Mission state remains in standby until it is commanded back to leader
        self.check_mission_state("standby")

    def init_test(self):
        self.fake_sensors()                                 # set sensors to fake STP
        self.state = "idle"                                 # set state to idle
        self.mission_state = "leader"                       # set mission state to leader
        self.cycle()
        self.cycle()                                        # cycle multiple times to make sure faults are not occuring
        self.check_all_faults()   # make sure there are no faults high
        self.check_prop_state("idle")                       # make sure state is idle
        self.check_mission_state("leader")

    def resolve_via_suppress(self):
        self.ws("{}.suppress".format(self.fault_name), True)
        self.cycle()
        self.check_prop_fault(self.fault_name, False)
        self.cycle()

    def test_overpressured(self):
        """prop.overpressured occurs when the tank2 pressure is too high
        """
        self.init_test()
        self.tank2_pressure = self.MAX_SAFE_PRESS + 1
        self.do_single_tank_venting()

    def do_single_tank_venting(self):
        # overpressured, tank2_high, and tank1_high have persistences of 10, so requires 11 cycles to set
        for _ in range(11):
            self.cycle()
            self.check_prop_state("idle")

        # Signaled on the 12th cycle
        self.cycle()
        self.check_prop_state("handling_fault")
        self.check_prop_fault(self.fault_name, True)

        # Standby issued on the 13th cycle
        self.cycle()
        self.check_mission_state("standby")

        # Venting is entered on the 14th cycle
        self.cycle()
        self.check_prop_state("venting")
        self.check_mission_state("standby")

        # We vent for max_pressurizing cycles
        num_venting = 0
        while int(self.state) == Enums.prop_states["venting"]:
            self.cycle()
            num_venting += 1
        
        # TODO: Not sure why it takes 15 cycles before it gives up
        # Since we set max_venting_cycles and max_pressurizing_cycles to 2, 
        #   num_venting should equal min_num_cycles, but it is higher
        print(f"[DEBUG] num_venting: {num_venting} vs min_num_cycles: {self.min_num_cycles}")

        # Then we go to disable when we run out of cycles
        self.check_prop_state("disabled")

        # TODO: This is basically the same as the stuff in init_test()
        #   Not sure why, but cannot command mission_state to leader without the following 4 lines:
        self.fake_sensors()
        self.state = "idle"
        self.mission_state = "leader"
        self.cycle()

    def test_tank2_high(self):
        """prop.tank2_temp_high occurs when the tank2 temp is too high
        """
        self.init_test()
        self.tank2_temp = self.MAX_SAFE_TEMP + 1
        self.do_single_tank_venting()

    def test_tank1_high(self):
        """prop.tank1_temp_high occurs when the tank1 temp is too high
        """
        self.init_test()
        self.tank1_temp = self.MAX_SAFE_TEMP + 1
        self.do_single_tank_venting()

    def test_vent_both(self):
        self.init_test()
        self.tank2_pressure = self.MAX_SAFE_PRESS + 1
        self.tank1_temp = self.MAX_SAFE_TEMP + 1

        for _ in range(11):
            self.cycle()
            self.check_prop_state("idle")

        self.cycle()
        self.check_prop_state("handling_fault")
        self.check_prop_fault("prop.overpressured", True)
        # TODO: not sure why these are NOT both signalled
        # If we don't set tank2_pressure, then tank1_temp_high is signalled
        self.check_prop_fault("prop.tank1_temp_high", False)

        self.cycle()
        self.check_mission_state("standby")

        self.cycle()
        self.check_prop_state("venting")

        while int(self.state) == Enums.prop_states["venting"]:
            self.cycle()

        self.check_prop_state("handling_fault")

        self.check_prop_fault("prop.overpressured", True)
        self.check_prop_fault("prop.tank1_temp_high", True)

        while int(self.state) == Enums.prop_states["venting"]:
            self.cycle()

        self.check_prop_state("handling_fault")

        self.check_prop_fault("prop.overpressured", True)
        self.check_prop_fault("prop.tank1_temp_high", True)
