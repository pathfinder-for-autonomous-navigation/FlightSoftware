from .base import SingleSatOnlyCase
from .utils import FSWEnum, Enums, TestCaseFailure, BootUtil

# pio run -e fsw_native_leader
# python -m ptest runsim -c ptest/configs/ci.json -t PropFaultHandler
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
        self.sim.flight_controller.write_state("dcdc.SpikeDock_cmd", True)

# --------------------------------------------------------------------------------------
# Helper methods for tests
# --------------------------------------------------------------------------------------

    def force_fault(self, fault_name):
        self.ws("{}.suppress".format(fault_name), False)
        self.ws("{}.override".format(fault_name), True)

    def reset_fault(self, fault_name):
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
# Helper methods for the prop state machine
# --------------------------------------------------------------------------------------

    def goto_idle(self):
        self.state = "idle"
        self.cycle()
        self.check_prop_state("idle")
    
    def goto_await_pressurizing(self):
        self.goto_idle()
        self.sched_valve1 = 100
        self.sched_valve2 = 700
        self.sched_valve3 = 800
        self.sched_valve4 = 400
        self.cycles_until_firing = int(self.min_num_cycles) + 2
        self.cycle()
        self.check_prop_state("await_pressurizing")

    def goto_pressurize(self):
        self.goto_await_pressurizing()
        self.cycle_until_change()
        self.check_prop_state("pressurizing")

    def goto_await_firing(self):
        self.goto_pressurize()
        self.cycle_until_change()
        self.check_prop_state("await_firing")

    def goto_firing(self):
        self.goto_await_firing()
        self.cycle_until_change()
        self.check_prop_state("firing")

# --------------------------------------------------------------------------------------
# Test Case
# --------------------------------------------------------------------------------------

    def run_case_singlesat(self):
        if not self.finished:
            self.collect_diagnostic_data()
        if not hasattr(self, "test_stage"):
            self.test_stage = "force_fault"
            self.state = "idle"

        if self.test_stage == "force_fault":
            self.check_prop_state("idle")
            self.logger.put("Overriding the prop.overpressured fault.")
            self.force_fault("prop.overpressured")
            self.test_stage = "handling_fault"

        elif self.test_stage == "handling_fault":
            self.check_prop_state("handling_fault", "prop state should be in handling fault since prop.overpressured is forced")
            self.logger.put("Prop now in handling_fault.")
            self.test_stage = "venting"

        elif self.test_stage == "venting":
            self.check_prop_state("venting", "prop state should be in venting since prop.overpressured is forced")
            self.logger.put("Prop now in venting.")
            self.check_mission_state("standby", "mission state should be in standby when prop is venting")
            self.logger.put("Satellite now in in standby state.")
            self.test_stage = "finished"

        elif self.test_stage == "finished":
            self.finish()
