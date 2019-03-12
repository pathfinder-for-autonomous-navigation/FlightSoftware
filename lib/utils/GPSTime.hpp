#ifndef GPSTIME_HPP_
#define GPSTIME_HPP_

#include <Piksi/libsbp/navigation.h>

static constexpr unsigned int MILLISECONDS_IN_WEEK = 7*24*60*60*1000;

struct gps_time_t {
    msg_gps_time_t gpstime;
    bool is_not_set;

    /** Default constructor **/
    gps_time_t() {
        gpstime.wn = 0;
        gpstime.tow = 0;
        is_not_set = true;
    }

    /** Copy constructor **/
    gps_time_t(const msg_gps_time_t& t) {
        gpstime.wn = t.wn;
        gpstime.tow = t.tow;
        is_not_set = false;
    }

    /** Copy constructor **/
    gps_time_t(const gps_time_t& t) {
        gpstime.wn = t.gpstime.wn;
        gpstime.tow = t.gpstime.tow;
        is_not_set = false;
    }

    /** Assignment operator **/
    gps_time_t& operator=(const msg_gps_time_t& t) {
        this->gpstime.wn = t.wn;
        this->gpstime.tow = t.tow;
        is_not_set = false;
        return *(this);
    }
    
    /** Assignment operator  **/
    gps_time_t& operator=(const gps_time_t& t) {
        this->gpstime.wn = t.gpstime.wn;
        this->gpstime.tow = t.gpstime.tow;
        is_not_set = false;
        return *(this);
    }

    /** Inequality operator **/
    bool operator!=(const gps_time_t& t1) {
        return t1.gpstime.wn == gpstime.wn && t1.gpstime.tow != gpstime.tow;
    }

    /** Inequality operator **/
    bool operator!=(const unsigned int& t1) {
        return gpstime.wn * 7*24*60*60*1000 + gpstime.tow != t1;
    }

    /** Equality operator **/
    bool operator==(const gps_time_t& t1) {
        return t1.gpstime.wn == gpstime.wn && t1.gpstime.tow == gpstime.tow;
    }

    /** Equality operator **/
    bool operator==(const unsigned int& t1) {
        return gpstime.wn * 7*24*60*60*1000 + gpstime.tow == t1;
    }

    /** Comparison operator **/
    bool operator<(const gps_time_t& t1) {
        return gpstime.wn < t1.gpstime.wn || (gpstime.wn == t1.gpstime.wn && gpstime.tow < t1.gpstime.tow);
    }

    /** Comparison operator **/
    bool operator<(const unsigned int& t1) {
        return gpstime.wn * 7*24*60*60*1000 + gpstime.tow < t1;
    }

    /** Comparison operator **/
    bool operator>(const gps_time_t& t1) {
        return !(this->operator<(t1)) && !(this->operator==(t1));
    }

    /** Comparison operator **/
    bool operator>(const unsigned int& t1) {
        return gpstime.wn * 7*24*60*60*1000 + gpstime.tow > t1;
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
        if (t.gpstime.tow + t1.gpstime.tow > MILLISECONDS_IN_WEEK)
            t.gpstime.wn += (t.gpstime.tow + t1.gpstime.tow) / MILLISECONDS_IN_WEEK;
        t.gpstime.tow = (t.gpstime.tow + t1.gpstime.tow) % MILLISECONDS_IN_WEEK;

        return t;
    }

    /** Add milliseconds **/
    gps_time_t operator+(const unsigned int t1) const {
        gps_time_t t(*this);
        t.is_not_set = false;

        gps_time_t t2;
        t2.is_not_set = false;
        t2.gpstime.wn = 0;
        t2.gpstime.tow = t1;

        return t + t2;
    }

    /** Subtraction operator **/
    gps_time_t operator-(const gps_time_t& t1) const {
        gps_time_t t(*this);
        t.is_not_set = false;

        if (t.gpstime.wn < t1.gpstime.wn) t.gpstime.wn = 0;
        else t.gpstime.wn -= t1.gpstime.wn;
        
        if (t.gpstime.tow - t1.gpstime.tow < 0) {
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