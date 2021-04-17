from .base import SingleSatCase, PSimCase
from .utils import FSWEnum, Enums, TestCaseFailure
from psim.sims import SingleAttitudeOrbitGnc

class PiksiFaultHandler(SingleSatCase, PSimCase):
    def __init__(self, *args, **kwargs):
        super(PiksiFaultHandler, self).__init__(*args, **kwargs)

        self.initial_state = "standby"
        self.psim_configs += ["truth/standby"]

    def post_boot(self):
        self.mission_state = "leader"
        self.cycle()

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")
        self.rs("piksi.last_rtkfix_ccno")
        # self.rs("pan.enter_close_approach_ccno")
        self.rs("pan.cycle_no")

    def run(self):
        self.collect_diagnostic_data()


        # TODO implement testcase here.
        
        # piski faults handled when:
        #     - piski is dead
        #     - last rtk before close approach
        #     -

        if self.mission_state != "leader":
            raise TestCaseFailure("PTest does not intialize mission state to 'leader'.")
        self.ws("piksi.state",Enums.piksi_modes["dead"])
        self.cycle()
        if self.mission_state != "standby":
            raise TestCaseFailure("Failed to set mission state to 'standby' when piksi mode was dead.")

        self.ws("pan.state", Enums.mission_states["leader_close_approach"])
        if self.mission_state != "leader_close_approach":
            raise TestCaseFailure("Failed to set mission state to 'leader_close_approach'.")


        self.finish()
