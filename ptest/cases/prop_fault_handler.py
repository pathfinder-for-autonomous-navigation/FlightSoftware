from .base import SingleSatCase
from .utils import Enums, TestCaseFailure

# pio run -e fsw_native_leader
# python -m ptest runsim -c ptest/configs/ci.json -t PropFaultHandler
class PropFaultHandler(SingleSatCase):
    def __init__(self, *args, **kwargs):
        super(PropFaultHandler, self).__init__(*args, **kwargs)

        self.tank2_pressure = 12.0
        self.tank2_temp = 25.0
        self.tank1_temp = 25.0

        self.THRESH_PRESS = 25.0
        self.MAX_SAFE_TEMP = 49.0
        self.MAX_SAFE_PRESS = 75.0

        self.fault_name = "prop.pressurize_fail"

    @property
    def initial_state(self):
        return "leader"

    @property
    def fast_boot(self):
        return True

    def pre_boot(self):
        self.powercycle_happening = None

    def post_boot(self):
        self.ws("fault_handler.enabled", True)
        
        self.ws("prop.overpressured.suppress", "false")
        self.logger.put("Releasing overpressured suppress")

        self.ws("prop.tank2_temp_high.suppress", "false")
        self.logger.put("Releasing Tank2 temp high suppress")

        self.ws("prop.tank1_temp_high.suppress", "false")
        self.logger.put("Releasing Tank1 temp high suppress")

        # Lower these so that we don't need to wait
        self.ws("prop.ctrl_cycles_per_filling", 1)
        self.ws("prop.ctrl_cycles_per_cooling", 2)
        self.ws("prop.max_pressurizing_cycles", 4)
        self.ws("prop.max_venting_cycles", 8)

        # Enable SpikeDock so we can actually change prop state
        self.ws("dcdc.SpikeDock_cmd", True)

# --------------------------------------------------------------------------------------
# Prop Properties
# --------------------------------------------------------------------------------------
    @property
    def prop_state(self):
        return self.rs("prop.state")

    @prop_state.setter
    def prop_state(self, val):
        self.ws("prop.state", Enums.prop_states[val])

    # Set fake sensor values on the prop
    # Default values are set if param list is empty
    def fake_sensors(self, tank2_press=12.0, tank2_temp=25.0, tank1_temp=25.0):
        self.tank2_pressure = tank2_press
        self.tank2_temp = tank2_temp
        self.tank1_temp = tank1_temp
        self.write_sensors()

    def write_sensors(self):
        self.ws("prop.tank2.temp",  str(self.tank2_temp))
        self.ws("prop.tank1.temp",  str(self.tank1_temp))
        self.ws("prop.tank2.pressure", str(self.tank2_pressure))

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
        """Minimum number of cycles needed to set the schedule in idle
        """
        return (int(self.ctrl_cycles_per_filling) + int(self.ctrl_cycles_per_cooling))*int(self.max_pressurizing_cycles) + 4

    @property
    def num_vent_cycles(self):
        """Maximum number of control cycles spent venting
        """
        return (int(self.ctrl_cycles_per_filling) + int(self.ctrl_cycles_per_cooling))*(int(self.max_venting_cycles)*2)

    @property
    def num_duel_vent(self):
        """Number of control cycles spent venting one tank when we need to vank both tanks
        """
        return (int(self.ctrl_cycles_per_filling) + int(self.ctrl_cycles_per_cooling))*2 + 2

    def cycle(self):
        """Overriden from base.py
        Write sensors at the END because PropController updates sensor values at the end of execution cycle
        """
        super().cycle()
        self.write_sensors()

# --------------------------------------------------------------------------------------
# Helper methods for tests
# --------------------------------------------------------------------------------------

    def check_mission_state(self, expected, reason=""):
        if self.mission_state != expected:
            raise TestCaseFailure(
                f"Expected mission_state = {expected}, but mission_state = {self.mission_state}: {reason}")
    
    def check_prop_state(self, expected, reason=""):
        if int(self.prop_state) != Enums.prop_states[expected]:
            raise TestCaseFailure(
                f"Expected prop.state = {expected}, but prop.state = {Enums.prop_states[self.prop_state]}: {reason}")

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
        self.fault_name = "Venting Both Tanks"
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
        """Ran at the beginning of each subtest to "reset" HITL state
        """
        self.logger.put("[TEST] Starting test {}".format(self.fault_name))
        self.fake_sensors()                                 # set sensors to "good" values
        self.prop_state = "idle"                            # set state to idle
        self.mission_state = "leader"                       # set mission state to leader
        self.cycle()
        self.check_all_faults()                             # make sure there are no faults high
        self.check_prop_state("idle")                       # make sure state is idle
        self.check_mission_state("leader")                  # make sure mission is leader

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
        # Fault requires 1 more than the persistence value to be signalled
        for _ in range( self.rs("prop.overpressured.persistence") + 1):
            self.check_prop_state("idle")
            self.check_mission_state("leader")
            self.cycle()
        self.check_prop_state("idle")
        self.check_prop_fault(self.fault_name, False)
        self.check_mission_state("leader")

        # Standby issued on the 12th cycle because PropFaultHandler executes BEFORE PropController
        self.cycle()
        self.check_prop_state("handling_fault")
        self.check_prop_fault(self.fault_name, True)
        self.check_mission_state("leader")

        # Venting is entered on the 13th cycle
        self.cycle()
        self.check_prop_state("venting")
        self.check_mission_state("standby")

        # We vent for max_venting_cycles
        for _ in range(self.num_vent_cycles):
            self.cycle()
        
        self.check_prop_state("venting")
        self.cycle()
        
        # Then we go to disable when we run out of cycles
        self.check_prop_state("disabled")

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

        for _ in range(self.rs("prop.tank1_temp_high.persistence") + 1):
            self.check_prop_state("idle")
            self.cycle()
        self.check_prop_state("idle")
        self.check_prop_fault("prop.overpressured", False)
        self.check_prop_fault("prop.tank1_temp_high", False)

        self.cycle()
        self.check_mission_state("leader")
        self.check_prop_state("handling_fault")
        self.check_prop_fault("prop.overpressured", True)
        self.check_prop_fault("prop.tank1_temp_high", True)

        self.cycle()
        self.check_mission_state("standby")
        self.check_prop_state("venting")
        self.check_prop_fault("prop.overpressured", True)
        self.check_prop_fault("prop.tank1_temp_high", True)

        # Venting tank2
        for _ in range(self.num_duel_vent-1):
            self.cycle()
        self.check_prop_state("handling_fault")
        self.check_prop_fault("prop.overpressured", True)
        self.check_prop_fault("prop.tank1_temp_high", True)

        # Venting tank1
        for _ in range(self.num_duel_vent):
            self.cycle()
        self.check_prop_state("handling_fault")
        self.check_prop_fault("prop.overpressured", True)
        self.check_prop_fault("prop.tank1_temp_high", True)

        # Venting tank2
        for _ in range(self.num_duel_vent-1):
            self.cycle()

        # Venting tank1
        for _ in range(self.num_duel_vent):
            self.cycle()

        # Venting tank2
        # Now pretend that tank1 conditions improve
        for _ in range(self.num_duel_vent):
            self.tank1_temp -= 5
            self.cycle()

        # Now instead of switching to tank1, we should continue venting tank2
        self.check_prop_state("venting")
        self.check_prop_fault("prop.overpressured", True)
        self.check_prop_fault("prop.tank1_temp_high", False)
        for _ in range(self.num_vent_cycles):
            self.cycle()

        # Unfortunately, tank2 conditions don't improve..., so we go into disabled
        self.check_prop_state("disabled")