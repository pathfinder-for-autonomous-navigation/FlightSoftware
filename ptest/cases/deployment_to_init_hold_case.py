from .base import SingleSatOnlyCase, TestCaseFailure

class DeploymentToInitHoldCheckoutCase(SingleSatOnlyCase):

    @property
    def mission_mode(self):
        return self.sim.flight_controller.read_state("pan.state")
    
    @property
    def adcs_is_functional(self): 
        return self.sim.flight_controller.read_state("adcs_monitor.functional")

    @mission_mode.setter
    def mission_mode(self, value):
        self.sim.flight_controller.write_state("pan.state", value)

    @adcs_is_functional.setter 
    def adcs_is_functional(self, value): 
        assert(value == "true" or value == "false")
        self.sim.flight_controller.write_state("adcs_monitor.functional", value)

    def setup_case_singlesat(self):
        # ADCS wheels are functional
        self.adcs_is_functional = "true"

        self.mission_mode = 0  # Startup state
        self.run_case_singlesat()
        self.logger.put("Deployment to Initialization Hold testcase finished.")

    def run_case_singlesat(self):

        # Wait the entire deployment timer length (100 control cycles)
        for _ in range(100):
            self.cycle()

        # Should transition to detumble if ADCS motors are functional
        self.logger.put("Test case 1: ADCS motors are all functional")
        self.cycle()
        self.cycle()
        if (self.mission_mode != "1"):
            self.logger.put("Failed: Mission Mode moved out of startup")
            self.logger.put(f"Mission mode: {self.mission_mode}")
        else:
            self.logger.put("Passed")

        # ADCS is not functional. This should trigger the adcs functional fault
        # and cause the satellite to move to initialization hold
        self.logger.put("Test case 2: ADCS is not functional")
        self.adcs_is_functional = "false"
        self.cycle()
        if (self.mission_mode != "2"):
            self.logger.put("Failed: Satellite did not move to initialization hold")
            self.logger.put(f"Mission mode: {self.mission_mode}")
        else:
            self.logger.put("Passed")

        # ADCS is not functional. This should trigger the adcs functional fault
        # and cause the satellite to move to initialization hold
        # self.logger.put("Test case 3: ADCS motor 1 is not functional")
        # self.adcs_is_functional = "true"
        # self.cycle()
        # if (self.mission_mode != "2"):
        #     self.logger.put("Failed: Satellite did not move to initialization hold")
        #     self.logger.put(f"Mission mode: {self.mission_mode}")
        # else:
        #     self.logger.put("Passed")

        self.finish()