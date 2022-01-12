from .base import AMCCase
import time
from psim.sims import OrbitControllerTest
from psim import Configuration, Simulation
import lin
import numpy as np
from .utils import str_to_val, Enums
from typing import NamedTuple, List
from ..gpstime import GPSTime
from .conversions import ecef2eci_mc_runs, ecef2eci, get_covariances, time2astropyTime
from datetime import datetime


FLIGHT_MC_RUNS = 100
FLIGHT_RUN_TIME = 1000000000 * 60 * 60 * 24 * 7 # 7 day sim time measured in nanos
ONE_HOUR = 1000000000 * 60 * 60 * 1

RUNTIME = FLIGHT_RUN_TIME
NUM_MC_RUNS = FLIGHT_MC_RUNS

HALF = False
HARDCODED_SIGMAS = True # setting to true since by default sigmas are not downlinked.
HARDCODED = False

CC_NANOS = 170000000 # number of nanoseconds in a cycle
STEPS_PER_MIN = int(1000000000 * 60 / CC_NANOS) # Number of simulation steps in one minute 
STEPS_PER_LOG_ENTRY = STEPS_PER_MIN # How often to log positions and time
PAN_FOLLOWER_ID = '12345'
DONE_FILE_NAME = 'done_file.done'
LOG_PATH = 'pro/mc_runs/'


np.random.seed(123)


class OrbitData(NamedTuple):
    """A class for storing orbit data"""
    pos: list
    vel: list
    time: list

