from .base import MissionCase
import time
from psim.sims import DualAttitudeOrbitGnc, SingleOrbitGnc
from psim import Configuration, Simulation
import lin
from .utils import str_to_val, Enums
from typing import NamedTuple

class OrbitData(NamedTuple):
    pos: lin.Vector3
    vel: lin.Vector3
    time: float
class AutonomousMissionController(MissionCase):

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
    
    def readDownlinkData(self, satellite):
        pos = lin.Vector3(str_to_val(satellite.read_state("orbit.pos")))
        vel = lin.Vector3(str_to_val(satellite.read_state("orbit.vel")))
        time = str_to_val(satellite.read_state("time.gps"))
        return OrbitData(pos, vel, time)

    def writeUplinkData(self, satellite, orbit):
        uplink_orbit_data_fields = ["rel_orbit.uplink.pos", "rel_orbit.uplink.vel", "rel_orbit.uplink.time"]
        satellite.write_multiple_states(uplink_orbit_data_fields, list(orbit))
    
    #default forward propagation time of 10 minutes
    def propagate_orbits(self, orbit, propagation_time = 10*60*1000000000):
        
        #get default sim configs
        configs = ['sensors/base', 'truth/base', 'truth/detumble']
        configs = ['config/parameters/' + f + '.txt' for f in configs]
        #should we use the default truth.dt.ns, or should it depend on something like prop_time?
        config = Configuration(configs)

        #update values to current (sim assumes leader, works equally for follower)
        config['truth.leader.orbit.r'] = orbit.pos
        config['truth.leader.orbit.v'] = orbit.vel
        config['truth.t.ns'] = orbit.time #what are the units on this

        #step sim to desired time
        sim = Simulation(SingleOrbitGnc, config)
        while sim['truth.t.ns'] < config['truth.t.ns'] + propagation_time:
            sim.step()

        #return the sim propagated orbit
        propagatedOrbit = OrbitData(sim['truth.leader.orbit.r'], 
                                    sim['truth.leader.orbit.v'],
                                    sim['truth.t.ns'])
        return propagatedOrbit


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
        return 0
        

    def run_case_fullmission(self):

        #setup
        self.leader = self.radio_leader
        self.follower = self.radio_follower

        self.leader_time_last_comms = time.time()
        self.follower_time_last_comms = time.time()
        self.comms_time_threshold = 60*5 #currently 5 minutes for testing

        #Pass telemetry between spacecraft 
        while(self.continue_mission()): 

            #wait for data from both spacecrafts to come down from Iridium
            while("Unable to find" in self.leader.read_state("time.valid") or 
                    "Unable to find" in self.follower.read_state("time.valid")): 
                pass

            #read the orbit data from each satellite from database
            downlinked_data_vals_leader = self.readDownlinkData(self.leader)
            downlinked_data_vals_follower = self.readDownlinkData(self.follower)

            #propagate the orbits of each satellite using psim 
            propagated_data_vals_leader = self.propagate_orbits(downlinked_data_vals_leader)
            propagated_data_vals_follower = self.propagate_orbits(downlinked_data_vals_follower)

            #uplink the leader's data to the follower and vice versa   
            self.writeUplinkData(self.follower, propagated_data_vals_leader)
            self.writeUplinkData(self.leader, propagated_data_vals_follower)

            #update time of last comms
            self.leader_time_last_comms = str_to_val(self.leader.read_state("time.gps"))
            self.follower_time_last_comms = str_to_val(self.follower.read_state("time.gps"))    

        self.logger.put("EXITING AMC")
        self.finish()
