from .base import SingleSatCase, PSimCase
from .utils import FSWEnum, Enums, TestCaseFailure

class PiksiFaultHandler(SingleSatCase, PSimCase):
    def __init__(self, *args, **kwargs):
        super(PiksiFaultHandler, self).__init__(*args, **kwargs)
        self.initial_state = "standby"
        self.psim_configs += ["truth/standby"]
        self.debug_to_console = True
        self.skip_deployment_wait = True

    def post_boot(self):
        self.mission_state = "leader"
        self.ws("piksi_fh.dead.suppress", False)
        self.mock_sensor_validity = True
        self.ws("fault_handler.enabled", True)
        self.ws("piksi_fh.enabled", True)
        self.piksi_dead_threshold = self.one_day_ccno//6 + 1
        self.cycle()

    def check_is_leader(self):
        if self.mission_state != "leader":
            raise TestCaseFailure("Failed to set mission state to leader. Mission state is " + self.mission_state + ".")

    def check_is_standby(self):
        if self.mission_state != "standby":
            raise TestCaseFailure("Failed to set mission state to 'standby'.")

    def check_piksi_dead_fault(self, error="crc_error", status=None):
        self.mission_state = "leader"
        self.cycle()
        # Cycle for time until fault would be triggered
        for _ in range(self.piksi_dead_threshold):
            self.check_is_leader()
            self.ws("piksi.state",Enums.piksi_modes[error])
            self.cycle()

        self.check_is_standby()
        
    def run(self):
        """After receiving consecutive piksi errors for piksi_dead_threshold
        cycles, we transition to standby.
        """
        self.logger.put("Running piksi dead test")

        self.check_piksi_dead_fault(error="crc_error")
        self.check_piksi_dead_fault(error="no_data_error")
        self.check_piksi_dead_fault(error="data_error")

        self.logger.put("PiksiFaultHandler successfully changed mission state to 'standby' when piksi mode was dead.")
        self.finish()