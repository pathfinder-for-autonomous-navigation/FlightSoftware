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

    def get_sigmas_hardcoded(self):
        '''Hardcoded version of sigmas pos, vel, pulled from first few cycles of SingleSatStanby'''
        pos_sigma = [1.369557, 1.369557, 1.369559]
        vel_sigma = [0.363415, 0.363415, 0.363415]
        return pos_sigma, vel_sigma

    def get_sigmas(self):
        pos_sigma = str_to_val(self.read_state("orbit.pos_sigma"))
        vel_sigma = str_to_val(self.read_state("orbit.vel_sigma"))
        return pos_sigma, vel_sigma

    def read_downlink_data_hardcoded_leader(self):
        '''Hardcoded version of get pos, vel, time, pulled from first few cycles of SingleSatStanby'''
        pos = [-3394650.605799, -3784598.685641, -4595138.924735]
        vel = [6165.990371, -3462.978608, -1700.082949]
        time = [2045, 3942980, 0]
        return OrbitData(pos, vel, time)
    
    def read_downlink_data_hardcoded_follower(self):
        pos = [-3392661.953954, -3785470.995339, -4595393.482806]
        vel = [6167.019697, -3462.423257, -1698.505773]
        time = [2045, 3942980, 0]
        return OrbitData(pos, vel, time)

    def readDownlinkData(self, satellite):
        pos = str_to_val(satellite.read_state("orbit.pos"))
        vel = str_to_val(satellite.read_state("orbit.vel"))
        time = GPSTime(*(str_to_val(satellite.read_state("time.gps")))).to_list()
        return OrbitData(pos, vel, time)

    #TODO add step size option 
    def run_psim(self, 
                leader_orbit, 
                follower_orbit,
                follower_r_noise = lin.Vector3([0,0,0]), 
                follower_v_noise = lin.Vector3([0,0,0]),
                # runtime=1000000000 * 60 * 60 * 24 * 7, # 7 days 
                runtime=1000000000 * 60 * 60, # 1 hr
                ):
    
        # get sim configs TODO: get the configs we need
        configs = ["sensors/base", "truth/base", "truth/detumble"]
        configs = ["lib/common/psim/config/parameters/" + f + ".txt" for f in configs]
        config = Configuration(configs)

        # update values to current 
        config["truth.leader.orbit.r"] = lin.Vector3(leader_orbit.pos)
        config["truth.leader.orbit.v"] = lin.Vector3(leader_orbit.vel)

        print(follower_r_noise)
        
        print(follower_orbit.pos)
        print(lin.Vector3(follower_orbit.pos))

        config["truth.follower.orbit.r"] = lin.Vector3(follower_orbit.pos) + follower_r_noise
        config["truth.follower.orbit.v"] = lin.Vector3(follower_orbit.vel) + follower_v_noise
        config["truth.t.ns"] = GPSTime(*(follower_orbit.time)).to_pan_ns() 

        propagated_follower_orbits = []

        # step sim to desired time
        sim = Simulation(DualOrbitGnc, config)
        steps = 0
        while sim["truth.t.ns"] < config["truth.t.ns"] + runtime:
            sim.step()
            
            propagated_follower_orbits.append(OrbitData(
                list(sim["truth.follower.orbit.r"]),
                list(sim["truth.follower.orbit.v"]),
                GPSTime(sim["truth.t.ns"]).to_list(),
            ))
            steps += 1
            
            if steps % 100000 == 0:
                print(steps)
                print(sim['truth.t.ns'])
                print(config["truth.t.ns"] + runtime)

        return propagated_follower_orbits

    def batch_convert_to_eci(self, orbit_data):
        pass

    def run(self):
        print("Running Monte Carlo")

        # setup
        self.leader = self.radio_leader
        self.follower = self.radio_follower

        # TODO configs for runtime, step size (log freq), num runs, noise sigma, ...  

        # read the orbit data from each satellite from database
        # downlinked_data_vals_leader = self.readDownlinkData(self.leader)
        # downlinked_data_vals_follower = self.readDownlinkData(self.follower)

        downlinked_data_vals_leader = self.read_downlink_data_hardcoded_leader()
        downlinked_data_vals_follower = self.read_downlink_data_hardcoded_follower()
        
        # run without noise
        orbits_no_noise = self.run_psim(downlinked_data_vals_leader, 
                                downlinked_data_vals_follower) 

        pos_sigma, vel_sigma = self.get_sigmas_hardcoded()

        # run simulations w/ noise
        all_r_values = [] #list of positions for each trial
        for i in range(10):
            orbits = self.run_psim(downlinked_data_vals_leader, 
                                downlinked_data_vals_follower,
                                follower_r_noise=lin.Vector3(np.random.normal(scale=pos_sigma)),
                                follower_v_noise=lin.Vector3(np.random.normal(scale=vel_sigma)), 
                )
            curr_r_values = orbits[-1].pos
            all_r_values.append(curr_r_values)
            
            if i % 1 == 0:
                print(f"Finished {i} simulations")

        # post-process lists, calculate covariances to output ephemeris

        np_all_r_values = np.array(all_r_values).transpose(1, 2, 0)
        covariance_per_step = [np.cov(step_positions) for step_positions in np_all_r_values]

        eph_file = open("Ephemeris.txt","a") # TODO correct naming

        for i in range(len(orbits_no_noise)):
            date = orbits_no_noise[i].time #TODO format properly
            pos = orbits_no_noise[i].pos #TODO format properly
            cov = covariance_per_step[i]
            
            s = "{date} {pos}\n{cov}\n".format(date, pos, cov)
            eph_file.write(s)

        eph_file.close()

        self.logger.put("EXITING MONTECARLO SIMS")
        self.finish()
