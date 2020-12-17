from .base import MissionCase
import time
from psim.sims import DualAttitudeOrbitGnc
import lin
from .utils import str_to_val

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
            self.logger.put("Leader is experiencing comms blackout. Ending mission." + str(leader_time_since_comms))
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


    @property
    def sim_configs(self):
        configs = ["truth/ci", "truth/base"]
        configs += ["sensors/base"]
        return configs

    @property
    def sim_model(self):
        return DualAttitudeOrbitGnc

    @property
    def sim_mapping(self):
        return "ci_mapping.json"
        
    @property
    def sim_duration(self):
        return float("inf")

    def setup_post_bootsetup_follower(self):
        self.using_radios = self.radio_follower != None #assumes only using one physical radio
        self.leader = self.flight_controller_leader
        self.follower = self.radio_follower if self.using_radios else self.flight_controller_follower

        self.leader_time_last_comms = time.time()
        self.follower_time_last_comms = time.time()
        self.comms_time_threshold = 60*5*1000000000 #currently 5 minutes for testing

    def run_case_fullmission(self):

        if(self.mission_conditions_met()): 

            #Pass telemetry between spacecraft 

            #wait for data from both spacecrafts to come down from Iridium
            orbit_data_fields = ["orbit.pos", "orbit.vel"]
            while(self.leader.read_state("orbit.time") == None): #what does es actually respond with?
                pass

            downlinked_data_vals_leader = [lin.Vector3(str_to_val(self.leader.read_state(field))) for field in orbit_data_fields]
            downlinked_data_vals_follower = [lin.Vector3(str_to_val(self.follower.read_state(field))) for field in orbit_data_fields]

            #uplink the leader's data to the follower and vice versa
            baseline_orbit_data_fields = ["orbit.baseline_pos", "orbit.baseline_vel"]
            baseline_orbit_data_vals_follower = [downlinked_data_vals_leader[i] - downlinked_data_vals_follower[i]
                                                                             for i in range(len(baseline_orbit_data_fields))]
            baseline_orbit_data_vals_leader = [-1*val for val in baseline_orbit_data_vals_follower]

            baseline_orbit_data_vals_follower = [val for val in baseline_orbit_data_vals_follower]
            baseline_orbit_data_vals_leader = [val for val in baseline_orbit_data_vals_leader]

            self.follower.write_multiple_states(baseline_orbit_data_fields, baseline_orbit_data_vals_follower)
            self.leader.write_multiple_states(baseline_orbit_data_fields, baseline_orbit_data_vals_leader)

            self.leader_time_last_comms = float(self.leader.read_state("orbit.time"))
            self.follower_time_last_comms = float(self.follower.read_state("orbit.time"))

            
            
            
            #TODO propagate orbits from the data -> how to verify precision?      
 



        
