from .base import SingleSatOnlyCase, TestCaseFailure
from .utils import Enums
import math
import time

class MagLogCase(SingleSatOnlyCase):

    
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

    def torque_test(self):
        """
        For a low wheel spin value read in the magnetometer values while 
        wheels are spinning for 10 seconds
        """
        self.ws( "adcs_cmd.rwa_speed_cmd", [10,10,10] )
        time.sleep(.2)

        self.print_header( "Testing command " + str( [10,10,10] ) )
        results = self.take_mag_measurements( 10 )
        self.logger.put( self.avg_list( results ) )
       
    def setup_post_bootsetup(self):
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
        return 

    def run_case_singlesat(self):

        #run main testing with matrix of command vectors
        self.torque_test()


        


