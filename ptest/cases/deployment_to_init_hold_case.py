from .base import SingleSatCase
from .utils import Enums, TestCaseFailure

class DeploymentToInitHold(SingleSatCase):    
    @property
    def initial_state(self):
        return "startup"

    @property
    def adcs_is_functional(self): 
        return self.flight_controller.read_state("adcs_monitor.functional")

    @property
    def wheel1_is_functional(self):
        return self.flight_controller.read_state("adcs_monitor.havt_device10")

    @property
    def wheel2_is_functional(self):
        return self.flight_controller.read_state("adcs_monitor.havt_device11")

    @property
    def wheel3_is_functional(self):
        return self.flight_controller.read_state("adcs_monitor.havt_device12")

    @property
    def wheelpot_is_functional(self):
        return self.flight_controller.read_state("adcs_monitor.havt_device6")

    @property
    def suppress_faults(self):
        return False

    @adcs_is_functional.setter 
    def adcs_is_functional(self, value): 
        assert(value == "true" or value == "false")
        self.flight_controller.write_state("adcs_monitor.functional", value)

    @wheel1_is_functional.setter
    def wheel1_is_functional(self, value):
        assert(value == "true" or value == "false")
        self.flight_controller.write_state("adcs_monitor.havt_device10", value)

    @wheel2_is_functional.setter
    def wheel2_is_functional(self, value):
        assert(value == "true" or value == "false")
        self.flight_controller.write_state("adcs_monitor.havt_device11", value)

    @wheel3_is_functional.setter
    def wheel3_is_functional(self, value):
        assert(value == "true" or value == "false")
        self.flight_controller.write_state("adcs_monitor.havt_device12", value)

    @wheelpot_is_functional.setter
    def wheelpot_is_functional(self, value):
        assert(value == "true" or value == "false")
        self.flight_controller.write_state("adcs_monitor.havt_device6", value)

    def post_boot(self):
        # Move to startup and wait the full deployment length
        self.mission_state = "startup"
        self.logger.put("Now in startup. Cycling through deployment wait...")
        for _ in range(self.one_day_ccno // (24 * 2)):
            self.cycle()
        self.logger.put("Completed deployment wait.")

    def run_case_singlesat(self):
        self.run_case_all_functional()
        self.run_case_adcs_failure()
        self.run_case_wheel1_failure()
        self.run_case_wheel2_failure()
        self.run_case_wheel3_failure()
        self.run_case_wheelpot_failure()
        self.unsignal_faults()
        self.finish()

    def run_case_all_functional(self):
        self.unsignal_faults()
        self.mission_state = "startup"

        # Should transition to detumble if ADCS motors are functional
        self.logger.put("Case 1: ADCS motors are all functional")
        self.cycle()
        if (self.mission_state != "detumble"):
            raise TestCaseFailure(f"Failed: Satellite did not move to detumble. \n Mission mode: {self.mission_state}")
        else:
            self.logger.put("Passed")

    def run_case_adcs_failure(self):
        self.unsignal_faults()

        # Set the ADCS to be disfunctional. Will signal adcs.functional fault
        self.adcs_is_functional = "false" 
        self.cycle()
        self.cycle()

        self.mission_state = "startup"

        # Satellite should move to initialization hold
        self.logger.put("Case 2: ADCS is not functional")
        self.cycle()
        self.check_moved_to_init_hold()

    def run_case_wheel1_failure(self):
        self.unsignal_faults()

        # Set wheel 1 to be disfunctional. Will signal adcs_monitor.wheel1_fault
        self.wheel1_is_functional = "false"
        self.cycle()
        self.cycle()

        self.mission_state = "startup"

        # Satellite should move to initialization hold
        self.logger.put("Case 3: Wheel 1 is not functional")
        self.cycle()
        self.check_moved_to_init_hold()

    def run_case_wheel2_failure(self):
        self.unsignal_faults()

        # Set wheel 2 to be disfunctional. Will signal adcs_monitor.wheel2_fault
        self.wheel2_is_functional = "false"
        self.cycle()
        self.cycle()

        self.mission_state = "startup"

        # Satellite should move to initialization hold
        self.logger.put("Case 4: Wheel 2 is not functional")
        self.cycle()
        self.check_moved_to_init_hold()

    def run_case_wheel3_failure(self):
        self.unsignal_faults()

        # Set wheel 3 to be disfunctional. Will signal adcs_monitor.wheel3_fault
        self.wheel3_is_functional = "false"
        self.cycle()
        self.cycle()

        self.mission_state = "startup"

        # Satellite should move to initialization hold
        self.logger.put("Case 5: Wheel 3 is not functional")
        self.cycle()
        self.check_moved_to_init_hold()

    def run_case_wheelpot_failure(self):
        self.unsignal_faults()

        # Set POT to be disfunctional. Will signal adcs_monitor.wheel_pot_fault
        self.wheelpot_is_functional = "false" 
        self.cycle()
        self.cycle()

        self.mission_state = "startup"

        # Satellite should move to initialization hold
        self.logger.put("Case 6: POT is not functional")
        self.cycle()
        self.check_moved_to_init_hold()

    ######## Helper Functions ###########
    def unsignal_faults(self):
        self.adcs_is_functional = "true"
        self.wheel1_is_functional = "true"
        self.wheel2_is_functional = "true"
        self.wheel3_is_functional = "true"
        self.wheelpot_is_functional = "true"
        self.cycle()

    def check_moved_to_init_hold(self):
        if (self.mission_state != "initialization_hold"):
            raise TestCaseFailure(f"Failed: Satellite did not move to initialization hold. \n Mission mode: {self.mission_state}")
        else:
            self.logger.put("Passed")
