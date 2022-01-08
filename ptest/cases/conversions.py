#eci_ecef_conversions.py
#Nathan Zimmerberg (nhz2)
#November 24, 2019
# using examples from https://astropy.readthedocs.io/en/latest/coordinates/velocities.html
"""Module for using astropy to convert between ECI and ECEF coordinates and velocities"""
import astropy.units as u
from astropy.coordinates import (ITRS,GCRS)
from astropy.coordinates import (CartesianRepresentation,CartesianDifferential)
from astropy.time import TimeDelta
from astropy.time import Time

def time2astropyTime(time,init_gps_weeknum):
    """
    args:
        time(double): time since init_GPS_week_number in seconds
        init_gps_weeknum(int): initial GPS week number."""
    return Time(init_gps_weeknum*7*24*60*60, time, scale='tai', format='gps')

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