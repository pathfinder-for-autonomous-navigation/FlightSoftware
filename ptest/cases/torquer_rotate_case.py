from .base import SingleSatOnlyCase, TestCaseFailure
from .utils import Enums

class MTorquerCase(SingleSatOnlyCase):

    def read_state(self, string_state):
        return self.sim.flight_controller.read_state(string_state)
    
    def write_state(self, string_state, state_value):
        self.sim.flight_controller.write_state(string_state, state_value)
        return self.read_state(string_state)

    def log_states(self):
        self.logger.put("<add every state that needs logging here>")

    def prepare_mag(self):
        """
            prepares the magnetorquers for testing by enabling them, 
            waiting and then checking for white noise
        """
        #enables MTorquers
        self.cycle()
        self.ws("adcs_cmd.mtr_mode", MTR_ENABLED)

        # reads in base values for both magnetometers
        time.sleep(2)
        self.print_rs("")

    def finish(self):
        """
            exits test case gracefully
        """   

        self.cycle()
        self.ws("adcs_cmd.mtr_mode", MTR_DISABLED)
        

    def run_case_singlesat(self):
        #change headers
        self.print_header("Begin ADCS Magnetorquers Case")

        self.ws("cycle.auto", False)

        # Needed so that ADCSMonitor updates its values
        self.cycle()
        self.ws("dcdc.ADCSMotor_cmd", True)

        # Necessary so that motor commands are pre-empted by ADCS DCDC being on
        self.cycle()

        self.ws("adcs.state", Enums.adcs_states["point_manual"])
        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_SPEED_CTRL"])
        self.ws("adcs_cmd.rwa_speed_cmd", [0,0,0])
    
        self.print_header("Finished Initialization")
        #checking that adcs is functional
        self.print_rs("adcs_monitor.functional")

        if not self.read_state("adcs_monitor.functional"):
            self.logger.put("ADCS is NOT functional, Exiting Test Case")
            self.finish()
            return 
        


#turn the magnetorquers and log all the fields 
#sim.flightcontroller
#1. build an array of vectors (direction, magnitude) that reads the white noise of the magnetometer sensor
# create a mean and stdev (save to output file or csv)
#2. gather data to determine a (print "expose satellite to a magnetic field from docking phase")
# align long side of satellite with earth's magnetic field
#build script that takes a amount of readings for both magnets vectors
# make sure to follow order of the document precisely
#when looking at wheel checkout case dont autocycle
#in adcscommander set mtr_mode to MTR_ENABLED which allows commanding of magnetorquer
# mtr_cmd takes in vectors to command magnetorquer
#before exiting ptest case set mtr_cmd back to zero