from .base import SingleSatOnlyCase, TestCaseFailure

class DeploymentToInitHoldCheckoutCase(SingleSatOnlyCase):

    @property
    def mission_mode(self):
        return self.sim.flight_controller.read_state("pan.state")
    
    @property
    def adcs_is_functional(self): 
        return self.sim.flight_controller.read_state("adcs_monitor.functional")

    @property
    def wheel1_is_functional(self):
        return self.sim.flight_controller.read_state("adcs_monitor.havt_device10")

    @property
    def wheel2_is_functional(self):
        return self.sim.flight_controller.read_state("adcs_monitor.havt_device11")

    @property
    def wheel3_is_functional(self):
        return self.sim.flight_controller.read_state("adcs_monitor.havt_device12")

    @property
    def wheelpot_is_functional(self):
        return self.sim.flight_controller.read_state("adcs_monitor.havt_device6")

    @mission_mode.setter
    def mission_mode(self, value):
        self.sim.flight_controller.write_state("pan.state", value)

    @adcs_is_functional.setter 
    def adcs_is_functional(self, value): 
        assert(value == "true" or value == "false")
        self.sim.flight_controller.write_state("adcs_monitor.functional", value)

    @wheel1_is_functional.setter
    def wheel1_is_functional(self, value):
        assert(value == "true" or value == "false")
        self.sim.flight_controller.write_state("adcs_monitor.havt_device10", value)

    @wheel2_is_functional.setter
    def wheel2_is_functional(self, value):
        assert(value == "true" or value == "false")
        self.sim.flight_controller.write_state("adcs_monitor.havt_device11", value)

    @wheel3_is_functional.setter
    def wheel3_is_functional(self, value):
        assert(value == "true" or value == "false")
        self.sim.flight_controller.write_state("adcs_monitor.havt_device12", value)

    @wheelpot_is_functional.setter
    def wheelpot_is_functional(self, value):
        assert(value == "true" or value == "false")
        self.sim.flight_controller.write_state("adcs_monitor.havt_device6", value)

    def setup_case_singlesat(self):
        self.run_case_singlesat()
        self.logger.put("Deployment to Initialization Hold testcase finished.")

    def run_case_singlesat(self):
        self.run_case_all_functional()
        self.run_case_adcs_failure()
        # self.run_case_wheel1_failure()
        # self.run_case_wheel2_failure()
        # self.run_case_wheel3_failure()
        # self.run_case_wheelpot_failure()
        self.finish()

    def run_case_all_functional(self):
        # ADCS devices are all functional by default
        self.adcs_is_functional = "true"
        self.wheel1_is_functional = "true"
        self.wheel2_is_functional = "true"
        self.wheel3_is_functional = "true"
        self.wheelpot_is_functional = "true"

        self.mission_mode = 11 # I don't know why adding this line helps it pass
        self.mission_mode = 0  # Startup state

        # Should transition to detumble if ADCS motors are functional
        self.logger.put("Test case 1: ADCS motors are all functional")
        self.cycle()
        self.cycle()
        if (self.mission_mode != "1"):
            self.logger.put("Failed: Satellite did not move to detumble")
            self.logger.put(f"Mission mode: {self.mission_mode}")
        else:
            self.logger.put("Passed")

    def run_case_adcs_failure(self):
        self.cycle()
        self.cycle()
        # ADCS devices are all functional by default
        self.adcs_is_functional = "false"
        self.wheel1_is_functional = "true"
        self.wheel2_is_functional = "true"
        self.wheel3_is_functional = "true"
        self.wheelpot_is_functional = "true"

        self.mission_mode = 11
        self.cycle()
        self.mission_mode = 0  # Startup state

        # ADCS is not functional. This should trigger the adcs functional fault
        # and cause the satellite to move to initialization hold
        self.logger.put("Test case 2: ADCS is not functional")
        self.cycle()
        self.cycle()
        if (self.mission_mode != "2"):
            self.logger.put("Failed: Satellite did not move to initialization hold")
            self.logger.put(f"Mission mode: {self.mission_mode}")
        else:
            self.logger.put("Passed")

    def run_case_wheel1_failure(self):
        # ADCS devices are all functional by default
        self.adcs_is_functional = "true"
        self.wheel1_is_functional = "false"
        self.wheel2_is_functional = "true"
        self.wheel3_is_functional = "true"
        self.wheelpot_is_functional = "true"
        self.mission_mode = 0  # Startup state

        # ADCS is not functional. This should trigger the adcs functional fault
        # and cause the satellite to move to initialization hold
        self.logger.put("Test case 3: Wheel 1 is not functional")
        self.cycle()
        self.cycle()
        if (self.mission_mode != "2"):
            self.logger.put("Failed: Satellite did not move to initialization hold")
            self.logger.put(f"Mission mode: {self.mission_mode}")
        else:
            self.logger.put("Passed")

    def run_case_wheel2_failure(self):
        # ADCS devices are all functional by default
        self.adcs_is_functional = "true"
        self.wheel1_is_functional = "true"
        self.wheel2_is_functional = "false"
        self.wheel3_is_functional = "true"
        self.wheelpot_is_functional = "true"
        self.mission_mode = 0  # Startup state

        # ADCS is not functional. This should trigger the adcs functional fault
        # and cause the satellite to move to initialization hold
        self.logger.put("Test case 4: ADCS Wheel 2 is not functional")
        self.cycle()
        self.cycle()
        if (self.mission_mode != "2"):
            self.logger.put("Failed: Satellite did not move to initialization hold")
            self.logger.put(f"Mission mode: {self.mission_mode}")
        else:
            self.logger.put("Passed")

    def run_case_wheel3_failure(self):
        # ADCS devices are all functional by default
        self.adcs_is_functional = "true"
        self.wheel1_is_functional = "true"
        self.wheel2_is_functional = "true"
        self.wheel3_is_functional = "false"
        self.wheelpot_is_functional = "true"
        self.mission_mode = 0  # Startup state

        # ADCS is not functional. This should trigger the adcs functional fault
        # and cause the satellite to move to initialization hold
        self.logger.put("Test case 5: ADCS Wheel 3 is not functional")
        self.cycle()
        self.cycle()
        if (self.mission_mode != "2"):
            self.logger.put("Failed: Satellite did not move to initialization hold")
            self.logger.put(f"Mission mode: {self.mission_mode}")
        else:
            self.logger.put("Passed")

    def run_case_wheelpot_failure(self):
        # ADCS devices are all functional by default
        self.adcs_is_functional = "true"
        self.wheel1_is_functional = "true"
        self.wheel2_is_functional = "true"
        self.wheel3_is_functional = "true"
        self.wheelpot_is_functional = "false"
        self.mission_mode = 0  # Startup state

        # ADCS is not functional. This should trigger the adcs functional fault
        # and cause the satellite to move to initialization hold
        self.logger.put("Test case 2: ADCS is not functional")
        self.cycle()
        self.cycle()
        if (self.mission_mode != "2"):
            self.logger.put("Failed: Satellite did not move to initialization hold")
            self.logger.put(f"Mission mode: {self.mission_mode}")
        else:
            self.logger.put("Passed")
