from ..base import DualSatCase, SingleSatCase, PSimCase
from .utils import log_fc_data, log_psim_data


class SingleSatDetumbleCase(SingleSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(SingleSatDetumbleCase, self).__init__(*args, **kwargs)

        self.psim_configs += ["truth/detumble"]
        self.initial_state = "detumble"
        self.skip_deployment_wait = True

    def run(self):
        if not self.is_interactive:
            self.finish()
            return

        log_fc_data(self.flight_controller)
        log_psim_data(self, "leader")

        self.cycle()


class DualSatDetumbleCase(DualSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(DualSatDetumbleCase, self).__init__(*args, **kwargs)

        self.psim_configs += ["truth/detumble"]
        self.leader_initial_state = "detumble"
        self.follower_initial_state = "detumble"
        self.leader_skip_deployment_wait = True
        self.follower_skip_deployment_wait = True

    def run(self):
        if not self.is_interactive:
            self.finish()
            return

        log_fc_data(self.flight_controller_leader)
        log_fc_data(self.flight_controller_follower)
        log_psim_data(self, "leader", "follower")

        self.cycle()
