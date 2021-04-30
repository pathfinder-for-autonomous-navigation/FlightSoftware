from ..base import DualSatCase, SingleSatCase, PSimCase
from ..utils import TestCaseFailure
from .utils import log_fc_data, log_psim_data


class SingleSatStandbyCase(SingleSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(SingleSatStandbyCase, self).__init__(*args, **kwargs)

        self.psim_configs += ["truth/standby"]
        self.initial_state = "standby"
        self.skip_deployment_wait = True

    def run(self):
        self.cycle()

        if not self.is_interactive:
            if not self.flight_controller.smart_read("attitude_estimator.valid"):
                raise TestCaseFailure("Attitude estimator failed to initialize.")

            self.finish()
            return

        log_fc_data(self.flight_controller)
        log_psim_data(self, "leader")


class DualSatStandbyCase(DualSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(DualSatStandbyCase, self).__init__(*args, **kwargs)

        self.psim_configs += ["truth/standby"]
        self.leader_initial_state = "standby"
        self.follower_initial_state = "standby"
        self.leader_skip_deployment_wait = True
        self.follower_skip_deployment_wait = True

    def run(self):
        self.cycle()

        if not self.is_interactive:
            if not self.flight_controller_leader.smart_read("attitude_estimator.valid"):
                raise TestCaseFailure("Leader attitude estimator failed to initialize.")
            if not self.flight_controller_follower.smart_read("attitude_estimator.valid"):
                raise TestCaseFailure("Follower attitude estimator failed to initialize.")

            self.finish()
            return

        log_fc_data(self.flight_controller_leader)
        log_fc_data(self.flight_controller_follower)
        log_psim_data(self, "leader", "follower")
