from .base import SingleSatCase
from .utils import Enums, TestCaseFailure
import math
import time

class MTorquerCase(SingleSatCase):

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
            amount of time and cycle after each reading
            @param duration: time in seconds of measurement duration
            @return readings: a list of both mag1 and mag2 vector readings
        """
        readings = []
        t_end = time.time() + duration #set duration
        while time.time() < t_end:
            #read in magnetometer states and create pairing
            mag1 = self.rs("adcs_monitor.mag1_vec")
            mag2 = self.rs("adcs_monitor.mag2_vec")
            pair = [ mag1, mag2 ]

            #read in command vector for graphing purposes and cycle
            self.rs( "adcs_cmd.mtr_cmd" )
            self.cycle()

            #add pair to readings list
            readings.append(pair)
            
        return readings


    def log_white_noise(self):
        """
            Logs field vector readings of magnetometer 
            with autocycling, but no torquer active for 5 seconds.
            Values are entered into a list in pairs with (mag1,mag2)
        """
        
        self.print_header( "Testing white noise" )

        #record values for 5 seconds
        readings = self.take_mag_measurements( 5 )
        
        #log matrix and finish procedure
        self.logger.put( self.avg_list( readings ) )
        
        self.print_header( "Whitespace vector matrix created" )

    def prepare_mag(self):
        """
            prepares the magnetorquers for testing by enabling them, 
            waiting and then checking for white noise
        """
        #enables MTorquers
        self.cycle()
        self.ws( "adcs_cmd.mtr_mode", 1 )# 1 == MTR_ENABLED

        self.log_white_noise()

    def torque_test(self, value):
        """
        Given a value test the magnetorquers individually and then all at once,
        At each command vector, 
            1. read in the magnetometer values while magnetorquer is
            active for 10 seconds
            2. turn off the magnetorquer and read in magnetometer 
            values for 10 seconds
        @param value: the value to test for each x, y, z and altogether
        """
        x = value
        test_matrix = [# get some specifics as to what to test
            [x, 0, 0],
            [0, x, 0],
            [0, 0, x],
            [x, x, x],
        
        ]
        self.print_header( "Testing value " + str( x ) )

        for cmd_array in test_matrix:
            self.print_header( "Testing command " + str( cmd_array ) )
            self.ws( "adcs_cmd.mtr_cmd", cmd_array )
            results = self.take_mag_measurements( 10 )
            
            self.logger.put( self.avg_list( results ) )

            self.ws( "adcs_cmd.mtr_cmd", [0,0,0] )
            results = self.take_mag_measurements( 10 )
            self.logger.put( self.avg_list( results ) )

    def mtr_test(self):
        """
            Tests the magnetorquer with a matrix of 
            values by turning it for values from 0.025 to 0.1 
            and also for 0.11
        """
        self.print_header( "TORQUE TESTS: " )
        for val in range( 125, 501, 125 ): #start at 0.0125 and ramp up by 0.0125 to 0.05 inclusive
            val = val / 10000
            self.torque_test( val )

        #Test for max value: from src/adcs/constants.hpp (truncated)
        self.torque_test( 0.056668)

    def post_boot(self):
        self.mission_state = "manual"
        self.cycle()

    def run(self):
        self.print_header( "Begin ADCS Magnetorquers Case" )
        self.ws( "cycle.auto", False )

        # Needed so that ADCSMonitor updates its values
        self.cycle()
        self.print_ws( "dcdc.ADCSMotor_cmd", True )

        self.print_ws("pan.state", Enums.mission_states["manual"])
        self.print_ws("adcs.state", Enums.adcs_states["manual"])	
        self.print_ws("adcs_cmd.mtr_mode", Enums.mtr_modes["MTR_ENABLED"])	
        
        self.cycle()
        self.print_header( "Finished Initialization" )
        #checking that adcs is functional

        if not self.print_rs( "adcs_monitor.functional" ):
            self.logger.put( "ADCS is NOT functional, Exiting Test Case" )
            self.finish()
            return 

        #generate white space data
        self.log_white_noise()

        #run main testing with matrix of command vectors
        self.mtr_test()

        #finish and turn off magnetorquers
        self.print_header( "MAGNETORQUER TEST COMPLETE" )
        self.ws( "adcs_cmd.mtr_cmd", [0,0,0] )
        self.finish()
        


