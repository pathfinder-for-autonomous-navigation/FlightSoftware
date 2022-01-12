#eci_ecef_conversions.py
#Nathan Zimmerberg (nhz2)
#November 24, 2019
# using examples from https://astropy.readthedocs.io/en/latest/coordinates/velocities.html
"""Module for using astropy to convert between ECI and ECEF coordinates and velocities"""
import astropy.units as u
from astropy.coordinates import (ITRS,GCRS)
from astropy.coordinates import (CartesianRepresentation,CartesianDifferential)
from astropy.time import Time
import numpy as np

def time2astropyTime(time,init_gps_weeknum):
    """
    args:
        time(double): time since init_GPS_week_number in seconds
        init_gps_weeknum(int): initial GPS week number."""
    return Time(init_gps_weeknum*7*24*60*60, time, scale='tai', format='gps')

def ecef2eci_mc_runs(times, mc_runs, init_gps_weeknum):
    '''
    Times and mc_runs have the same length.
    args: 
        mc_runs, a list of all mc_runs, each of which is a list of positions across the duration of the run
        times, a list of times, measured in seconds since the pan_epoch
        init_gps_weeknum, the GPS week number of the pan epoch
    returns:
        a list of all the mc_runs, each of which is a list of positions in ECI
    '''
    num_times = len(times)
    list_of_mc_snapshots = np.array(mc_runs).transpose((1,0,2))
    list_of_mc_snapshots_eci = np.array([ecef2eci_same_time_batch(times[idx], list_of_mc_snapshots[idx], init_gps_weeknum) for idx in range(num_times)])
    return list_of_mc_snapshots_eci

def get_covariances(mc_snapshots_eci):
    '''Return a list of the covariances of the position vectors in ECI

    args:
        a list over mc runs, then over time of the position vectors
    returns:
        a list of 3 x 3 matricies over time'''
    return np.array([np.cov(x.T) for x in mc_snapshots_eci])

def ecef2eci_same_time_batch(time, vectors, init_gps_weeknum):
    '''Converts a batch of position vectors at the same time into the ECI coordinate frame
    
    args:
        vectors: a list of 3-element numpy arrays, each representing a position vector in ECEF
        time: a single time in seconds since the pan epoch
        init_gps_weeknum: the pan epoch
    returns:
        a list of 3-element numpy arrays representing the position in ECI coordinates, as a 2d numpy matrix
    '''
    vectors_transposed = np.array(vectors).T
    return ecef2eci(time, vectors_transposed, vectors_transposed, init_gps_weeknum)[0].T
    
def ecef2eci(time,r_ecef,v_ecef,init_gps_weeknum):
    """Returns a tuple of position and velocity in ECI"""
    coord_ecef=ITRS(x=r_ecef[0]*u.m, y=r_ecef[1]*u.m, z=r_ecef[2]*u.m,
        v_x=v_ecef[0]*u.m/u.s, v_y=v_ecef[1]*u.m/u.s, v_z=v_ecef[2]*u.m/u.s,
        representation_type=CartesianRepresentation,
        differential_type=CartesianDifferential,
        obstime=time2astropyTime(time,init_gps_weeknum))
    coord_eci= coord_ecef.transform_to(GCRS(obstime=time2astropyTime(time,init_gps_weeknum)))
    return (coord_eci.cartesian.xyz.to_value(u.m),coord_eci.velocity.d_xyz.to_value(u.m/u.s))

def eci2ecef(time,r_eci,v_eci,init_gps_weeknum):
    """Returns a tuple of position and velocity in ECEF"""
    coord_eci=GCRS(x=r_eci[0]*u.m, y=r_eci[1]*u.m, z=r_eci[2]*u.m,
        v_x=v_eci[0]*u.m/u.s, v_y=v_eci[1]*u.m/u.s, v_z=v_eci[2]*u.m/u.s,
        representation_type=CartesianRepresentation,
        differential_type=CartesianDifferential,
        obstime=time2astropyTime(time,init_gps_weeknum))
    coord_ecef= coord_eci.transform_to(ITRS(obstime=time2astropyTime(time,init_gps_weeknum)))
    return (coord_ecef.cartesian.xyz.to_value(u.m),coord_ecef.velocity.d_xyz.to_value(u.m/u.s))

if __name__ == '__main__':
    xs = [[0,0,3],[1,0,0],[0,1,0],[0,0,1],[1,1,1]]
    xss = [ [[1,2,3],[4,5,6], [4,5,6]], [[4,2,3],[4,5,6],[5,15,16]], [[1,2,3],[4,5,6], [40,15,60]]  ]

    def copy_xs():
        return [[y for y in x] for x in xs]

    mc_runs = [xss[0], xss[1], xss[2]]

    times = [1,2,3]
    
    converted_mc_runs = ecef2eci_mc_runs(times, mc_runs, 2045)
    covariances = get_covariances(converted_mc_runs)
    # print(converted_mc_runs.shape)
    # print(converted_mc_runs)
    print(covariances.shape)
    print(covariances)
    # print(covariances)