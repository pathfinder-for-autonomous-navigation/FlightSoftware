from .base import SingleSatOnlyCase

class DCDCCheckoutCase(SingleSatOnlyCase):

    @property
    def adcs_cmd(self): 
        return self.sim.flight_controller.read_state("dcdc.ADCSMotor_cmd")

    @property
    def sph_cmd(self): 
        return self.sim.flight_controller.read_state("dcdc.SpikeDock_cmd")
    
    @property
    def disable_cmd(self): 
        return self.sim.flight_controller.read_state("dcdc.disable_cmd")

    @property
    def reset_cmd(self): 
        return self.sim.flight_controller.read_state("dcdc.reset_cmd")

    @property
    def adcs_rd(self): 
        return self.sim.flight_controller.read_state("dcdc.ADCSMotor")
    
    @property
    def sph_rd(self): 
        return self.sim.flight_controller.read_state("dcdc.SpikeDock")

    @adcs_cmd.setter 
    def adcs_cmd(self, value): 
        assert(value == "true" or value == "false")
        self.sim.flight_controller.write_state("dcdc.ADCSMotor_cmd", value)

    @sph_cmd.setter 
    def sph_cmd(self, value): 
        assert(value == "true" or value == "false")
        self.sim.flight_controller.write_state("dcdc.SpikeDock_cmd", value)

    @disable_cmd.setter 
    def disable_cmd(self, value): 
        assert(value == "true" or value == "false")
        self.sim.flight_controller.write_state("dcdc.disable_cmd", value)

    @reset_cmd.setter 
    def reset_cmd(self, value): 
        assert(value == "true" or value == "false")
        self.sim.flight_controller.write_state("dcdc.reset_cmd", value)

    def setup_case_singlesat(self):
        self.sim.flight_controller.write_state(
            "pan.state", 11)  # Manual state
        self.run_case_singlesat()
        self.logger.put("DCDC cases finished.")

    def run_case_singlesat(self):
        self.sim.cycle_no = self.sim.flight_controller.read_state("pan.cycle_no")

        if self.adcs_cmd==self.adcs_rd and self.sph_cmd==self.sph_rd and self.disable_cmd=="false" and self.reset_cmd=="false":
            self.logger.put("Control task initialized correctly") 

        print ("Test case 1: Try both DCDCs on. Turn on all systems (ADCS motors, prop valves, docking motor).")

        self.adcs_cmd="true"
        self.sph_cmd="true"

        if self.adcs_rd=="true" and self.sph_rd=="true":
            self.logger.put("Passed")
        else:
            self.logger.put("Failed")

        self.logger.put("Test case 2: ADCS DCDC on, SPH + Prop DCDC off. Turn on all systems.")

        self.adcs_cmd="true"
        self.sph_cmd="false"
        if self.adcs_rd=="true" and self.sph_rd=="false":
            self.reset_cmd="true"
            # Reset takes at least one control cycle to complete
            self.cycle()
            if (self.adcs_rd == "true" and self.sph_rd == "true"):
                self.logger.put("Passed")
            else:
                self.logger.put("Unable to reset pins")
                self.logger.put("ADCS Motor pin: "+self.adcs_rd)
                self.logger.put("Spike and Hold pin: "+self.sph_rd)
        else:
            self.logger.put("Unable to turn ADCS Motor on and SpikeDock off")
            self.logger.put("ADCS Motor pin: "+self.adcs_rd)
            self.logger.put("Spike and Hold pin: "+self.sph_rd)

        self.logger.put("Test Case 3: ADCS DCDC off, SPH + Prop DCDC on. Turn on all systems. ")

        self.adcs_cmd="false"
        self.sph_cmd="true"
        if self.adcs_rd=="false" and self.sph_rd=="true":
            self.reset_cmd="true"
            # Reset takes at least one control cycle to complete
            self.cycle()
            if (self.adcs_rd == "true" and self.sph_rd == "true"):
                self.logger.put("Passed")
            else:
                self.logger.put("Unable to reset pins")
                self.logger.put("ADCS Motor pin: "+self.adcs_rd)
                self.logger.put("Spike and Hold pin: "+self.sph_rd)
        else:
            self.logger.put("Unable to turn ADCS Motor off and SpikeDock on")
            self.logger.put("ADCS Motor pin: "+self.adcs_rd)
            self.logger.put("Spike and Hold pin: "+self.sph_rd)

        self.logger.put("Test Case 4: ADCS DCDC off, SPH + Prop DCDC off. Turn on all systems. ")

        self.disable_cmd="true"
        if self.adcs_rd=="false" and self.sph_rd=="false":
            self.reset_cmd="true"
            # Reset takes at least one control cycle to complete
            self.cycle()
            if (self.adcs_rd == "true" and self.sph_rd == "true"):
                self.logger.put("Passed")
            else:
                self.logger.put("Unable to reset pins")
                self.logger.put("ADCS Motor pin: "+self.adcs_rd)
                self.logger.put("Spike and Hold pin: "+self.sph_rd)
        else:
            self.logger.put("Unable to disable pins")
            self.logger.put("ADCS Motor pin: "+self.adcs_rd)
            self.logger.put("Spike and Hold pin: "+self.sph_rd)

        self.finish()
