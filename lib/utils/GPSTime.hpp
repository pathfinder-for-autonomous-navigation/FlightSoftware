#ifndef GPSTIME_HPP_
#define GPSTIME_HPP_

#include "../libsbp/navigation.h"

static constexpr unsigned long MILLISECONDS_IN_WEEK = 7*24*60*60*1000;
static constexpr unsigned long NANOSECONDS_IN_WEEK = MILLISECONDS_IN_WEEK*1000000;

struct gps_time_t {
    msg_gps_time_t gpstime;
    bool is_not_set;

    /** Default constructor **/
    gps_time_t() {
        gpstime.wn = 0;
        gpstime.tow = 0;
        gpstime.ns = 0;
        is_not_set = true;
    }

    /** Copy constructor **/
    gps_time_t(const msg_gps_time_t& t) {
        gpstime.wn = t.wn;
        gpstime.tow = t.tow;
        gpstime.ns = t.ns;
        is_not_set = false;
    }

    /** Copy constructor **/
    gps_time_t(const gps_time_t& t) {
        gpstime.wn = t.gpstime.wn;
        gpstime.tow = t.gpstime.tow;
        gpstime.ns = t.gpstime.ns;
        is_not_set = false;
    }

    /** Assignment operator **/
    gps_time_t& operator=(const msg_gps_time_t& t) {
        this->gpstime.wn = t.wn;
        this->gpstime.tow = t.tow;
        this->gpstime.ns = t.ns;
        is_not_set = false;
        return *(this);
    }
    
    /** Assignment operator  **/
    gps_time_t& operator=(const gps_time_t& t) {
        this->gpstime.wn = t.gpstime.wn;
        this->gpstime.tow = t.gpstime.tow;
        this->gpstime.ns = t.gpstime.ns;
        is_not_set = false;
        return *(this);
    }

    /** Inequality operator **/
    bool operator!=(const gps_time_t& t1) {
        return t1.gpstime.wn != gpstime.wn || t1.gpstime.tow != gpstime.tow || t1.gpstime.ns != gpstime.ns;
    }

    /** Inequality operator **/
    bool operator!=(const unsigned long& t1) {
        return (gpstime.wn * NANOSECONDS_IN_WEEK) + (gpstime.tow * 1000*1000) != t1;
    }

    /** Equality operator **/
    bool operator==(const gps_time_t& t1) {
        return !(this->operator!=(t1));
    }

    /** Equality operator **/
    bool operator==(const unsigned int& t1) {
        return !(this->operator!=(t1));
    }

    /** Comparison operator **/
    bool operator<(const gps_time_t& t1) {
        return gpstime.wn < t1.gpstime.wn || 
            (gpstime.wn == t1.gpstime.wn && gpstime.tow < t1.gpstime.tow) ||
            (gpstime.wn == t1.gpstime.wn && gpstime.tow == t1.gpstime.tow && gpstime.ns == t1.gpstime.ns );
    }

    /** Comparison operator **/
    bool operator<(const unsigned long& t1) {
        return gpstime.wn * NANOSECONDS_IN_WEEK + gpstime.tow*1000000 + gpstime.ns < t1;
    }

    /** Comparison operator **/
    bool operator>(const gps_time_t& t1) {
        return !(this->operator<(t1) || this->operator==(t1));
    }

    /** Comparison operator **/
    bool operator>(const unsigned int& t1) {
        return !(this->operator<(t1) || this->operator==(t1));
    }

    /** Comparison operator **/
    bool operator<=(const gps_time_t& t1) {
        return !(this->operator>(t1));
    }

    /** Comparison operator **/
    bool operator<=(const unsigned int& t1) {
        return !(this->operator>(t1));
    }

    /** Comparison operator **/
    bool operator>=(const gps_time_t& t1) {
        return !(this->operator<(t1));
    }

    /** Comparison operator **/
    bool operator>=(const unsigned int& t1) {
        return !(this->operator<(t1));
    }

    /** Addition operator **/
    gps_time_t operator+(const gps_time_t& t1) const {
        gps_time_t t(*this);

        t.is_not_set = false;

        t.gpstime.wn += t1.gpstime.wn;
        if (t.gpstime.ns + t1.gpstime.ns > 1000000)
            t.gpstime.tow += (t.gpstime.ns + t1.gpstime.ns) / 1000000;
        t.gpstime.ns = (t.gpstime.ns + t1.gpstime.ns) % 1000000;

        if (t.gpstime.tow + t1.gpstime.tow > MILLISECONDS_IN_WEEK)
            t.gpstime.wn += (t.gpstime.tow + t1.gpstime.tow) / MILLISECONDS_IN_WEEK;
        t.gpstime.tow = (t.gpstime.tow + t1.gpstime.tow) % MILLISECONDS_IN_WEEK;

        return t;
    }

    /** Add nanoseconds **/
    gps_time_t operator+(const unsigned long t1) const {
        gps_time_t t(*this);
        t.is_not_set = false;

        gps_time_t t2;
        t2.is_not_set = false;
        t2.gpstime.wn = 0;
        t2.gpstime.tow = 0;
        t2.gpstime.ns = t1;

        return t + t2;
    }

    /** Subtraction operator. Requires t1 is less than "this" or else
     *  there is undefined behavior. **/
    gps_time_t operator-(const gps_time_t& t1) const {
        gps_time_t t(*this);
        t.is_not_set = false;
        t.gpstime.wn -= t1.gpstime.wn;
        
        if (t.gpstime.ns - t1.gpstime.ns < 0) {
            t.gpstime.wn -= 1;
            t.gpstime.tow = MILLISECONDS_IN_WEEK - t.gpstime.tow + t1.gpstime.tow;
        }
        else {
            t.gpstime.tow = t.gpstime.tow - t1.gpstime.tow;
        }
        return t;
    }

    /** Subtract milliseconds **/
    gps_time_t operator-(const unsigned int t1) const {
        gps_time_t t(*this);
        t.is_not_set = false;

        gps_time_t t2;
        t2.is_not_set = false;
        t2.gpstime.wn = 0;
        t2.gpstime.tow = t1;

        return t - t2;
    }

    /** Cast to integer operator **/
    explicit operator unsigned int() {
        return gpstime.wn * MILLISECONDS_IN_WEEK + gpstime.tow;
    }
};

#endif