from .base import DualSatCase
import time
from psim.sims import DualAttitudeOrbitGnc
import lin
from .utils import str_to_val, Enums

class AutonomousMissionManagerCase(DualSatCase):

    def state_check(self, satellite, designation):
        satellite_state = satellite.read_state("pan.state")
        if(satellite_state == str(Enums.mission_states["standby"])):
            self.logger.put(designation + " is in standby. Ending mission." )
            return False
        elif(satellite_state == str(Enums.mission_states["safehold"])):
            self.logger.put(designation + " is in safehold. Ending mission.")
            return False
        return True
    
    def continue_mission(self):
        #check for operating leader state
        leader_state_functional = self.state_check(self.leader, "Leader")
        follower_state_functional = self.state_check(self.follower, "Follower")
        if not leader_state_functional:
            if follower_state_functional:
                self.follower.write_state("pan.state", Enums.mission_states["standby"])
            elif self.follower.read_state("pan.state") != str(Enums.mission_states["standby"]):
                self.follower.write_state("pan_state", Enums.mission_states["safehold"])
            return False

        #check faulting state for follower state
        if not follower_state_functional:
            self.leader.write_state("pan.state", Enums.mission_states["standby"])
            return False

        #check time since last comms
        leader_comms_time_diff = time.time() - self.leader_time_last_comms
        if(leader_comms_time_diff > self.comms_time_threshold):
            self.logger.put("Leader is experiencing comms blackout. Ending mission. Time since last comms: " + str(leader_comms_time_diff))
            self.leader.write_state("pan.state", Enums.mission_states["standby"])
            self.follower.write_state("pan.state", Enums.mission_states["standby"])
            return False
        follower_comms_time_diff = time.time() - self.follower_time_last_comms 
        if(follower_comms_time_diff > self.comms_time_threshold):
            self.logger.put("Follower is experiencing comms blackout. Ending mission. Time since last comms: " + str(follower_comms_time_diff))
            self.leader.write_state("pan.state", Enums.mission_states["standby"])
            self.follower.write_state("pan.state", Enums.mission_states["standby"])
            return False

        return True
    
    def propagate_orbits(self, vals):
        return vals

    @property
    def initial_state_leader(self):
        return "leader"
    @property
    def initial_state_follower(self):
        return "follower"

    @property
    def fast_boot_leader(self):
        return True
    @property
    def fast_boot_follower(self):
        return True
        

    def run_case_fullmission(self):

        #setup
        self.using_radios = self.radio_follower != None #assumes only using one physical radio
        self.leader = self.flight_controller_leader
        self.follower = self.radio_follower if self.using_radios else self.flight_controller_follower

        self.leader_time_last_comms = time.time()
        self.follower_time_last_comms = time.time()
        self.comms_time_threshold = 60*5 #currently 5 minutes for testing

        while(self.continue_mission()): 

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

        self.logger.put("EXITING AMC")
        self.finish()
