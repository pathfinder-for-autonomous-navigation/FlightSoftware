from .base import SingleSatOnlyCase, TestCaseFailure
from .utils import Enums
import math
import time

class MTorquerCase(SingleSatOnlyCase):

    def average_vectors(self, vectorList):
        """
        averages the readings of the white noise list for 
        mag1 and mag2
        """
        sum1 = 0
        sum2 = 0
        for point in vectorList:
            sum1 += point[0]
            sum2 += point[1]
        sum1 = sum1 / len(vectorList)
        sum2 = sum2 / len(vectorList)
        result = {"mag1avg" : sum1, "mag2avg" : sum2} 
        return result
    
    def stdev_vectors(self, vectorList):
        """
        calculates the standard deviation of the vector list
        for mag1 and mag2
        """
        average = self.average_vectors(vectorList)
        sum1 = 0
        sum2 = 0
        for  point in vectorList:
            sum1 += pow((point - average[0]),2)
            sum2 += pow((point - average[1]),2)
        std1 = math.sqrt(sum1 / len(vectorList))
        std2 = math.sqrt(sum2 / len(vectorList))
        result = {"mag1stdev": std1, "mag2stdev": std2}
        return result


    def log_white_noise(self):
        """
            Logs field vector readings of magnetometer 
            with autocycling, but no torquer active for 5 seconds.
            Values are entered into a list in pairs with (mag1,mag2)
        """
        self.ws("cycle.auto", True)
        readings = []

        #record values for 5 seconds
        while time.time() < time.time() + 5: 
            pair = [self.rs("adcs_monitor.mag1_vec"),self.rs("adcs_monitor.mag1_vec")]
            readings.append(pair)
            time.sleep(0.1)
        
        #log matrix and finish procedure
        self.logger.put(readings)
        self.logger.put(self.average_vectors(readings))
        self.logger.put(self.stdev_vectors(readings))
        self.ws("cycle.auto", False)
        self.print_header("Whitespace vector matrix created")

    def prepare_mag(self):
        """
            prepares the magnetorquers for testing by enabling them, 
            waiting and then checking for white noise
        """
        #enables MTorquers
        self.cycle()
        self.ws("adcs_cmd.mtr_mode", 1)#MTR_ENABLED)#how to import this constant

        self.log_white_noise()
    
    def mtr_test(self):
        """
            Turns the mtr at three different torques 
            uses a matrix with different vectors and a 
            certain magnitude in the positive and negative 
            x, y, and z direction then reads in state fields
        """
        x = 0.1
        y = 0.2
        z = 0.3
        test_matrix = [# get some specifics as to what to test
            [x, 0, 0],
            [0, x, 0],
            [0, 0, x],
            [-x, 0, 0],
            [0, -x, 0],
            [0, 0, -x],
            [y, 0, 0],
            [0, y, 0],
            [0, 0, y],
            [-y, 0, 0],
            [0, -y, 0],
            [0, 0, -y],
            [z, 0, 0],
            [0, z, 0],
            [0, 0, z],
            [-z, 0, 0],
            [0, -z, 0],
            [0, 0, -z],
            [y, y, y],
            [-y, -y, -y],
            [x, x, x],
            [-x, -x, -x],
            [z, z, z],
            [-z, -z, -z]
        ]
        self.print_header("TORQUE TESTS: ")

        for cmd_array in test_matrix:
            self.ws("adcs_cmd.mtr_cmd", cmd_array)
            time.sleep(0.2)#change to hold for 10 sec
            pair = [self.rs("adcs_monitor.mag1_vec"),self.rs("adcs_monitor.mag1_vec")]
            self.logger.put(pair)
            self.ws("adcs_cmd.mtr_cmd", [0,0,0])
            time.sleep(0.2)#turn off for 10 sec 
            pair = [self.rs("adcs_monitor.mag1_vec"),self.rs("adcs_monitor.mag1_vec")]
            self.logger.put(pair)
    

    def finish(self):
        """
        exits test case gracefully
        """   
        #take final measurements
        pair = [self.rs("adcs_monitor.mag1_vec"),self.rs("adcs_monitor.mag1_vec")]
        self.logger.put(pair)
        
        self.cycle()
        self.ws("adcs_cmd.mtr_mode", 0)#,MTR_DISABLED)
        self.ws("adcs_cmd.mtr_cmd", [0,0,0])
        super.finish()
        

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