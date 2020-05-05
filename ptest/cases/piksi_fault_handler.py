from .base import SingleSatOnlyCase
from .utils import FSWEnum, Enums, TestCaseFailure, BootUtil

class PiksiFaultHandler_Fast(SingleSatOnlyCase):
    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state(self):
        return "leader"

    @property
    def fast_boot(self):
        return False

    @property
    def one_day_ccno(self):
        return 60 * 1000 // 170

    @property
    def compilation_notice(self):
        return \
        """
        NOTE: This test requires Flight Software to be compiled with the
        \"SPEEDUP\" flag so that the timeouts for the Piksi Fault
        Handler only take minutes, not hours.\n
        """

    def setup_pre_bootsetup(self):
        self.logger.put(self.compilation_notice)

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")
        self.rs("pan.cycle_no")

    def run_case_singlesat(self):
        self.collect_diagnostic_data()

        # TODO implement testcase here.

        self.finish()

class PiksiFaultHandler_Realtime(PiksiFaultHandler_Fast):
    @property
    def compilation_notice(self):
        return \
        """
        NOTE: This test requires Flight Software to be compiled without the
        \"SPEEDUP\" flag so that the timeouts for the Piksi Fault
        Handler are realistic.\n
        """

    @property
    def one_day_ccno(self):
        return 24 * 60 * 1000 // 170
