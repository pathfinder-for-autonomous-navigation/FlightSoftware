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
 * \file jacobian_autocoded.hpp
 * \author Nathan Zimmerberg
 * \date 10 APR 2020
 * \brief Helper function to get the jacobian of in Orbit::shortupdate().
 * \details  Partially auto code from sympy in jacobian_autocoder.py
 */

#pragma once

#include <lin/core.hpp>
#include <cmath>

namespace orb
{

/** Helper function to get the jacobian of in Orbit::shortupdate().
 * This is partially auto code from sympy in JacobianHelpers/jacobian_autocoder.py
 * 
 * The jacobian is calculated assuming point mass earth and constant earth rate of:
 *  w= 1.0e-04L * 0.729211585530000L rad/s in ecef z direction.
 * 
 * grav calls: 0
 * @param[in] x_h: The sats x position in ecef0 at the half step (m).
 * @param[in] y_h: The sats y position in ecef0 at the half step (m).
 * @param[in] z_h: The sats z position in ecef0 at the half step (m).
 * @param[in] w: The z of earths rate in ecef (rad/s). 
 * @param[in] mu: Earths standard gravitational parameter (m^3/s^2).
 * @param[in] dt: Time step used (s). 
 * @param[out] jac: 
 * The jacobian of y=f(x) where x and y are vectors
         [r_ecef;
        v_ecef;] in m and m/s.
and f is the Orbit::shortupdate() function.
*/
inline void jacobian_autocoded(const double& x_h, const double& y_h, const double& z_h, const double& w, const double& mu, const double& dt, lin::Matrix<double, 6, 6>& jac){       
        double x0 = dt*w;
        double x1 = std::cos(x0);
        double x2 = x_h*x_h;
        double x3 = y_h*y_h;
        double x4 = z_h*z_h;
        double x5 = x2 + x3 + x4;
        double x5sqrt= std::sqrt(x5);
        double x5sqrt3= x5*x5sqrt;
        double x6 = mu/x5sqrt3;
        double x7 = -dt*x6;
        double x8 = 3*x_h;
        double x9 = (3.0/2.0)*x0;
        double x10 = -x8 - x9*y_h;
        double x11 = mu/(x5*x5sqrt3);
        double x12 = dt*x11;
        double x13 = x10*x12;
        double x14 = -x13*x_h + x7;
        double x15 = (1.0/2.0)*dt;
        double x16 = x14*x15 + 1;
        double x17 = std::sin(x0);
        double x18 = (1.0/2.0)*x0;
        double x19 = dt*dt;
        double x20 = (1.0/2.0)*x19;
        double x21 = x20*x6;
        double x22 = w*x21;
        double x23 = w - x13*y_h - x22;
        double x24 = x15*x23;
        double x25 = x18 + x24;
        double x26 = 3*y_h;
        double x27 = -x26 + x9*x_h;
        double x28 = x12*x27;
        double x29 = -x28*y_h + x7;
        double x30 = x15*x29;
        double x31 = x30 + 1;
        double x32 = -x18;
        double x33 = -w + x22 - x28*x_h;
        double x34 = x15*x33 + x32;
        double x35 = (3.0/2.0)*x11*x19;
        double x36 = x35*x_h;
        double x37 = x36*z_h;
        double x38 = x35*y_h*z_h;
        double x39 = dt*dt*dt;
        double x40 = (3.0/4.0)*x11*x39*x_h*y_h;
        double x41 = x17*x40;
        double x42 = -x21 + 1;
        double x43 = x2*x35 + x42;
        double x44 = x15*x43 + x15;
        double x45 = x1*x40;
        double x46 = x3*x35 + x42;
        double x47 = x15*x46;
        double x48 = x15 + x47;
        double x49 = x11*z_h;
        double x50 = (3.0/4.0)*x39*x49;
        double x51 = x50*x_h;
        double x52 = x50*y_h;
        double x53 = x20*x49;
        double x54 = 3*x12*x4 + x7;
        double x55 = x35*x4 + x42;
        double x56 = -w*x16 + x23;
        double x57 = -w*(-x24 + x32) + x14;
        double x58 = -w*(-x30 - 1) + x33;
        double x59 = -w*x34 + x29;
        double x60 = x12*z_h;
        double x61 = -w*x37 + x26*x60;
        double x62 = w*x38 + x60*x8;
        double x63 = w*x40;
        double x64 = x43 + x63;
        double x65 = x36*y_h;
        double x66 = -w*x44 + x65;
        double x67 = x46 - x63;
        double x68 = -w*(-x15 - x47) + x65;
        double x69 = -w*x51 + x38;
        double x70 = w*x52 + x37;
        jac(0,0)= x1*x16 + x17*x25;
        jac(0,1)= x1*x34 + x17*x31;
        jac(0,2)= x1*x37 + x17*x38;
        jac(0,3)= x1*x44 + x41;
        jac(0,4)= x17*x48 + x45;
        jac(0,5)= x1*x51 + x17*x52;
        jac(1,0)= x1*x25 - x16*x17;
        jac(1,1)= x1*x31 - x17*x34;
        jac(1,2)= x1*x38 - x17*x37;
        jac(1,3)= -x17*x44 + x45;
        jac(1,4)= x1*x48 - x41;
        jac(1,5)= x1*x52 - x17*x51;
        jac(2,0)= -x10*x53;
        jac(2,1)= -x27*x53;
        jac(2,2)= x15*x54 + 1;
        jac(2,3)= x51;
        jac(2,4)= x52;
        jac(2,5)= x15*x55 + x15;
        jac(3,0)= x1*x57 + x17*x56;
        jac(3,1)= x1*x58 + x17*x59;
        jac(3,2)= x1*x62 + x17*x61;
        jac(3,3)= x1*x64 + x17*x66;
        jac(3,4)= x1*x68 + x17*x67;
        jac(3,5)= x1*x70 + x17*x69;
        jac(4,0)= x1*x56 - x17*x57;
        jac(4,1)= x1*x59 - x17*x58;
        jac(4,2)= x1*x61 - x17*x62;
        jac(4,3)= x1*x66 - x17*x64;
        jac(4,4)= x1*x67 - x17*x68;
        jac(4,5)= x1*x69 - x17*x70;
        jac(5,0)= -x13*z_h;
        jac(5,1)= -x28*z_h;
        jac(5,2)= x54;
        jac(5,3)= x37;
        jac(5,4)= x38;
        jac(5,5)= x55;
}
}