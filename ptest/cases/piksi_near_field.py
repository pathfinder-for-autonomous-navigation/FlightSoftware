from .mission import DualSatNearFieldCase
from .utils import Enums, TestCaseFailure


class PiksiFaultNearField(DualSatNearFieldCase):
    """Test the basic functionality of the piksi fault handler.
    """
    def __init__(self, *args, **kwargs):
        super(PiksiFaultNearField, self).__init__(*args, **kwargs)
        self.debug_to_console = False

    def post_boot(self):
        """We need to enable the piksi fault handler again and call near field's post_boot function.
        """
        self.flight_controller_leader.write_state("fault_handler.enabled", True)
        self.flight_controller_follower.write_state("fault_handler.enabled", True)
        self.flight_controller_leader.write_state("piksi_fh.enabled", True)
        self.flight_controller_follower.write_state("piksi_fh.enabled", True)

        self.cdpgs_delay_max_wait = self.flight_controller_leader.smart_read("piksi_fh.cdpgs_delay_max_wait")

        super(PiksiFaultNearField, self).post_boot()

    def check_no_fix(self):
        if self.flight_controller_leader.smart_read("piksi.state") == Enums.piksi_modes["fixed_rtk"]:
            raise TestCaseFailure("Leader Piksi mode is still 'fixed_rtk'")
        if self.flight_controller_follower.smart_read("piksi.state") == Enums.piksi_modes["fixed_rtk"]:
            raise TestCaseFailure("Follower Piksi mode is still 'fixed_rtk'")

    def check_is_close_appr(self):
        if self.mission_state_leader != "leader_close_approach":
            raise TestCaseFailure("Failed to set mission state to 'leader_close_approach'. Mission state is: " + str(self.mission_state_leader))
        if self.mission_state_follower != "follower_close_approach":
            raise TestCaseFailure("Failed to set mission state to 'follower_close_approach'. Mission state is: " + str(self.mission_state_follower))

    def check_is_standby(self):
        if self.mission_state_leader != "standby":
            raise TestCaseFailure("Failed to set leader mission state to 'standby'. Mission state is: " + str(self.mission_state_leader))
        if self.mission_state_follower != "standby":
            raise TestCaseFailure("Failed to set follower mission state to 'standby'. Mission state is: " + str(self.mission_state_follower))

    def run(self):
        """After cycling for an arbitrary amount of time in close_approach, if 
        we lose cdgps readings for more than cdpgs_delay_max_wait cycles, we
        transition to standby.
        """
        for _ in range(10):
            self.cycle()

        # Shut off cdgps readings
        self.ws_psim("sensors.leader.cdgps.disabled", True)
        self.ws_psim("sensors.follower.cdgps.disabled", True)
        
        self.cycle()

        for _ in range(self.cdpgs_delay_max_wait + 1):
            self.check_is_close_appr()
            self.check_no_fix()
            self.cycle()

        self.check_is_standby()
        self.logger.put("PiksiFaultHandler successfully changed mission state to 'standby' after waiting for 'cdpgs_delay_max_wait' cycles.'")
        self.finish()
