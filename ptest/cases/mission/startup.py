from ..base import DualSatCase, SingleSatCase, PSimCase
from .utils import log_fc_data, log_psim_data


class SingleSatStartupCase(SingleSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(SingleSatStartupCase, self).__init__(*args, **kwargs)

        self.psim_configs += ["truth/deployment"]
        self.initial_state = "startup"

    def run(self):
        self.cycle()

        if not self.is_interactive:
            self.finish()
            return

        log_fc_data(self.flight_controller)
        log_psim_data(self, "leader")


class DualSatStartupCase(DualSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(DualSatStartupCase, self).__init__(*args, **kwargs)

        self.psim_configs += ["truth/deployment"]
        self.leader_initial_state = "startup"
        self.follower_initial_state = "startup"

    def run(self):
        self.cycle()

        if not self.is_interactive:
            self.finish()
            return

        log_fc_data(self.flight_controller_leader)
        log_fc_data(self.flight_controller_follower)
        log_psim_data(self, "leader", "follower")
