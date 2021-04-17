from . import SingleSatStandbyCase
from .utils import TestCaseFailure

import lin


class AttitudeFaultHandlerCase(SingleSatStandbyCase):
    """Test the basic functionality of the attitude estimator fault handler.
    """
    def post_boot(self):
        """We need to enable the attitude estimator fault again and disable the
        gyroscope to simulate a sensor failure.
        """
        self.ws("fault_handler.enabled", True)
        self.ws("attitude_estimator.fault.suppress", False)
        self.ws_psim("sensors.leader.gyroscope.disabled", True)

    def run(self):
        """Essentially we're going to disable the gyroscope, ensure that the
        attitude estimator becomes invalid, and check the satellite is in
        standby after ten cycles.
        """
        self.cycle()
        self.cycle()

        if self.rs("attitude_estimator.valid"):
            raise TestCaseFailure(
                    "The attitude estimator shouldn't be valid after disabling the gyroscope")

        # Step for nine more cycles
        for _ in range(19):
            self.cycle()

        # The attitude estimator fault should be tripped
        if not self.rs("attitude_estimator.fault.base"):
            raise TestCaseFailure(
                    "The attitude estimator fault wasn't triggered.")

        # The attitude estimator fault should've forced a transition to safehold
        if self.mission_state != "safehold":
            raise TestCaseFailure(
                    "The satellite was not forced into safehold; it's currently in {}".format(self.mission_state))

        self.finish()
