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
     * The Orbit must be not propagating.
     * grav calls: 0 */
    uint64_t nsgpstime() const{
        return _ns_gps_time;
    }

    /** Return position of the sat (m).
     * The Orbit must be not propagating.
     * grav calls: 0 */
    lin::Vector3d recef() const{
        return _recef;
    }

    /** Return velocity of the sat (m/s).
     * The Orbit must be not propagating.
     * grav calls: 0 */
    lin::Vector3d vecef() const{
        return _vecef;
    }

    /** Return true if the Orbit is valid.
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
     * Helper function calculate if the Orbit is valid, see valid().
     * If the orbit is invalid set the orbit to the default values
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


    /** Gravity function in International Terrestrial Reference System coordinates.
     * grav calls: 1
     * @param[in] r_ecef(Above the surface of earth): The location where the gravity is calculated, units m.
     * @param[out] g_ecef: Acceleration due to gravity, units m/s^2. 
     * @param[out] potential: Gravity potential, the acceleration is the gradient of this (J/kg).
     */
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
     * The Orbit must be not propagating.
     * grav calls: 0
     * @param[in] deltav_ecef(): The change in velocity in ecef frame (m/s). 
     */
    void applydeltav(const lin::Vector3d& deltav_ecef){
        _vecef= _vecef+deltav_ecef;
        _check_validity();
    }

    /**
     * Return the specific energy of the Orbit (J/kg).
     * The Orbit must be not propagating.
     * If the Orbit is invalid this returns NAN
     * grav calls: 1 if Orbit valid, 0 if Orbit invalid
     * @param[in] earth_rate_ecef(): The earth's angular rate in ecef frame (rad/s). 
     */
    double specificenergy(const lin::Vector3d& earth_rate_ecef) const{
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

    /************* PROPAGATION ******************/

    /** Return the dcm to rotate from initial ecef to ecef dt seconds latter.
     * Use equation 2.110 from Markley, Crassidis, Fundamentals of Spacecraft Attitude Determination and Control.
     * grav calls: 0
     */ 
    static void relative_earth_dcm_helper(const lin::Vector3d& earth_rate_ecef, const double& dt, lin::Matrix<double, 3, 3>&  A_EI){
        double earth_omega= lin::norm(earth_rate_ecef);
        lin::Vector3d e= earth_rate_ecef/earth_omega;
        double theta= earth_omega*dt;
        double c= std::cos(theta);
        double s= std::sin(theta);
        double e1= e(0);
        double e2= e(1);
        double e3= e(2);
        //equation 2.110 from the Markley Crassidis ADCS book.
        A_EI(0,0)= c+(1-c)*e1*e1;
        A_EI(0,1)= (1-c)*e1*e2+s*e3;
        A_EI(0,2)= (1-c)*e1*e3-s*e2;
        A_EI(1,0)= (1-c)*e2*e1-s*e3;
        A_EI(1,1)= c+(1-c)*e2*e2;
        A_EI(1,2)= (1-c)*e2*e3+s*e1;
        A_EI(2,0)= (1-c)*e3*e1+s*e2;
        A_EI(2,1)= (1-c)*e3*e2-s*e1;
        A_EI(2,2)= c+(1-c)*e3*e3;
    }

    /** Get the jacobian of a shortupdate.
     * This is partially auto code from sympy
     * 
     * The jacobian is calculated assuming point mass earth and constant earth rate of:
     *  w= 1.0e-04L * 0.729211585530000L rad/s in ecef z direction.
     * grav calls: 0
     * @param[in] r_half: The sats position in ecef0 at the half step (m).
     * @param[in] dt: Time step used (s). 
     * @param[out] jac: 
     * The jacobian of y=f(x) where x and y are vectors
           [r_ecef;
            v_ecef;] in m and m/s.
       and f is the shortupdate function.
     */
    static void _jacobian_helper(const lin::Vector3d& r_half ,const  double& dt, lin::Matrix<double, 6, 6>& jac){
        double mu= PANGRAVITYMODEL.earth_gravity_constant;
        double x_half= r_half(1);
        double y_half= r_half(1);
        double z_half= r_half(2);
        double w= 1.0e-04L * 0.729211585530000L; //earths angular rate in z
        double x0 = dt*w; //
        double x1 = std::cos(x0); //
        double x2 = (1.0/2.0)*dt; //
        double x3 = z_half; 
        double x4 = x3*x3;
        //double x5 = v_y + w*x;
        double x6 = y_half;
        double x7 = x6*x6;
        //double x8 = v_x - w*y;
        double x9 = x_half;
        double x10 = x4 + x7 + x9*x9;
        double sqrtx10= std::sqrt(x10);
        double sqrtx103= x10*sqrtx10;
        double x11 = mu/(sqrtx103);
        double x12 = -dt*x11;
        //double x13 = (3.0/2.0)*dt;
        double x14 = (3.0/2.0)*x0;
        double x15 = -3*x_half - x14*x6;
        double x16 = mu/(sqrtx103*x10);
        double x17 = dt*x16;
        double x18 = x15*x17;
        double x19 = x12 - x18*x9;
        double x20 = x19*x2 + 1;
        double x21 = std::sin(x0); //
        double x22 = (1.0/2.0)*x0; //
        double x23 = dt*dt; //
        double x24 = (1.0/2.0)*x23; //
        double x25 = x11*x24; 
        double x26 = w*x25;
        double x27 = w - x18*x6 - x26;
        double x28 = x2*x27;
        double x29 = x22 + x28;
        double x30 = -3*y_half + x14*x9;
        double x31 = x17*x30;
        double x32 = x12 - x31*x6;
        double x33 = x2*x32;
        double x34 = x33 + 1;
        double x35 = -x22;
        double x36 = -w + x26 - x31*x9;
        double x37 = x2*x36 + x35;
        double x38 = -3*z_half;
        double x39 = x16*x24*x38;
        double x40 = x21*x39;
        double x41 = x1*x9;
        double x42 = dt*dt*dt;
        double x43 = x16*x3;
        double x44 = (3.0/4.0)*x42*x43;
        double x45 = x44*x6;
        double x46 = x21*x45 + x41*x44;
        double x47 = x6*x9;
        double x48 = (3.0/4.0)*x16*x42*x47;
        double x49 = (3.0/2.0)*x16*x23;
        double x50 = -x25 + 1;
        double x51 = x49*x7 + x50;
        double x52 = x2*x51;
        double x53 = x2 + x52;
        double x54 = x39*x6;
        double x55 = x44*x9;
        double x56 = x1*x45 - x21*x55;
        double x57 = x24*x43;
        double x58 = x17*x38;
        double x59 = x12 - x3*x58;
        double x60 = x4*x49 + x50;
        double x61 = x2*x60 + x2;
        double x62 = -w*x20 + x27;
        double x63 = -w*(-x28 + x35) + x19;
        double x64 = -w*(-x33 - 1) + x36;
        double x65 = -w*x37 + x32;
        double x66 = w*x39*x9 - x58*x6;
        double x67 = -w*x54 - x58*x9;
        double x68 = x3*x49;
        double x69 = x6*x68;
        double x70 = -w*x55 + x69;
        double x71 = w*x45 + x68*x9;
        double x72 = x1*x71 + x21*x70;
        double x73 = -w*x48 + x51;
        double x74 = -w*(-x2 - x52) + x47*x49;
        double x75 = x1*x70 - x21*x71;
        jac(0,0)= x1*x20 + x21*x29;
        jac(0,1)= x1*x37 + x21*x34;
        jac(0,2)= -x39*x41 - x40*x6;
        jac(0,3)= x46;
        jac(0,4)= x1*x48 + x21*x53;
        jac(0,5)= x46;
        jac(1,0)= x1*x29 - x20*x21;
        jac(1,1)= x1*x34 - x21*x37;
        jac(1,2)= -x1*x54 + x40*x9;
        jac(1,3)= x56;
        jac(1,4)= x1*x53 - x21*x48;
        jac(1,5)= x56;
        jac(2,0)= -x15*x57;
        jac(2,1)= -x30*x57;
        jac(2,2)= x2*x59 + 1;
        jac(2,3)= x61;
        jac(2,4)= x45;
        jac(2,5)= x61;
        jac(3,0)= x1*x63 + x21*x62;
        jac(3,1)= x1*x64 + x21*x65;
        jac(3,2)= x1*x67 + x21*x66;
        jac(3,3)= x72;
        jac(3,4)= x1*x74 + x21*x73;
        jac(3,5)= x72;
        jac(4,0)= x1*x62 - x21*x63;
        jac(4,1)= x1*x65 - x21*x64;
        jac(4,2)= x1*x66 - x21*x67;
        jac(4,3)= x75;
        jac(4,4)= x1*x73 - x21*x74;
        jac(4,5)= x75;
        jac(5,0)= -x18*x3;
        jac(5,1)= -x3*x31;
        jac(5,2)= x59;
        jac(5,3)= x60;
        jac(5,4)= x69;
        jac(5,5)= x60;
    }

    /**
     * Helper to do a short update of the orbit.
     * The orbit propigator is designed for nearly circular low earth orbit, and
     * ignores all forces except constant gravity from earth.
     * The Orbit must be not propagating and valid.
     * grav calls: 1
     * @param[in] dt (in the range [-0.2,0.2]): Time step (s).
     * @param[in] earth_rate_ecef: The earth's angular rate in ecef frame (rad/s). 
     * @param[out] r_half_ecef0: The sats position in ecef0 at the half step (m).
     * @param[out] specificenergy: Specific energy of the Orbit at the half step (J/kg).
     */
    void _shortupdate_helper(const double& dt,const lin::Vector3d& earth_rate_ecef, lin::Vector3d& r_half_ecef0, double& specificenergy){
        double mu= PANGRAVITYMODEL.earth_gravity_constant;
        // step 1a ecef->ecef0
        lin::Vector3d r_ecef0= _recef;
        lin::Vector3d v_ecef0= lin::cross(earth_rate_ecef,_recef)+_vecef;
        // step 1b get circular orbit reference
        double energy= 0.5*lin::dot(v_ecef0,v_ecef0)-mu/lin::norm(r_ecef0);
        double a= -mu/2/energy;
        lin::Vector3d h_ecef0= lin::cross(r_ecef0,v_ecef0);
        lin::Vector3d x= r_ecef0;
        x= x/lin::norm(x)*a;
        lin::Vector3d y= lin::cross(h_ecef0,r_ecef0);
        y= y/lin::norm(y)*a;
        lin::Vector3d omega= h_ecef0/lin::norm(h_ecef0)*std::sqrt(mu/(a*a*a));
        lin::Vector3d rel_r= r_ecef0-x;
        lin::Vector3d rel_v= v_ecef0-lin::cross(omega,x);
        // step 2 drift
        rel_r= rel_r+rel_v*dt*0.5;
        // step 3a calc acceleration at the half step
        double t= 0.5*dt;
        double theta= t*lin::norm(omega);
        double costheta= std::cos(theta);
        double sintheta= std::sin(theta);
        lin::Vector3d orb_r= x*costheta+y*sintheta;
        r_ecef0= rel_r+orb_r;
        r_half_ecef0= r_ecef0;
        lin::Matrix<double, 3, 3> dcm_ecefhalf_ecef0;
        relative_earth_dcm_helper(earth_rate_ecef, t, dcm_ecefhalf_ecef0);
        lin::Vector3d pos_ecef= dcm_ecefhalf_ecef0*r_ecef0;
        lin::Vector3d g_ecef;
        double potential;
        calc_geograv(pos_ecef, g_ecef, potential);
        //convert to ECEF0
        lin::Vector3d g_ecef0= lin::transpose(dcm_ecefhalf_ecef0)*g_ecef + mu*orb_r/(a*a*a);
        // step 3b kick velocity
        lin::Vector3d relhalf_v= rel_v + g_ecef0*dt*0.5;
        double halfstepke= lin::fro(relhalf_v+lin::cross(omega,orb_r))*0.5;
        rel_v= rel_v + g_ecef0*dt;
        // step 4 drift
        rel_r= rel_r+rel_v*dt*0.5;
        // step 5a get back absolute orbit
        double cos2theta= costheta*costheta-sintheta*sintheta;
        double sin2theta= 2*sintheta*costheta;
        orb_r= x*cos2theta+y*sin2theta;
        r_ecef0= rel_r+ orb_r;
        v_ecef0= rel_v+ lin::cross(omega,orb_r);
        // step 5b rotate back to ecef
        _recef= dcm_ecefhalf_ecef0*dcm_ecefhalf_ecef0*r_ecef0;
        _vecef= dcm_ecefhalf_ecef0*dcm_ecefhalf_ecef0*v_ecef0;
        // step 6 remove cross r term from velocity
        _vecef= _vecef-lin::cross(earth_rate_ecef,_recef);
        specificenergy= halfstepke - potential;
    }

    /**
     * Do a short update of the orbit.
     * The orbit propigator is designed for nearly circular low earth orbit, and
     * ignores all forces except constant gravity from earth.
     * The jacobians are calculated assuming point mass earth and constant earth rate of:
     *  w= 1.0e-04L * 0.729211585530000L rad/s in ecef z direction.
     * The Orbit must be not propagating.
     * grav calls: 1 if Orbit valid, 0 if Orbit invalid
     * @param[in] dt_ns (in the range [-2E8,2E8]): Time step (ns).
     * @param[in] earth_rate_ecef: The earth's angular rate in ecef frame (rad/s). 
     * @param[out] jac: 
     * The jacobian of y=f(x) where x and y are vectors
           [r_ecef;
            v_ecef;] in m and m/s.
       and f is the shortupdate function.
     * @param[out] specificenergy: Specific energy of the Orbit at the half step (J/kg).
     */
    void shortupdate(const uint64_t& dt_ns,const lin::Vector3d& earth_rate_ecef, lin::Matrix<double, 6, 6>& jac, double& specificenergy){
        if (!valid()){
            jac={NAN,NAN,NAN,NAN,NAN,NAN,
                 NAN,NAN,NAN,NAN,NAN,NAN,
                 NAN,NAN,NAN,NAN,NAN,NAN,
                 NAN,NAN,NAN,NAN,NAN,NAN,
                 NAN,NAN,NAN,NAN,NAN,NAN,
                 NAN,NAN,NAN,NAN,NAN,NAN};
            specificenergy= NAN;
            return;
        }
        lin::Vector3d r_half_ecef0;
        double dt= double(dt_ns)*1E-9L;
        _shortupdate_helper(dt,earth_rate_ecef, r_half_ecef0, specificenergy);
        _jacobian_helper(r_half_ecef0 ,dt, jac); 
    }

    /**
     * Do a short update of the orbit.
     * The orbit propigator is designed for nearly circular low earth orbit, and
     * ignores all forces except constant gravity from earth.
     * The Orbit must be not propagating.
     * grav calls: 1 if Orbit valid, 0 if Orbit invalid
     * @param[in] dt_ns (in the range [-2E8,2E8]): Time step (ns).
     * @param[in] earth_rate_ecef: The earth's angular rate in ecef frame (rad/s). 
     * @param[out] specificenergy: Specific energy of the Orbit (J/kg).
     */
    void shortupdate(const uint64_t& dt_ns,const lin::Vector3d& earth_rate_ecef, double& specificenergy){
        if (!valid()){
            specificenergy= NAN;
            return;
        }
        lin::Vector3d r_half_ecef0;
        double dt= double(dt_ns)*1E-9L;
        _shortupdate_helper(dt,earth_rate_ecef, r_half_ecef0, specificenergy);
    }
    







};