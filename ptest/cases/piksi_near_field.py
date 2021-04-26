from .mission import DualSatNearFieldCase
from .utils import Enums, TestCaseFailure
# from .utils import log_fc_data, log_psim_data


class PiksiFaultNearField(DualSatNearFieldCase):
    """Test the basic functionality of the piksi fault handler.
    """
    def post_boot(self):
        """We need to enable the piksi fault handler again.
        """
        self.logger.put(
            "[TESTCASE] Setting leader and follower sat designations.")

        self.flight_controller_leader.write_state("fault_handler.enabled",
                                                  True)
        self.flight_controller_follower.write_state("fault_handler.enabled",
                                                    True)
        self.flight_controller_leader.write_state("piksi_fh.enabled", True)
        self.flight_controller_follower.write_state("piksi_fh.enabled", True)

        self.no_cdpgs_max_wait = self.flight_controller_leader.smart_read(
            "piksi_fh.no_cdpgs_max_wait")
        self.cdpgs_delay_max_wait = self.flight_controller_leader.smart_read(
            "piksi_fh.cdpgs_delay_max_wait")

        self.flight_controller_leader.write_state(
            "pan.sat_designation", Enums.sat_designations["leader"])
        self.flight_controller_follower.write_state(
            "pan.sat_designation", Enums.sat_designations["follower"])

        for _ in range(10):
            self.cycle()

        if Enums.mission_states[self.flight_controller_leader.smart_read(
                "pan.state")] != "leader_close_approach":
            raise TestCaseFailure(
                "The leader's mission state should be in leader close approach."
            )
        if Enums.mission_states[self.flight_controller_follower.smart_read(
                "pan.state")] != "follower_close_approach":
            raise TestCaseFailure(
                "The follower's mission state should be in follower close approach."
            )

        # self.set_fixed_rtk()
        self.cycle()
        print(self.flight_controller_leader.smart_read("piksi.state"))

    def set_fixed_rtk(self):
        self.flight_controller_leader.write_state(
            "piksi.state", Enums.piksi_modes["fixed_rtk"])
        self.flight_controller_follower.write_state(
            "piksi.state", Enums.piksi_modes["fixed_rtk"])

    def set_no_fix(self):
        self.flight_controller_leader.write_state("piksi.state",
                                                  Enums.piksi_modes["no_fix"])
        self.flight_controller_follower.write_state(
            "piksi.state", Enums.piksi_modes["no_fix"])

    def check_no_fix(self):
        if self.flight_controller_leader.smart_read(
                "piksi.state") != Enums.piksi_modes["no_fix"]:
            raise TestCaseFailure(
                "Leader Piksi mode is no longer 'no_fix'. Piksi mode is: " +
                str(self.flight_controller_leader.smart_read("piksi.state")))
        if self.flight_controller_follower.smart_read(
                "piksi.state") != Enums.piksi_modes["no_fix"]:
            raise TestCaseFailure(
                "Follower Piksi mode is no longer 'no_fix'. Piksi mode is: " +
                str(self.flight_controller_follower.smart_read("piksi.state")))

    def check_is_close_appr(self):
        if self.flight_controller_leader.smart_read(
                "pan.state") != Enums.mission_states[
                    "leader_close_approach"]:  # "leader_close_approach":
            raise TestCaseFailure(
                "Failed to set mission state to 'leader_close_approach'. Mission state is: "
                + str(self.flight_controller_leader.smart_read("pan.state")))
        if self.flight_controller_follower.smart_read(
                "pan.state") != Enums.mission_states[
                    "follower_close_approach"]:  # "follower_close_approach":
            raise TestCaseFailure(
                "Failed to set mission state to 'follower_close_approach'. Mission state is: "
                + str(self.flight_controller_follower.smart_read("pan.state")))

    def check_is_standby(self):
        if self.flight_controller_leader.smart_read != "standby":
            raise TestCaseFailure(
                "Failed to set leader mission state to 'standby'.")
        if self.flight_controller_follower.smart_read != "standby":
            raise TestCaseFailure(
                "Failed to set follower mission state to 'standby'.")

    def run(self):
        """Essentially we're going to disable the gyroscope, ensure that the
        attitude estimator becomes invalid, and check the satellite is in
        standby after ten cycles.
        """
        for _ in range(10):
            self.cycle()
            if self.flight_controller_leader.smart_read("piksi.state") != Enums.piksi_modes["fixed_rtk"]:
                raise TestCaseFailure("Lost valid cdgps")
            if self.flight_controller_follower.smart_read("piksi.state") != Enums.piksi_modes["fixed_rtk"]:
                raise TestCaseFailure("Lost valid cdgps")

        self.set_no_fix()
        self.cycle()
        self.check_no_fix()

        for i in range(self.cdpgs_delay_max_wait):
            self.check_is_close_appr()
            print(i)
            self.check_no_fix()
            self.flight_controller_leader.smart_read(
                "sensors.leader.cdgps.disabled")
            self.cycle()
            self.flight_controller_leader.write_state("sensors.leader.cdgps.disabled", True)
            self.flight_controller_follower.write_state("sensors.follower.cdgps.disabled", True)

        self.check_is_standby()
        self.logger.put(
            "PiksiFaultHandler successfully changed mission state to 'standby' after waiting for 'cdpgs_delay_max_wait' cycles.'"
        )
        self.finish()
