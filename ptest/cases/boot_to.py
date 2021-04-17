from .base import SingleSatCase, PSimCase
from .utils import TestCaseFailure
import lin

class BootToStartupCase(SingleSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(BootToStartupCase, self).__init__(*args, **kwargs)

        self.psim_configs += ["truth/deployment"]
        self.initial_state = "startup"

    def run(self):
        if self.mission_state != "startup":
            raise TestCaseFailure("Testcase failed to boot into startup")

        self.finish()


class BootToDetumbleCase(SingleSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(BootToDetumbleCase, self).__init__(*args, **kwargs)

        self.psim_configs += ["truth/detumble"]
        self.initial_state = "detumble"
        self.skip_deployment_wait = True

    def run(self):
        if self.mission_state != "detumble":
            raise TestCaseFailure("Testcase failed to boot into detumble")

        self.finish()


class BootToStandbyCase(SingleSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(BootToStandbyCase, self).__init__(*args, **kwargs)

        self.psim_configs += ["truth/standby"]
        self.psim_config_overrides["truth.leader.attitude.w"] = lin.Vector3([0.01,0.071,-0.01])

        self.initial_state = "standby"
        self.skip_deployment_wait = True

    def run(self):
        if self.mission_state != "standby":
            raise TestCaseFailure("Testcase failed to boot into standby")
        if not self.rs("attitude_estimator.valid"):
            raise TestCaseFailure("Attitude estimator never became valid")

        self.finish()