class MonteCarlo(AMCCase):
    '''A special PTest Case for Running MonteCarlos for NASA'''

    def get_sigmas_hardcoded(self):
        '''Hardcoded version of sigmas pos, vel, pulled from first few cycles of SingleSatStanby'''
        pos_sigma = [1.369557, 1.369557, 1.369559]
        vel_sigma = [0.363415, 0.363415, 0.363415]
        return pos_sigma, vel_sigma

    def get_sigmas_half(self):
        '''Hardcoded version of sigmas pos, vel, pulled from first few cycles of SingleSatStanby'''
        pos_sigma = [1.369557, 1.369557, 1.369559]
        vel_sigma = [0.183415, 0.183415, 0.183415]
        return pos_sigma, vel_sigma

    def get_zero_sigmas(self):
        return [0,0,0], [0,0,0]

    def get_sigmas(self):
        '''Return the sigmas for seeding the montecarlo cases'''
        if HALF:
            return self.get_sigmas_half()
        if HARDCODED_SIGMAS:
            return self.get_sigmas_hardcoded()
        pos_sigma = str_to_val(self.follower.read_state("orbit.pos_sigma"))
        vel_sigma = str_to_val(self.follower.read_state("orbit.vel_sigma"))
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
        """Read downlink data from the satellite
        
        args: 
            satellite: a FancyFlightController for the satellite
        returns:
            Orbit data from the satellite
        """
        if HARDCODED:
            if satellite == self.leader:
                return self.read_downlink_data_leader_hardcoded()
            else:
                return self.read_downlink_data_follower_hardcoded()

        pos = str_to_val(satellite.read_state("orbit.pos"))
        vel = str_to_val(satellite.read_state("orbit.vel"))
        time = GPSTime(*(str_to_val(satellite.read_state("time.gps")))).to_list() # this is a GPS Time since GPS epoch
        return OrbitData(pos, vel, time)

    def run_psim(self, 
                 leader_orbit, 
                 follower_orbit,
                 follower_r_noise = lin.Vector3([0,0,0]), 
                 follower_v_noise = lin.Vector3([0,0,0]),
                 runtime=RUNTIME
                 ) -> List[OrbitData]:
        """Run a simulation with the given parameters
        
        args:
            leader_orbit: OrbitData for the leader, at the start of the sim
            follower_orbit: Orbitdata for the follower, at the start of the sim
            follower_r_noise: noise to add to the follower position
            follower_v_noise: noise to add to the follower velocity
            runtime: the length of the simulation in nanoseconds
        returns:
            a list of OrbitData objects, describing the follower position over the simulation
                times are GPSTime objects
        """
        configs = ["sensors/base", "truth/base", "truth/vox_standby", "fc/base"]
        configs = ["lib/common/psim/config/parameters/" + f + ".txt" for f in configs]
        config = Configuration(configs)
        
        # set psim random seed to random number
        config["seed"] = np.random.randint(0, 2**32 - 1)

        # update values to current 
        config["truth.leader.orbit.r"] = lin.Vector3(leader_orbit.pos)
        config["truth.leader.orbit.v"] = lin.Vector3(leader_orbit.vel)
        config["truth.follower.orbit.r"] = lin.Vector3(follower_orbit.pos) + follower_r_noise
        config["truth.follower.orbit.v"] = lin.Vector3(follower_orbit.vel) + follower_v_noise
        config["truth.t.ns"] = GPSTime(*(follower_orbit.time)).to_pan_ns() # takes in GPS epoch time -> pan epoch time in nano seconds

        propagated_follower_orbits = []

        # step sim to desired time
        sim = Simulation(OrbitControllerTest, config)
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

        return propagated_follower_orbits

    def generate_monte_carlo_data(self, downlinked_leader, downlinked_follower, pos_sigma, vel_sigma):
        """Generate monte carlo data for the given downlink data
        
        args:
            downlinked_leader: OrbitData for the leader, at the start of the sim
            downlinked_follower: OrbitData for the follower, at the start of the sim
            pos_sigma: the sigma for the position noise
            vel_sigma: the sigma for the velocity noise
        returns:
            a list of simulation runs, each as a list of OrbitData objects, describing the follower over the simulation"""
        monte_carlo_position_runs = [] #list of positions for each trial
        for i in range(1, NUM_MC_RUNS + 1):
            list_of_orbit_data = self.run_psim(downlinked_leader, 
                                                downlinked_follower,
                                                follower_r_noise=lin.Vector3(np.random.normal(scale=pos_sigma)),
                                                follower_v_noise=lin.Vector3(np.random.normal(scale=vel_sigma)), 
                )
            list_of_orbit_positions = [x.pos for x in list_of_orbit_data]
            monte_carlo_position_runs.append(list_of_orbit_positions)
            
            print(f"Finished {i} simulations")
                
        return monte_carlo_position_runs

    def batch_convert_to_eci(self, ecef_positions, times):
        '''Convert ECEF positions to ECI positions
        
        args: 
            ecef_positions:
            times: Times are all GPSTime objects
            
        returns:
            eci_positions
        '''
        print('Converting to ECI coordinates')
        eci_start_time = time.time()
        seconds_since_pan_epoch_times = [MonteCarlo.time_since_pan_epoch(t) for t in times]
        eci_positions = [ecef2eci(seconds_since_pan_epoch_times[i], ecef_positions[i], [0,0,0], GPSTime.EPOCH_WN)[0] 
            for i in range(len(ecef_positions))]
        eci_finish_time = time.time()
        print(f'Spent {eci_finish_time - eci_start_time} seconds converting to ECI coordinates.')

        return eci_positions

    def get_file_name(self, start_time) -> str:
        """Get the name of the file, given the start time.

        args:
            start_time: the start time of the simulation, as a UTC datetime object
        returns:
            a string name for the file"""
        data_type = "MEME"
        sat_num = PAN_FOLLOWER_ID
        common_name = "PANF"
        
        doy = start_time.timetuple().tm_yday
        hour = start_time.hour
        minute = start_time.minute
        time_str = f"{doy:03d}{hour:02d}{minute:02d}"        
        
        oper_spec = "operational" 
        meta_data = "" 
        classification = "unclassified"
        file_ext = "txt"

        return f"{data_type}_{sat_num}_{common_name}_{time_str}_{oper_spec}_{meta_data}_{classification}.{file_ext}"
    
    @staticmethod
    def time_since_pan_epoch(time):
        '''time is a GPS time since the GPS epoch
        args:
            time: GPSTime object expressed as list
        returns:
            seconds since the PAN epoch'''
        pan_gps_time = GPSTime(*time)
        time_in_seconds_since_pan_epoch = pan_gps_time.to_pan_seconds()

        return time_in_seconds_since_pan_epoch
    
    @staticmethod
    def batch_convert_to_utc_time(times):
        """Convert GPSTime times to UTC datetimes
        
        args:
            times: as a list of GPSTime objects as lists
        returns:
            utc_times: as a list of UTC datetime objects
        """
        print('Converting times to UTC')
        times_in_seconds_since_pan_epoch = [MonteCarlo.time_since_pan_epoch(t) for t in times]
        astropy_times = [time2astropyTime(t, GPSTime.EPOCH_WN) for t in times_in_seconds_since_pan_epoch]
        utc_times = [t.utc.to_datetime() for t in astropy_times]
        
        return utc_times
    
    @staticmethod
    def format_time(time: datetime):
        '''Formats datetime to yyyDOYhhmmss.sss # i think this is an error, should be yyyyDOYhhmmss.sss
        
        args:
            time: a datetime object
        return:
            a string of the time
        '''

        year = time.year
        doy = time.timetuple().tm_yday
        hour = time.hour
        minute = time.minute
        second = time.second
        millis = time.microsecond / 1000
        final_str = f"{year:04d}{doy:03d}{hour:02d}{minute:02d}{second:02d}.{millis:03.0f}"
        return final_str
                
    def batch_convert_to_formatted_times(self, utc_times):
        '''takes in utc times and converts to formatted times
        
        args:
            utc_times: a list of UTC datetime objects
        returns:
            a list of formatted strings for the times
        '''
        print('Formatting times')
        formatted = [MonteCarlo.format_time(t) for t in utc_times]
        return formatted

    def write_done_file(self, filename):
        '''Write a donefile with the filname at the below path.'''
        done_file = open(LOG_PATH + filename, "w")
        done_file.write("done")
        done_file.close()
        print('done file written')

    def write_file(self, times, km_positions, covariances):
        '''Writes the ephemeris to a file, then writes a done file

        args:
            times: list of GPSTime objects as lists
            positions: km_positions in eci in km
            covariances: covariances in km^2
        returns:
            None
        '''
        utc_times = self.batch_convert_to_utc_time(times)
        first_utc_time = utc_times[0]
        print(first_utc_time)

        file_name = self.get_file_name(first_utc_time)
        eph_file = open(LOG_PATH + file_name, "w")

        formatted_times = self.batch_convert_to_formatted_times(utc_times)

        # write header
        eph_file.write('header1\n')
        eph_file.write('header2\n')
        eph_file.write('header3\n')
        eph_file.write('MEME2000\n')

        num_entries = len(times)
        print(f"Writing {num_entries} entries to {file_name}.")

        for i in range(num_entries):
            time = formatted_times[i]
            pos = km_positions[i]
            pos_string = " ".join(str(x) for x in pos)
            
            cov_matrix = covariances[i]
            refs = [(0,0), (1,0), (1,1), (2,0), (2,1), (2,2)]
            cov_array = [cov_matrix[i][j] for i,j in refs]
            cov_string = " ".join(str(x) for x in cov_array)
            
            s = f"{time} {pos_string}\n{cov_string}\n"
            eph_file.write(s)

        eph_file.close()
        print(f"Wrote {num_entries} entries to {file_name}.")

        self.write_done_file(DONE_FILE_NAME)

    def run(self):
        print("Running Monte Carlo")

        # Setup
        self.leader = self.radio_leader
        self.follower = self.radio_follower

        # Read the orbit data from each satellite from database
        downlinked_data_vals_leader = self.readDownlinkData(self.leader)
        downlinked_data_vals_follower = self.readDownlinkData(self.follower)
        pos_sigma, vel_sigma = self.get_sigmas()

        mc_start_time = time.time()
        monte_carlo_position_runs = self.generate_monte_carlo_data(downlinked_data_vals_leader,
                                                                   downlinked_data_vals_follower,
                                                                   pos_sigma,
                                                                   vel_sigma)

        mc_finish_time = time.time()
        
        orbits_no_noise = self.run_psim(downlinked_data_vals_leader,
                                        downlinked_data_vals_follower,
                                        follower_r_noise=lin.Vector3(),
                                        follower_v_noise=lin.Vector3())

        times = np.array([x.time for x in orbits_no_noise]) # list of GPSTime objects as lists
        ecef_positions = [x.pos for x in orbits_no_noise]
        eci_positions = self.batch_convert_to_eci(ecef_positions, times)

        # monte_carlo_position_runs_eci = [self.batch_convert_to_eci(mc_pos_run, times) for mc_pos_run in monte_carlo_position_runs]
        print(f'Spent {mc_finish_time - mc_start_time} seconds generating Monte Carlo data.')

        # Post-process lists, calculate covariances to output ephemeris

        start_covariance_time = time.time()
        pan_times = [MonteCarlo.time_since_pan_epoch(t) for t in times]
        monte_carlo_position_runs = np.array(monte_carlo_position_runs)
        print(monte_carlo_position_runs.shape)
        mc_runs_eci = ecef2eci_mc_runs(pan_times, monte_carlo_position_runs, GPSTime.EPOCH_WN) / 1000 # div by 1000 to convert to km
        covariance_per_step = get_covariances(mc_runs_eci)
        end_covariance_time = time.time()
        print(f'Spent {end_covariance_time - start_covariance_time} seconds calculating covariance, and converting to ECI.')
    
        km_positions = [[meter / 1000 for meter in position] for position in eci_positions] # convert to km

        self.write_file(times, km_positions, covariance_per_step)

        self.logger.put("EXITING MONTECARLO SIMS")
        self.finish()