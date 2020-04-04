/*
MIT License

Copyright (c) 2020 Pathfinder for Autonomous Navigation (PAN)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**
 * \file geograv.hpp
 * \author Nathan Zimmerberg
 * \date 31 Mar 2020
 * \brief a class to handle most of the logic related to storing, serializing, propagating, and checking validity of an orbit.
 */

#pragma once

#include <cstdint>
#include <limits>
#include <lin/core.hpp>
#include "constant_tracker.hpp"

#include "geograv.hpp"
#include "GGM05S.hpp"
#include "GPSTime.hpp"

#ifndef PANGRAVORDER //compiler flag to set the Gravity model order
#define PANGRAVORDER 40
#endif
TRACKED_CONSTANT(constexpr geograv::Coeff<PANGRAVORDER>, PANGRAVITYMODEL, static_cast<geograv::Coeff<PANGRAVORDER>>(GGM05S));

//std::numeric_limits<double>::quiet_NaN()
TRACKED_CONSTANT(const lin::Vector3d, LINNAN3d, {NAN,NAN,NAN});

TRACKED_CONSTANT(const double, MAXORBITRADIUS, 6378.0E3L+1000.0E3);

TRACKED_CONSTANT(const double, MINORBITRADIUS, 6378.0E3L);


/**
 * Class to handle most of the logic related to storing, serializing, propagating, and checking validity of an orbit.
 */
class Orbit {
  public:
    /** Position of the sat (m).*/
    lin::Vector3d _recef{LINNAN3d};

    /** Velocity of the sat (m/s).*/
    lin::Vector3d _vecef{LINNAN3d};

    /** Time since gps epoch (ns).*/
    uint64_t _ns_gps_time{0};

    /** Validity of the orbit.
     * A valid orbit has finite and real position and velocity, is in low
     * earth orbit, and has a reasonable time stamp (within 20 years of pan epoch).
     * The validity check should not reject 
     * gps readings due to reasonable noise of:
     * TODO add max expected gps error
     *
     * Low earth orbit is a Orbit that stays between MINORBITRADIUS and MAXORBITRADIUS.
     */
    bool _valid{false};

    /** Returns time since gps epoch (ns).
     * grav calls: 0 */
    uint64_t nsgpstime() const{
        return _ns_gps_time;
    }

    /** Returns position of the sat (m).
     * grav calls: 0 */
    lin::Vector3d recef() const{
        return _recef;
    }

    /** Returns velocity of the sat (m/s).
     * grav calls: 0 */
    lin::Vector3d vecef() const{
        return _vecef;
    }

    /** Returns true if the Orbit is valid.
     * A valid orbit has finite and real position and velocity, is in low
     * earth orbit, and has a reasonable time stamp (within 20 years of pan epoch).
     * The validity check should not reject 
     * gps readings due to reasonable noise of:
     * TODO add max expected gps error
     *
     * Low earth orbit is a Orbit that stays between MINORBITRADIUS and MAXORBITRADIUS.
     * grav calls: 0 */
    bool valid() const{
        return _valid;
    }

    /** 
     * Helper function calculates if the Orbit is valid, see valid().
     * If the orbit is invalid the orbit gets set to the default values
     * grav calls: 0 */
    void _check_validity(){
        double r2= lin::fro(_recef);
        //TODO add checks for time and velocity
        //note if position is NAN, these checks will be false.
        if (r2<MAXORBITRADIUS*MAXORBITRADIUS && r2>MINORBITRADIUS*MINORBITRADIUS){
            _valid= true;
        } else {
            _valid= false;
            _recef= LINNAN3d;
            _vecef= LINNAN3d;
            _ns_gps_time= 0;
        }
    }


    /** Helper method to call gravity function mks units in ECEF.*/
    static void calc_geograv(const lin::Vector3d& r_ecef, lin::Vector3d& g_ecef, double& potential) {
        geograv::Vector in;
        geograv::Vector g;
        in.x= r_ecef(0);
        in.y= r_ecef(1);
        in.z= r_ecef(2);
        potential=geograv::GeoGrav(in, g,PANGRAVITYMODEL,true);
        g_ecef(0)= g.x;
        g_ecef(1)= g.y;
        g_ecef(2)= g.z;
    }
    
    /**
     * Construct invalid Orbit.
     * grav calls: 0
     */
    Orbit(){}

    /**
     * Construct Orbit from time, position, and velocity.
     * Orbit self may be invalid if the inputs are bad, see valid().
     * grav calls: 0
     * @param[in] ns_gps_time(): time since gps epoch (ns).
     * @param[in] r_ecef(): position of the center of mass of the sat (m).
     * @param[in] v_ecef(): velocity of the sat (m/s).
     */
    Orbit(const uint64_t& ns_gps_time,const lin::Vector3d& r_ecef,const lin::Vector3d& v_ecef): 
        _recef(r_ecef),
        _vecef(v_ecef),
        _ns_gps_time(ns_gps_time) {
        _check_validity();
    }

    /**
     * Apply a deltav to the Orbit.
     * The Orbit should be not propagating.
     * grav calls: 0
     * @param[in] deltav_ecef(): The change in velocity in ecef frame (m/s). 
     */
    void applydeltav(const lin::Vector3d& deltav_ecef){
        _vecef= _vecef+deltav_ecef;
        _check_validity();
    }

    /**
     * Returns the specific energy of the Orbit (J/kg).
     * The Orbit should be not propagating.
     * If the Orbit is invalid this returns NAN
     * grav calls: 1 if Orbit valid, 0 if Orbit invalid
     */
    double specificenergy(const lin::Vector3d& earth_rate_ecef){
        if (valid()){
            double potential;
            lin::Vector3d junk;
            calc_geograv(recef(),junk,potential);
            lin::Vector3d v_ecef0= lin::cross(earth_rate_ecef,recef())+vecef();
            return 0.5L*lin::fro(v_ecef0) - potential;
        }
        else{
            return NAN;
        }
    }
    







};