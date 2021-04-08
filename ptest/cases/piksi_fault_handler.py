from .base import SingleSatOnlyCase
from .utils import FSWEnum, Enums, TestCaseFailure
from psim.sims import SingleAttitudeOrbitGnc

class PiksiFaultHandler(SingleSatOnlyCase):
    @property
    def sim_configs(self):
        configs = ["truth/ci", "truth/base"]
        configs += ["sensors/base"]
        return configs

    @property
    def sim_model(self):
        return SingleAttitudeOrbitGnc

    @property
    def sim_mapping(self):
        return "ci_mapping.json"

    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state(self):
        return "leader"       

    @property
    def fast_boot(self):
        return True

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)
        self.no_cdpgs_max_wait = self.rs("piksi_fh.no_cdpgs_max_wait")
        self.cdpgs_delay_max_wait = self.rs("piksi_fh.cdpgs_delay_max_wait")
        print(self.cdpgs_delay_max_wait)

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")
        self.rs("pan.cycle_no")

    def run_case_singlesat(self):
        self.collect_diagnostic_data()
        self.cycle_no = self.rs("pan.cycle_no")

        # TODO implement testcase here.
        
        if not hasattr(self, "test_stage"):
            self.test_stage = "test_dead_piksi"
            self.cycle_tracker = self.rs("pan.cycle_no") - 1

        if self.test_stage == "test_dead_piksi":
            self.logger.put("Entering test stage 1: 'test_dead_piksi'")
            if self.mission_state != "leader":
                raise TestCaseFailure("PTest does not intialize mission state to 'leader'.")
            self.ws("piksi.state",Enums.piksi_modes["dead"])
            self.cycle()

            if self.mission_state != "standby":
                raise TestCaseFailure("Failed to set mission state to 'standby' when piksi mode was dead.")
            
            self.test_stage = "test_rtkfix_before_close_approach"
            self.cycle_tracker = 0
            self.logger.put("Exiting test stage 1")


        if self.test_stage == "test_rtkfix_before_close_approach":
            if self.cycle_tracker == 0:
                self.logger.put("Entering test stage 2: 'test_rtkfix_before_close_approach'")
                self.ws("piksi.state", Enums.piksi_modes["fixed_rtk"])
                self.last_rtkfix_ccno = self.cycle_no

                # Immediately change piksi mode so this is registered as the last rtkfix cycle
                self.ws("piksi.state", Enums.piksi_modes["no_fix"])
                
                self.ws("pan.state", Enums.mission_states["leader_close_approach"])
                if self.mission_state != "leader_close_approach":
                    raise TestCaseFailure("Failed to set mission state to 'leader_close_approach'.")
                self.close_approach_time = self.cycle_no

            # Debugging
            if self.cycle_tracker <= self.no_cdpgs_max_wait: # self.one_day_ccno:
                print(self.mission_state)

            if self.cycle_tracker > self.no_cdpgs_max_wait: # self.one_day_ccno: 
                if self.mission_state != "standby":
                    raise TestCaseFailure("Failed to set mission state to 'standby' after one day of no rtk signal in 'leader_close_approach'.")
                self.cycle_tracker = 0
                self.test_stage = "test_rtkfix_after_close_approach"
                self.logger.put("Exiting test stage 2")
                
        if self.test_stage == "test_rtkfix_after_close_approach":
            if self.cycle_tracker == 0:
                self.logger.put("Entering test stage 3: 'test_rtkfix_after_close_approach'")
                
                # Should we reset to leader again?
                # self.ws("pan.state", Enums.mission_states["leader"])
                # self.cycle()
                self.ws("pan.state", Enums.mission_states["leader_close_approach"])
                if self.mission_state != "leader_close_approach":
                    raise TestCaseFailure("Failed to set mission state to 'leader_close_approach'.")
                self.close_approach_time = self.cycle_no

            if self.cycle_tracker == 1:
                self.ws("piksi.state", Enums.piksi_modes["fixed_rtk"])
                self.last_rtkfix_ccno = self.cycle_no
                # Immediately change piksi mode so this is registered as the last rtkfix cycle
                self.ws("piksi.state", Enums.piksi_modes["no_fix"])
                
            # Debugging
            if self.cycle_tracker <= self.cdpgs_delay_max_wait:
                print(self.mission_state)

            if self.cycle_tracker > self.cdpgs_delay_max_wait + 1:
                if self.mission_state != "standby":
                    raise TestCaseFailure("Failed to set mission state to 'standby' after one day of no rtk signal in 'leader_close_approach'.")
                self.cycle_tracker = 0
                self.logger.put("Exiting test stage 3: 'test_rtkfix_after_close_approach'")
                self.test_stage = "finished"
                self.finish()

            
        self.cycle_tracker += 1
