from .mission import SingleSatStandbyCase
from .utils import Enums, TestCaseFailure


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
        for _ in range(self.rs("attitude_estimator.fault.persistence")):
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


class AttitudeNoSunVectorsInitializationCase(SingleSatStandbyCase):
    """Test the attitude estimator will still initialize without a sun vector.
    """
    def run(self):
        """We'll allow the attitude estimator to initialize, disable the sun
        vector readings, and then force another initialization without the sun
        vector reading.
        """
        self.cycle()
        self.cycle()
        
        # First disable the sunsensor and check that we remain in standby
        self.ws("attitude_estimator.ignore_sun_vectors", True)
        for _ in range(10):
            self.cycle()
            
        if not self.rs("attitude_estimator.valid"):
            raise TestCaseFailure(
                    "The attitude estimator should remain valid, if it was previously valid"
                    ", and then we begin ignoring the sun vectors")
            
        # Disable ignoring the sunvectors, requiring them for the filter to remain valid
        self.ws("attitude_estimator.ignore_sun_vectors", False)
        self.cycle()
        
        # Now disable the sunsensors to test that the filter becomes invalid.
        self.ws_psim("sensors.leader.sun_sensors.disabled", True)
        self.cycle()
        self.cycle()

        if not self.rs("attitude_estimator.valid"):
            raise TestCaseFailure(
                    "The attitude estimator should be valid after disabling the sun sensors")

        if self.rs("adcs_monitor.ssa_mode") != Enums.ssa_modes["SSA_FAILURE"]:
            raise TestCaseFailure(
                    "The sun sensors should be reporting failed readings.")

        self.ws("attitude_estimator.reset_cmd", True)
        self.cycle()

        if self.rs("attitude_estimator.valid"):
            raise TestCaseFailure(
                    "The attitude estimator shouldn't valid after a reset command")

        self.cycle()
        self.cycle()

        if self.rs("attitude_estimator.valid"):
            raise TestCaseFailure(
                    "The attitude estimator still shouldn't valid after a reset command")

        self.ws("attitude_estimator.ignore_sun_vectors", True)
        self.cycle()

        if self.rs("attitude_estimator.valid"):
            raise TestCaseFailure(
                    "The attitude estimator should've reset without sun vector data")

        self.ws("attitude_estimator.ignore_sun_vectors", False)
        self.cycle()
        self.cycle()

        self.finish()

class AttitudePersistanceExceededCase(SingleSatStandbyCase):
    def __init__(self, *args, **kwargs):
        super(AttitudePersistanceExceededCase, self).__init__(*args, **kwargs)
        self.debug_to_console = True


    """Checking if we exceed the covariance frobenius norm's safety with a persistance value and
    reset attitude_estimator if exceeded
    """

    def run(self):
        """We will allow the attitude_estimator to initialize
        """
        self.cycle()
        self.cycle()

        #Basic test to see if the attitude_estimator gets reset
        self.ws("attitude_estimator.safety", 0.0001)
        self.print_rs("attitude_estimator.reset_persistance")

        for _ in range(40):
            #Add something that checks the attitude_estimator every cycle
            self.print_rs("attitude_estimator.valid")
            self.cycle()



