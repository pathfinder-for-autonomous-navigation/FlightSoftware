from .base import MissionCase
import time

class AutonomousMissionManagerCase(MissionCase):

    def mission_conditions_met(self):
        #check leader state
        leader_state = self.leader.read_state("pan.state")
        if(leader_state == "3"): 
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
        if(follower_state == "3"):
            self.logger.put("Alert: Follower is in standby.")
            return True
        elif(follower_state == "10"):
            self.logger.put("Alert: Follower is in state safehold.")
            return True

        #TODO notify if close to docking
        #TODO exit gracefully if mission ended

        return True

    def run_case_fullmission(self):

        self.using_radios = self.radio_follower != None
        self.leader = self.radio_leader if self.using_radios else self.flight_controller_leader
        self.follower = self.radio_follower if self.using_radios else self.flight_controller_follower

        self.leader_time_last_comms = time.time()
        self.follower_time_last_comms = time.time()
        self.comms_time_threshold = 60*5 #currently 5 minutes for testing

        while(self.mission_conditions_met()): 

            #Pass telemetry between spacecraft 

            #wait for leader's data to come down from Iridium (automatically sent)
            orbit_data_fields = ["orbit.pos", "orbit.vel", "orbit.time"]
            while(self.leader.read_state("orbit.time") == None): #what does es actually respond with?
                pass

            downlinked_data_vals_leader = [self.leader.read_state(field) for field in orbit_data_fields]

            #uplink the leader's data to the follower
            self.follower.write_multiple_states(orbit_data_fields, downlinked_data_vals_leader)

            self.leader_time_last_comms = int(self.leader.read_state("orbit.time"))
            self.follower_time_last_comms = int(self.follower.read_state("orbit.time"))
            
            
            #TODO propograte orbits from the data -> how to verify precision?        



        

