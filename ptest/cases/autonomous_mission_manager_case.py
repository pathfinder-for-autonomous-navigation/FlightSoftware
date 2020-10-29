from .base import MissionCase
import time

class AutonomousMissionManagerCase(MissionCase):

    def mission_conditions_met(self):
        #check leader state
        leader_state = self.leader.read_state("pan.state")
        if(leader_state == "3"): # is it 3 or "3"
            self.logger.put("Leader is in standby. Ending mission." )
            return False
        elif(leader_state == "10"):
            self.logger.put("Leader is in state safehold. Ending mission.")
            return False

        #check time since last comms
        leader_time_since_comms = time.time() - self.leader_time_last_comms 
        if(leader_time_since_comms > self.comms_time_threshold):
            self.logger.put("Leader is experiencing comms blackout. Ending mission.")
            return False
        follower_time_since_comms = time.time() - self.follower_time_last_comms 
        if(follower_time_since_comms > self.comms_time_threshold):
            self.logger.put("Follower is experiencing comms blackout. Ending mission.")
            return False

        #check faulting for follower state -> should this also end mission?
        follower_state = self.follower.read_state("pan.state")
        if(follower_state == "3"): # is it 3 or "3"
            self.logger.put("Alert: Follower is in standby.")
            return True
        elif(follower_state == "10"):
            self.logger.put("Alert: Follower is in state safehold.")
            return True

        #TODO notify if close to docking
        #TODO exit gracefully if mission ended

        return True

    def run_case_fullmission(self):

        self.leader = self.sim.flight_controller_leader
        self.follower = self.sim.flight_controller_follower

        self.leader_time_last_comms = time.time()
        self.follower_time_last_comms = time.time()
        self.comms_time_threshold = 60*5 #currently 5 minutes for testing

        while(self.mission_conditions_met()):
            pass 

            #TODO pass telemetry between spacecraft 
            #need some radio sessions to send up Iridium data
            #and send down piksi data
                #I dont know how to use the radios
                #I'm digging through usb session and radio session since they have uplinks and such
            
            #TODO propograte orbits from the data -> how to verify precision?

            

            #check the comms
            if(leader_comms_successful):
                self.leader_time_last_comms = time.time()
            if(follower_comms_successful):
                self.follower_time_last_comms = time.time()
            

        



        

