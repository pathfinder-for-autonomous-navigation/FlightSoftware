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

    # This section is autogenerated by ./pan_generate prop /home/athena/PAN/FlightSoftware/src/fsw/FCCode/PropFaultHandler.cpp

    @property
    def state(self):
        return str(self.read_state("prop.state"))

    @state.setter
    def state(self, val):
        self.write_state("prop.state", str(val))

    @property
    def max_venting_cycles(self):
        return str(self.read_state("prop.max_venting_cycles"))

    @max_venting_cycles.setter
    def max_venting_cycles(self, val):
        self.write_state("prop.max_venting_cycles", str(val))

    @property
    def overpressured_base(self):
        return str(self.read_state("prop.overpressured.base"))

    @overpressured_base.setter
    def overpressured_base(self, val):
        self.write_state("prop.overpressured.base", str(val))

    @property
    def pressurize_fail_base(self):
        return str(self.read_state("prop.pressurize_fail.base"))

    @pressurize_fail_base.setter
    def pressurize_fail_base(self, val):
        self.write_state("prop.pressurize_fail.base", str(val))

    @property
    def tank2_temp_high_base(self):
        return str(self.read_state("prop.tank2_temp_high.base"))

    @tank2_temp_high_base.setter
    def tank2_temp_high_base(self, val):
        self.write_state("prop.tank2_temp_high.base", str(val))

    @property
    def tank1_temp_high_base(self):
        return str(self.read_state("prop.tank1_temp_high.base"))

    @tank1_temp_high_base.setter
    def tank1_temp_high_base(self, val):
        self.write_state("prop.tank1_temp_high.base", str(val))

    def print_object(self):
        print(f"[TESTCASE] state: {self.state}")
        print(f"[TESTCASE] max_venting_cycles: {self.max_venting_cycles}")
        print(f"[TESTCASE] overpressured_base: {self.overpressured_base}")
        print(f"[TESTCASE] pressurize_fail_base: {self.pressurize_fail_base}")
        print(f"[TESTCASE] tank2_temp_high_base: {self.tank2_temp_high_base}")
        print(f"[TESTCASE] tank1_temp_high_base: {self.tank1_temp_high_base}")

    # End autogenerated section

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)
        self.sim.flight_controller.write_state("dcdc.SpikeDock_cmd", True)
        self.sim.flight_controller.write_state(
            "prop.state", Enums.prop_states["disabled"])
        self.cycle()

    def collect_diagnostic_data(self):
        self.prop_state = self.rs("prop.state")
        self.rs("pan.state")
        self.rs("pan.cycle_no")

    # Fake sensor readings

    # Fault Handling Responses

    def run_case_singlesat(self):
        self.collect_diagnostic_data()
        self.finish()
