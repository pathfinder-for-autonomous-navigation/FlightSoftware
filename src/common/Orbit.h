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
#include <lin/core.hpp>



namespace gnc {


/**
 * Class to handle most of the logic related to storing, serializing, propagating, and checking validity of an orbit.
 */
struct Orbit {
    /**
     * Construct invalid Orbit.
     * grav calls: 0
     */
     Orbit(){}
     /**
      * Construct Orbit from time, position, and velocity.
      * grav calls: 0
      * @param[in]
     */
     Orbit(uint64_t ns_gps_time,lin::Vector3d& r_ecef,lin::Vector3d& v_ecef){}
     /**
      * Construct invalid Orbit
      */
     Orbit(){}



};


}


#endif //PSIM_ORBIT_H
