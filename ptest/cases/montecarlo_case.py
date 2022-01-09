from .base import AMCCase
import time
from psim.sims import DualAttitudeOrbitGnc, DualOrbitGnc
from psim import Configuration, Simulation
import lin
import numpy as np
from .utils import str_to_val, Enums
from typing import NamedTuple
from ..gpstime import GPSTime
from .conversions import ecef2eci 

HARDCODED = True  # TODO CHANGE OUT OF HARDCODED
RUNTIME = 1000000000 * 60 * 60 # 1 hr # TODO INCREASE TIME BACK TO 7 Days
# RUNTIME = 1000000000 * 60 * 60 * 24 * 7, # 7 days 
STEPS_PER_MIN = int(1000000000 * 60 / 170) # Number of simulation steps in one minute 
STEPS_PER_LOG_ENTRY = STEPS_PER_MIN # How often to log positions and time
NUM_MC_RUNS = 10


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
        if HARDCODED:
            return self.get_sigmas_hardcoded()
        pos_sigma = str_to_val(self.read_state("orbit.pos_sigma")) # TODO should specify which sat here?
        vel_sigma = str_to_val(self.read_state("orbit.vel_sigma"))
        return pos_sigma, vel_sigma

    def read_downlink_data_leader_hardcoded(self):
        '''Hardcoded version of get pos, vel, time, pulled from first few cycles of SingleSatStanby'''
        pos = [-3394650.605799, -3784598.685641, -4595138.924735]
        vel = [6165.990371, -3462.978608, -1700.082949]
        time = [2045, 3942980, 0]
        return OrbitData(pos, vel, time)
    
    def read_downlink_data_follower_hardcoded(self):
        pos = [-3392661.953954, -3785470.995339, -4595393.482806]
        vel = [6167.019697, -3462.423257, -1698.505773]
        time = [2045, 3942980, 0]
        return OrbitData(pos, vel, time)

    def readDownlinkData(self, satellite):
        if HARDCODED:
            if satellite == self.leader:
                return self.read_downlink_data_leader_hardcoded()
            else:
                return self.read_downlink_data_follower_hardcoded()

        pos = str_to_val(satellite.read_state("orbit.pos"))
        vel = str_to_val(satellite.read_state("orbit.vel"))
        time = GPSTime(*(str_to_val(satellite.read_state("time.gps")))).to_list()
        return OrbitData(pos, vel, time)

    def run_psim(self, 
                 leader_orbit, 
                 follower_orbit,
                 follower_r_noise = lin.Vector3([0,0,0]), 
                 follower_v_noise = lin.Vector3([0,0,0]),
                 runtime=RUNTIME
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

            if steps % STEPS_PER_LOG_ENTRY == 0:
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

    def generate_monte_carlo_data(self, downlinked_leader, downlinked_follower, pos_sigma, vel_sigma):
        # run simulations w/ noise
        monte_carlo_position_runs = [] #list of positions for each trial
        for i in range(NUM_MC_RUNS):
            list_of_orbit_data = self.run_psim(downlinked_leader, 
                                                downlinked_follower,
                                                follower_r_noise=lin.Vector3(np.random.normal(scale=pos_sigma)),
                                                follower_v_noise=lin.Vector3(np.random.normal(scale=vel_sigma)), 
                )
            list_of_orbit_positions = [x.pos for x in list_of_orbit_data]
            monte_carlo_position_runs.append(list_of_orbit_positions)
            
            if i % 1 == 0:
                print(f"Finished {i} simulations")
                
        return monte_carlo_position_runs

    def batch_convert_to_eci(self, ecef_positions, times):
        return ecef_positions
        # TODO verify frames and units
        # TODO what is weeknum
        eci_positions = [ecef2eci(times[i], ecef_positions[i], [0,0,0], 0)[0] 
            for i in range(len(ecef_positions))]
        return eci_positions

    def get_file_name(self, start_time):
        data_type = "MEME"
        sat_num = "12345" # TODO
        common_name = "PANF" # TODO are we uploading for Leader as well
        day_time_group = start_time # TODO of form DOYHHMM in utc
        oper_spec = "operational" 
        meta_data = "" 
        classification = "unclassified"
        file_ext = "txt"

        return f"{data_type}_{sat_num}_{common_name}_{day_time_group}_{oper_spec}_{meta_data}_{classification}.{file_ext}"

    
    def batch_convert_to_utc_time(self, times):
        # TODO CONVERT Shihao
        return times
        
    def batch_convert_to_formatted_times(self, times):
        '''takes in gps times and converts to formatted times'''
        utc_times = self.batch_convert_to_utc_time(times)
        
        ## FORMATTING
        #“yyyDOYhhmmss.sss”)
        # TODO!! Shihao
        formatted = [str(x) for x in utc_times]
        return formatted

    def write_file(self, file_name, times, positions, covariances):
        '''assumes times in utc, positions in eci, covariances
        
        positions in km, covariances km^2
        '''
        eph_file = open(file_name,"a") # TODO correct naming

        formatted_times = self.batch_convert_to_formatted_times(times)

        # write header
        eph_file.write('header1\n')
        eph_file.write('header2\n')
        eph_file.write('header3\n')
        eph_file.write('MEME2000\n')

        num_entries = len(times)
        for i in range(num_entries):
            time = formatted_times[i]
            pos = positions[i]
            pos_string = " ".join(str(x) for x in pos)
            
            cov_matrix = covariances[i]
            refs = [(0,0), (1,0), (1,1), (2,0), (2,1), (2,2)]
            cov_array = [cov_matrix[i][j] for i,j in refs]
            cov_string = " ".join(str(x) for x in cov_array)
            
            s = f"{time} {pos_string}\n{cov_string}\n"
            eph_file.write(s)

        eph_file.close()
        print(f"Wrote {num_entries} to {file_name}.")

    def run(self):
        print("Running Monte Carlo")

        # Setup
        self.leader = self.radio_leader
        self.follower = self.radio_follower

        # Read the orbit data from each satellite from database
        downlinked_data_vals_leader = self.readDownlinkData(self.leader)
        downlinked_data_vals_follower = self.readDownlinkData(self.follower)
        pos_sigma, vel_sigma = self.get_sigmas()

        monte_carlo_position_runs = self.generate_monte_carlo_data(downlinked_data_vals_leader,
                                                                   downlinked_data_vals_follower,
                                                                   pos_sigma,
                                                                   vel_sigma)


        # Post-process lists, calculate covariances to output ephemeris

        np_monte_carlo_position_runs = np.array(monte_carlo_position_runs).transpose(1, 2, 0) / 1000 # convert to km
        
        covariance_per_step = [np.cov(step_positions) for step_positions in np_monte_carlo_position_runs]

        orbits_no_noise = self.run_psim(downlinked_data_vals_leader,
                                        downlinked_data_vals_follower,
                                        follower_r_noise=lin.Vector3(),
                                        follower_v_noise=lin.Vector3())

        times = [x.time for x in orbits_no_noise]
        ecef_positions = [x.pos for x in orbits_no_noise]
        eci_positions = self.batch_convert_to_eci(ecef_positions, times)
        
        km_positions = [[meter / 1000 for meter in position] for position in eci_positions] # convert to km

        self.write_file('Ephemeris.txt', times, km_positions, covariance_per_step) # TODO call get_file_name

        self.logger.put("EXITING MONTECARLO SIMS")
        self.finish()
