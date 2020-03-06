from .base import SingleSatOnlyCase

class DCDCCheckoutCase(SingleSatOnlyCase):

    @property
    def adcs_cmd(self): 
        return self.simulation.flight_controller.read_state("dcdc.ADCSMotor_cmd")

    @property
    def sph_cmd(self): 
        return self.simulation.flight_controller.read_state("dcdc.SpikeDock_cmd")
    
    @property
    def disable_cmd(self): 
        return self.simulation.flight_controller.read_state("dcdc.disable_cmd")

    @property
    def reset_cmd(self): 
        return self.simulation.flight_controller.read_state("dcdc.reset_cmd")

    @property
    def adcs_rd(self): 
        return self.simulation.flight_controller.read_state("dcdc.ADCSMotor")
    
    @property
    def sph_rd(self): 
        return self.simulation.flight_controller.read_state("dcdc.SpikeDock")

    @adcs_cmd.setter 
    def adcs_cmd(self, value): 
        assert(value == "true" or value == "false")
        self.simulation.flight_controller.write_state("dcdc.ADCSMotor_cmd", value)

    @sph_cmd.setter 
    def sph_cmd(self, value): 
        assert(value == "true" or value == "false")
        self.simulation.flight_controller.write_state("dcdc.SpikeDock_cmd", value)

    @disable_cmd.setter 
    def disable_cmd(self, value): 
        assert(value == "true" or value == "false")
        self.simulation.flight_controller.write_state("dcdc.disable_cmd", value)

    @reset_cmd.setter 
    def reset_cmd(self, value): 
        assert(value == "true" or value == "false")
        self.simulation.flight_controller.write_state("dcdc.reset_cmd", value)

    def setup_case_singlesat(self, simulation):
        self.simulation=simulation
        self.simulation.flight_controller.write_state(
            "pan.state", 11)  # Manual state
        self.run_case_singlesat()
        print("DCDC cases finished.")

    def run_case_singlesat(self):
        self.simulation.cycle_no = self.simulation.flight_controller.read_state("pan.cycle_no")

        if self.adcs_cmd==self.adcs_rd and self.sph_cmd==self.sph_rd and self.disable_cmd=="false" and self.reset_cmd=="false":
            print("Control task initialized correctly") 

        print ("Test case 1: Try both DCDCs on. Turn on all systems (ADCS motors, prop valves, docking motor).")

        self.adcs_cmd="true"
        self.sph_cmd="true"

        if self.adcs_rd=="true" and self.sph_rd=="true":
            print("Passed")
        else:
            print("Failed")

        print("Test case 2: ADCS DCDC on, SPH + Prop DCDC off. Turn on all systems.")

        self.adcs_cmd="true"
        self.sph_cmd="false"
        if self.adcs_rd=="true" and self.sph_rd=="false":
            self.reset_cmd="true"
            # Reset takes at least one control cycle to complete
            self.cycle()
            if (self.adcs_rd == "true" and self.sph_rd == "true"):
                print("Passed")
            else:
                print("Unable to reset pins")
                print("ADCS Motor pin: "+self.adcs_rd)
                print("Spike and Hold pin: "+self.sph_rd)
        else:
            print("Unable to turn ADCS Motor on and SpikeDock off")
            print("ADCS Motor pin: "+self.adcs_rd)
            print("Spike and Hold pin: "+self.sph_rd)

        print("Test Case 3: ADCS DCDC off, SPH + Prop DCDC on. Turn on all systems. ")

        self.adcs_cmd="false"
        self.sph_cmd="true"
        if self.adcs_rd=="false" and self.sph_rd=="true":
            self.reset_cmd="true"
            # Reset takes at least one control cycle to complete
            self.cycle()
            if (self.adcs_rd == "true" and self.sph_rd == "true"):
                print("Passed")
            else:
                print("Unable to reset pins")
                print("ADCS Motor pin: "+self.adcs_rd)
                print("Spike and Hold pin: "+self.sph_rd)
        else:
            print("Unable to turn ADCS Motor off and SpikeDock on")
            print("ADCS Motor pin: "+self.adcs_rd)
            print("Spike and Hold pin: "+self.sph_rd)

        print("Test Case 4: ADCS DCDC off, SPH + Prop DCDC off. Turn on all systems. ")

        self.disable_cmd="true"
        if self.adcs_rd=="false" and self.sph_rd=="false":
            self.reset_cmd="true"
            # Reset takes at least one control cycle to complete
            self.cycle()
            if (self.adcs_rd == "true" and self.sph_rd == "true"):
                print("Passed")
            else:
                print("Unable to reset pins")
                print("ADCS Motor pin: "+self.adcs_rd)
                print("Spike and Hold pin: "+self.sph_rd)
        else:
            print("Unable to disable pins")
            print("ADCS Motor pin: "+self.adcs_rd)
            print("Spike and Hold pin: "+self.sph_rd)