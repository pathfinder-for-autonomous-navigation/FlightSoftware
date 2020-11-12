from .base import SingleSatOnlyCase
from .utils import FSWEnum, Enums, TestCaseFailure, BootUtil

# pio run -e fsw_native_leader
# python -m ptest runsim -c ptest/configs/ci.json -t PropFaultHandler
FAKE_PRESSURE = True
class PropFaultHandler(SingleSatOnlyCase):
    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state(self):
        return "leader"

    @property
    def fast_boot(self):
        return False

    def setup_pre_bootsetup(self):
        self.powercycle_happening = None

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)
        self.flight_controller.write_state("dcdc.SpikeDock_cmd", True)

# --------------------------------------------------------------------------------------
# Helper methods for tests
# --------------------------------------------------------------------------------------

    def force_fault(self, fault_name):
        self.ws("{}.suppress".format(fault_name), False)
        self.ws("{}.override".format(fault_name), True)

    def reset_fault(self, fault_name):
        if FAKE_PRESSURE:
            self.ws("prop.tank2.pressure", 12)
        self.ws("{}.suppress".format(fault_name), True)
        self.ws("{}.override".format(fault_name), False)   

    def check_mission_state(self, expected, reason=""):
        if self.mission_state != expected:
            raise TestCaseFailure(
                f"Expected mission_state = {expected}, but mission_state = {self.mission_state}. Reason: {reason}")
    
    def check_prop_state(self, expected, reason=""):
        if self.state != str(Enums.prop_states[expected]):
            raise TestCaseFailure(
                f"Expected prop.state = {Enums.prop_states[expected]}, but prop.state = {self.state}. Reason: {reason}")

    def collect_diagnostic_data(self):
        self.rs("prop.state")
        self.rs("pan.state")
        self.rs("pan.cycle_no")

    @property
    def state(self):
        return str(self.read_state("prop.state"))

    @state.setter
    def state(self, val):
        self.ws("prop.state", str(Enums.prop_states[val]))

# --------------------------------------------------------------------------------------
# Test Case
# --------------------------------------------------------------------------------------

    def test_pressurize_fail(self):
        self.test_fault()
        if self.test_stage == "reset":
            self.fault_name = "prop.overpressured"

    def test_overpressured(self):
        self.test_fault()
        if self.test_stage == "reset":
            self.fault_name = "prop.tank2_temp_high"
    
    def test_tank2_temp_high(self):
        self.test_fault()
        if self.test_stage == "reset":
            self.fault_name = "prop.tank1_temp_high"

    def test_tank1_temp_high(self):
        self.test_fault()
        if self.test_stage == "reset":
            self.fault_name = "finished"

    def dispatch_test(self):
        if FAKE_PRESSURE:
            self.ws("prop.tank2.pressure", 12)
        if self.fault_name == "prop.pressurize_fail":
            self.test_pressurize_fail()
        elif self.fault_name == "prop.overpressured":
            self.test_overpressured()
        elif self.fault_name == "prop.tank2_temp_high":
            self.test_tank2_temp_high()
        elif self.fault_name == "prop.tank1_temp_high":
            self.test_tank1_temp_high()
        elif self.fault_name == "finished":
            self.finish()

    def test_fault(self):
        if FAKE_PRESSURE:
            self.ws("prop.tank2.pressure", 12)
        if self.test_stage == "init":
            self.logger.put("[TESTCASE] Starting test for {}".format(self.fault_name))
            self.state = "idle"
            self.test_stage = "force_fault"

        elif self.test_stage == "force_fault":
            self.check_prop_state("idle")
            self.check_mission_state("leader", "mission state should be in leader")
            self.logger.put("Overriding the {} fault.".format(self.fault_name))
            self.force_fault(self.fault_name)
            self.test_stage = "handling_fault"

        elif self.test_stage == "handling_fault":
            self.check_prop_state("handling_fault", "prop state should be in handling fault since the fault is forced")
            self.logger.put("Prop now in handling_fault.")
            # pressurize_fail has different behavior than the other faults. It stays in handling_fault until suppressed by the ground
            if FAKE_PRESSURE:
                self.ws("prop.tank2.pressure", 12)
            if self.fault_name == "prop.pressurize_fail":
                self.test_stage = "handle_pressurize_fail"
                self.cycle()
                if FAKE_PRESSURE:
                    self.ws("prop.tank2.pressure", 12)
            else:
                self.test_stage = "venting"
                self.cycle()
                if FAKE_PRESSURE:
                    self.ws("prop.tank2.pressure", 12)
        
        elif self.test_stage == "handle_pressurize_fail":
            self.check_prop_state("handling_fault", "prop should remain in handling fault if pressurize_fail is not suppressed")
            self.check_mission_state("standby", "satellite should be in standby when handling fault")
            # Need to reset completely since override takes precedence
            self.reset_fault("prop.pressurize_fail")
            self.logger.put("Suppressing pressurize_fail")
            self.test_stage = "recover_pressurize_fail"

        elif self.test_stage == "recover_pressurize_fail":
            self.check_prop_state("idle", "prop should be in idle when pressurize_fail is suppressed")
            self.test_stage = "reset"


        elif self.test_stage == "venting":
            self.check_prop_state("venting", "prop state should be in venting since the fault is forced")
            self.logger.put("Prop now in venting.")
            self.check_mission_state("standby", "mission state should be in standby when prop is venting")
            self.logger.put("Satellite now in in standby state.")
            self.reset_fault(self.fault_name)
            self.test_stage = "reset"

        elif self.test_stage == "reset":
            self.logger.put("Resetting satellite state")
            self.check_prop_state("idle", "prop state should be in idle when fault is suppressed")
            self.mission_state = "leader"
            self.test_stage = "init"

    def run_case_singlesat(self):
        if FAKE_PRESSURE:
            self.ws("prop.tank2.pressure", 12)
        if not hasattr(self, "fault_name"):
            self.fault_name = "prop.pressurize_fail"
            self.test_stage = "init"

        if not self.finished:
            self.collect_diagnostic_data()
            self.dispatch_test()
