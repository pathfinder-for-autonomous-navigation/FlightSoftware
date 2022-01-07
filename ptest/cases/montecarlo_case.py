from .base import AMCCase
import time
from psim.sims import DualAttitudeOrbitGnc, DualOrbitGnc
from psim import Configuration, Simulation
import lin
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


    def run_psim(self, leader_orbit, follower_orbit, propagation_time=10 * 60 * 1000000000):

        # run sim for 7 days 
        runtime = 1000000000 * 60 * 60 * 24 * 7


        # get sim configs TODO: get the configs we need
        configs = ["sensors/base", "truth/base", "truth/detumble"]
        configs = ["lib/common/psim/config/parameters/" + f + ".txt" for f in configs]
        config = Configuration(configs)

        # update values to current 
        config["truth.leader.orbit.r"] = lin.Vector3(leader_orbit.pos)
        config["truth.leader.orbit.v"] = lin.Vector3(leader_orbit.vel)
        config["truth.t.ns"] = GPSTime(*(leader_orbit.time)).to_pan_ns() 

        config["truth.follower.orbit.r"] = lin.Vector3(follower_orbit.pos)
        config["truth.follower.orbit.v"] = lin.Vector3(follower_orbit.vel)
        config["truth.t.ns"] = GPSTime(*(follower_orbit.time)).to_pan_ns() 


        # step sim to desired time
        sim = Simulation(DualOrbitGnc, config)
        while sim["truth.t.ns"] < config["truth.t.ns"] + runtime:
            sim.step()
            
            #TODO log intermediate steps to file
            propagated_leader_orbit = OrbitData(
                list(sim["truth.leader.orbit.r"]),
                list(sim["truth.leader.orbit.v"]),
                GPSTime(sim["truth.t.ns"]).to_list(),
            )
            propagated_follower_orbit = OrbitData(
                list(sim["truth.follower.orbit.r"]),
                list(sim["truth.follower.orbit.v"]),
                GPSTime(sim["truth.t.ns"]).to_list(),
            )


    def run(self):

        # setup
        self.leader = self.radio_leader
        self.follower = self.radio_follower


        while 1: #for testing purposes

            # read the orbit data from each satellite from database
            downlinked_data_vals_leader = self.readDownlinkData(self.leader)
            downlinked_data_vals_follower = self.readDownlinkData(self.follower)

            # run simulations
            for i in range(1000):
                self.run_psim()

            

        self.logger.put("EXITING MONTECARLO SIMS")
        self.finish()
