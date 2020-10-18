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
    
    def avg_list(self, inputList):
        """
        averages list measurements of magnetometers
        """
        sum1 = 0
        sum2 = 0
        sum3 = 0
        for value in inputList:
            for input in value:
                sum1 += input[0]
                sum2 += input[1]
                sum3 += input[2]
        result = [sum1/len(inputList),sum2/len(inputList),sum3/len(inputList)]
        return str(result)
    
    def take_mag_measurements(self, duration):
        """
            Repeatedly take magnetometer measurements for a certain 
            amount of time 
            @param duration: time in seconds of measurement duration
        """
        readings = []
        t_end = time.time() + duration
        while time.time() < t_end:
            pair = [self.rs("adcs_monitor.mag1_vec"),self.rs("adcs_monitor.mag2_vec")]
            self.rs("adcs_cmd.mtr_cmd")
            self.cycle()
            readings.append(pair)
            
        return readings


    def log_white_noise(self):
        """
            Logs field vector readings of magnetometer 
            with autocycling, but no torquer active for 5 seconds.
            Values are entered into a list in pairs with (mag1,mag2)
        """
        
        self.print_header("Testing white noise")

        #record values for 5 seconds
        readings = self.take_mag_measurements(5)
        
        #log matrix and finish procedure
        self.logger.put(self.avg_list(readings))
        #self.logger.put(str(self.average_vectors(readings)))
        #self.logger.put(str(self.stdev_vectors(readings)))
        
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

    def torque_test(self, value):
        """
        Given a value test the magnetorquers individually and then all at once,
        At each command vector, 
            1. read in the magnetometer values while magnetorquer is
            active for 10 seconds
            2. turn off the magnetorquer and read in magnetometer 
            values for 10 seconds
        """
        x = value
        test_matrix = [# get some specifics as to what to test
            [x, 0, 0],
            [0, x, 0],
            [0, 0, x],
            [x, x, x],
        
        ]
        self.print_header("Testing value " + str(x))

        for cmd_array in test_matrix:
            self.print_header("Testing command " + str(cmd_array))
            self.ws("adcs_cmd.mtr_cmd", cmd_array)
            results = self.take_mag_measurements(10)
            
            self.logger.put(self.avg_list(results))

            self.ws("adcs_cmd.mtr_cmd", [0,0,0])
            results = self.take_mag_measurements(10)
            self.logger.put(self.avg_list(results))

    def mtr_test(self):
        """
            Tests the magnetorquer with a matrix of 
            values by turning it for values from 0.025 to 0.1 
            and also for 0.11
        """
        self.print_header("TORQUE TESTS: ")

        for val in range(25, 100, 25): #start at 0.025 and ramp up by 0.025 until at 0.1
            val = val / 1000
            self.torque_test(val)

        #Test for max value = 0.11
        self.torque_test(0.11)


    def run_case_singlesat(self):
        self.print_header("Begin ADCS Magnetorquers Case")
        self.ws("cycle.auto", False)

        # Needed so that ADCSMonitor updates its values
        self.cycle()
        self.ws("dcdc.ADCSMotor_cmd", True)
    
        self.print_header("Finished Initialization")
        #checking that adcs is functional
        self.print_rs("adcs_monitor.functional")

        if not self.read_state("adcs_monitor.functional"):
            self.logger.put("ADCS is NOT functional, Exiting Test Case")
            self.finish()
            return 

        #generate white space data
        self.log_white_noise()

        #run main testing
        self.mtr_test()

        self.print_header("MAGNETORQUER TEST COMPLETE")
        self.ws("adcs_cmd.mtr_cmd", [0,0,0])
        self.finish()
        


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