#ifndef GPSTIME_HPP_
#define GPSTIME_HPP_

#include "libsbp/navigation.h"

static constexpr unsigned int MILLISECONDS_IN_WEEK = 7*24*60*60*1000;

struct gps_time_t {
    msg_gps_time_t gpstime;

    /** Default constructor **/
    gps_time_t() {}

    /** Copy constructor **/
    gps_time_t(const msg_gps_time_t& t) {
        gpstime.wn = t.wn;
        gpstime.tow = t.tow;
    }
    /** Copy constructor **/
    gps_time_t(const gps_time_t& t) {
        gpstime.wn = t.gpstime.wn;
        gpstime.tow = t.gpstime.tow;
    }

    /** Equality operator **/
    bool operator==(const gps_time_t& t1) {
        return t1.gpstime.wn == gpstime.wn && t1.gpstime.tow == gpstime.tow;
    }

    /** Comparison operator **/
    bool operator<(const gps_time_t& t1) {
        return gpstime.wn < t1.gpstime.wn || (gpstime.wn < t1.gpstime.wn && gpstime.tow < t1.gpstime.tow);
    }

    /** Comparison operator **/
    bool operator>(const gps_time_t& t1) {
        return !(this->operator<(t1)) && !(this->operator==(t1));
    }

    /** Addition operator **/
    gps_time_t operator+(const gps_time_t& t1) const {
        gps_time_t t(*this);

        t.gpstime.wn += t1.gpstime.wn;
        if (t.gpstime.tow + t1.gpstime.tow > MILLISECONDS_IN_WEEK) t.gpstime.wn += 1;
        t.gpstime.tow = (t.gpstime.tow + t1.gpstime.tow) % MILLISECONDS_IN_WEEK;

        return t;
    }

    /** Subtraction operator **/
    gps_time_t operator-(const gps_time_t& t1) const {
        gps_time_t t(*this);

        t.gpstime.wn -= t1.gpstime.wn;
        if (t.gpstime.tow - t1.gpstime.tow < 0) {
            t.gpstime.wn -= 1;
            t.gpstime.tow = MILLISECONDS_IN_WEEK - t.gpstime.tow + t1.gpstime.tow;
        }
        else {
            t.gpstime.tow = t.gpstime.tow - t1.gpstime.tow;
        }
        return t;
    }

    /** Cast to integer operator **/
    explicit operator unsigned int() {
        return gpstime.wn * MILLISECONDS_IN_WEEK + gpstime.tow;
    }
};

#endif