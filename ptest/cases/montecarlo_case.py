from .base import AMCCase
import time
from psim.sims import DualAttitudeOrbitGnc, DualOrbitGnc
from psim import Configuration, Simulation
import lin
import numpy as np
from .utils import str_to_val, Enums
from typing import NamedTuple
from ..gpstime import GPSTime


class OrbitData(NamedTuple):
    pos: list
    vel: list
    time: list

class MonteCarlo(AMCCase):

    def readDownlinkData(self, satellite):
        pos = str_to_val(satellite.read_state("orbit.pos"))
        vel = str_to_val(satellite.read_state("orbit.vel"))
        time = GPSTime(*(str_to_val(satellite.read_state("time.gps")))).to_list()
        return OrbitData(pos, vel, time)



    #TODO add step size option 
    def run_psim(self, 
                leader_orbit, 
                follower_orbit,
                follower_r_noise=lin.Vector3([0,0,0]), 
                follower_v_noise=lin.Vector3([0,0,0]),
                runtime=11000000000 * 60 * 60 * 24 * 7, # 7 days 
                ):
    

        # get sim configs TODO: get the configs we need
        configs = ["sensors/base", "truth/base", "truth/detumble"]
        configs = ["lib/common/psim/config/parameters/" + f + ".txt" for f in configs]
        config = Configuration(configs)

        # update values to current 
        config["truth.leader.orbit.r"] = lin.Vector3(leader_orbit.pos)
        config["truth.leader.orbit.v"] = lin.Vector3(leader_orbit.vel)

        config["truth.follower.orbit.r"] = lin.Vector3(follower_orbit.pos) + follower_r_noise
        config["truth.follower.orbit.v"] = lin.Vector3(follower_orbit.vel) + follower_v_noise
        config["truth.t.ns"] = GPSTime(*(follower_orbit.time)).to_pan_ns() 


        propagated_follower_orbits = []

        # step sim to desired time
        sim = Simulation(DualOrbitGnc, config)
        while sim["truth.t.ns"] < config["truth.t.ns"] + runtime:
            sim.step()
            
            propagated_follower_orbits.append(OrbitData(
                list(sim["truth.follower.orbit.r"]),
                list(sim["truth.follower.orbit.v"]),
                GPSTime(sim["truth.t.ns"]).to_list(),
            ))

        return propagated_follower_orbits




    def run(self):

        # setup
        self.leader = self.radio_leader
        self.follower = self.radio_follower

        # TODO configs for runtime, step size (log freq), num runs, noise sigma, ...  

        # read the orbit data from each satellite from database
        downlinked_data_vals_leader = self.readDownlinkData(self.leader)
        downlinked_data_vals_follower = self.readDownlinkData(self.follower)

        
        # run without noise
        orbits_no_noise = self.run_psim(downlinked_data_vals_leader, 
                                downlinked_data_vals_follower) 

        # run simulations w/ noise
        all_r_values = [] #list of positions for each trial
        for i in range(1000):
            orbits = self.run_psim(downlinked_data_vals_leader, 
                                downlinked_data_vals_follower,
                                follower_r_noise=lin.Vector3([0,0,0]), #TODO create noise
                                follower_v_noise=lin.Vector3([0,0,0]), 
                )
            curr_r_values = orbits[-1].pos
            all_r_values.append(curr_r_values)
            

        # post-process lists, calculate covariances to output ephemeris
        # TODO fix cov, formatting

        variance_per_step = np.var(all_r_values, axis=0) # we want the variance at each time step over all sims -> but we want a matrix so are we assuming independent or what data is missing here?

        eph_file = open("Ephemeris.txt","a") # TODO correct naming

        for i in range(len(orbits_no_noise)):
            date = orbits_no_noise[i].time #TODO format properly
            pos = orbits_no_noise[i].pos #TODO format properly
            cov = variance_per_step[i] #TODO this is an array of length 3 rn
            
            s = "{date} {pos}\n{cov}\n".format(date, pos, cov)
            eph_file.write(s)

        eph_file.close()
            

        self.logger.put("EXITING MONTECARLO SIMS")
        self.finish()
