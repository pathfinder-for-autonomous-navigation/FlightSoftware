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

            #TODO pass telemetry between spacecraft 
            #use radio sessions to send up Iridium data
            #and send down piksi data
            piksi_data_fields = ["piksi.state",
                "piksi.pos",
                "piksi.vel",
                "piksi.baseline_pos",
                "piksi.fix_error_count"]
            piksi_data_vals_leader = [self.flight_controller_leader.rs(field) for field in piksi_data_fields]
            piksi_data_vals_follower = [self.flight_controller_follower.rs(field) for field in piksi_data_fields]

            self.flight_controller_leader.uplink(piksi_data_fields, piksi_data_vals_leader)
            if(self.radio_leader.uplink_queued()):
                leader_comms_successful = self.radio_leader.send_uplink()
                if(leader_comms_successful):
                    self.leader_time_last_comms = time.time()

            self.flight_controller_follower.uplink(piksi_data_fields, piksi_data_vals_follower)
            if(self.radio_follower.uplink_queued()):
                follower_comms_successful = self.radio_follower.send_uplink()
                if(follower_comms_successful):
                    self.follower_time_last_comms = time.time()
            
            
            #TODO propograte orbits from the data -> how to verify precision?        



        

