from .base import MissionCase
import time
from psim.sims import DualAttitudeOrbitGnc
import lin
from .utils import str_to_val

class AutonomousMissionManagerCase(MissionCase):

    def state_check(self, satellite, designaton):
        satellite_state = satellite.read_state("pan.state")
        if(satellite_state == "3"):
            self.logger.put(designaton + " is in standby. Ending mission." )
            return False
        elif(satellite_state == "10"):
            self.logger.put(designaton + " is in state safehold. Ending mission.")
            return False
        return True
    
    def mission_conditions_met(self):
        #check for operating leader state
        leader_state_functional = self.state_check(self.leader, "Leader")
        follower_state_functional = self.state_check(self.follower, "Follower")
        if not leader_state_functional:
            if follower_state_functional:
                self.follower.write_state("pan.state", 3)
            elif self.follower.read_state("pan.state") != "3":
                self.follower.write_state("pan_state", 10)
            return False

        #check faulting state for follower state
        if not follower_state_functional:
            self.leader.write_state("pan.state", 3)
            return False

        #check time since last comms
        leader_time_since_comms = time.time() - self.leader_time_last_comms 
        if(leader_time_since_comms > self.comms_time_threshold):
            self.logger.put("Leader is experiencing comms blackout. Ending mission." + str(leader_time_since_comms))
            return False
        follower_time_since_comms = time.time() - self.follower_time_last_comms 
        if(follower_time_since_comms > self.comms_time_threshold):
            self.logger.put("Follower is experiencing comms blackout. Ending mission." + str(follower_time_since_comms))
            return False


        #TODO notify if close to docking
        #TODO exit gracefully if mission ended

        return True
    
    def propagate_orbits(self, vals):
        return vals


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
        self.comms_time_threshold = 60*5 #currently 5 minutes for testing

    def run_case_fullmission(self):

        if(self.mission_conditions_met()): 

            #Pass telemetry between spacecraft 

            #wait for data from both spacecrafts to come down from Iridium
            orbit_data_fields = ["orbit.pos", "orbit.vel"]
            while("Unable to find field" in self.leader.read_state("orbit.time") or 
                    "Unable to find field" in self.follower.read_state("orbit.time")): 
                pass

            downlinked_data_vals_leader = [lin.Vector3(str_to_val(self.leader.read_state(field))) for field in orbit_data_fields]
            downlinked_data_vals_follower = [lin.Vector3(str_to_val(self.follower.read_state(field))) for field in orbit_data_fields]
            propagated_data_vals_leader = self.propagate_orbits(downlinked_data_vals_leader)
            propagated_data_vals_follower = self.propagate_orbits(downlinked_data_vals_follower)

            #uplink the leader's data to the follower and vice versa
            baseline_orbit_data_fields = ["orbit.baseline_pos", "orbit.baseline_vel"]
            baseline_orbit_data_vals_follower = [propagated_data_vals_leader[i] - propagated_data_vals_follower[i] for i in range(len(baseline_orbit_data_fields))]
            baseline_orbit_data_vals_leader = [-1*val for val in baseline_orbit_data_vals_follower]

            baseline_orbit_data_vals_follower = [val for val in baseline_orbit_data_vals_follower]
            baseline_orbit_data_vals_leader = [val for val in baseline_orbit_data_vals_leader]

            self.follower.write_multiple_states(baseline_orbit_data_fields, baseline_orbit_data_vals_follower)
            self.leader.write_multiple_states(baseline_orbit_data_fields, baseline_orbit_data_vals_leader)

            self.leader_time_last_comms = float(self.leader.read_state("orbit.time"))
            self.follower_time_last_comms = float(self.follower.read_state("orbit.time"))    
