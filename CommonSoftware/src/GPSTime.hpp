#ifndef GPSTIME_HPP_
#define GPSTIME_HPP_

#include <libsbp/navigation.h>

static constexpr unsigned long NANOSECONDS_IN_WEEK = 7 * 24 * 60 * 60 * (unsigned long) (1000000000);

struct gps_time_t {
    unsigned short wn;
    unsigned int tow;
    int ns;
    bool is_set;

    /** Default constructor **/
    gps_time_t() {
        is_set = false;
        wn = 0;
        tow = 0;
        ns = 0;
    }

    /** Argumented constructor **/
    gps_time_t(unsigned short wn, unsigned int tow, unsigned long ns) {
        is_set = true;
        this->wn = wn;
        this->tow = tow;
        this->ns = ns;
    }

    /** Copy constructor **/
    explicit gps_time_t(const unsigned long t) {
        is_set = true;
        wn = t / NANOSECONDS_IN_WEEK;
        tow = (t - (wn * NANOSECONDS_IN_WEEK)) / 1000000;
        ns = (t - (wn * NANOSECONDS_IN_WEEK)) % 1000000;
    }

    gps_time_t(const msg_gps_time_t &t) {
        is_set = true;
        wn = t.wn;
        tow = t.tow;
        ns = t.ns;
    }

    /** Cast to integer operator **/
    explicit operator unsigned long() const {
        return wn * NANOSECONDS_IN_WEEK + tow * 1000000 + ns;
    }

    /** A bunch of equality and comparison operators. **/
    bool operator==(const unsigned long t) const {
        if (!is_set) return false;
        return static_cast<unsigned long>(*this) == t;
    }
    bool operator==(const gps_time_t &t)   const {
        if (!t.is_set) return false;
        const unsigned long t_ns = static_cast<unsigned long>(t);
        return *this == t_ns;
    }
    bool operator<(const unsigned long t)  const {
        if (!is_set) return false;
        return static_cast<unsigned long>(*this) < t;
    }
    bool operator<(const gps_time_t &t)    const {
        if (!t.is_set) return false;
        const unsigned long t_ns = static_cast<unsigned long>(t);
        return *this < t_ns;
    }
    bool operator!=(const unsigned long t) const { return !(*this == t); }
    bool operator!=(const gps_time_t &t)   const { return !(*this == t); }
    bool operator>(const unsigned long t)  const { return !(*this < t || *this == t ); }
    bool operator>(const gps_time_t &t)    const { return !(*this < t || *this == t ); }
    bool operator<=(const unsigned long t) const { return !(*this > t); }
    bool operator<=(const gps_time_t &t)   const { return !(*this > t); }
    bool operator>=(const unsigned long t) const { return !(*this < t); }
    bool operator>=(const gps_time_t &t)   const { return !(*this < t); }

    /** Addition operators **/
    gps_time_t& operator+=(const unsigned long t) {
        is_set = true;
        *this = static_cast<gps_time_t>(static_cast<unsigned long>(*this) + t);
        return *this;
    }
    gps_time_t& operator+=(const gps_time_t &t) {
        *this += static_cast<unsigned long>(t);
        return *this;
    }

    /** Subtraction operator. Unsets the current object if the number of nanoseconds
     * is greater than the current time. **/
    gps_time_t& operator-=(const unsigned long t) {
        if (*this < t) {
            is_set = false;
            return *this;
        }

        is_set = true;
        *this = static_cast<gps_time_t>(static_cast<unsigned long>(*this) - t);
        return *this;
    }

    /** Subtraction operator. Requires t1 is less than "this" or else
     *  the current object becomes unset. **/
    gps_time_t& operator-=(const gps_time_t &t) {
        *this -= static_cast<unsigned long>(t);
        return *this;
    }
};

inline gps_time_t operator+(gps_time_t lhs, const gps_time_t& rhs) {
    lhs += rhs;
    return lhs;
}
inline gps_time_t operator+(gps_time_t lhs, const unsigned long rhs) {
    lhs += rhs;
    return lhs;
}
inline gps_time_t operator-(gps_time_t lhs, const gps_time_t& rhs) {
    lhs -= rhs;
    return lhs;
}
inline gps_time_t operator-(gps_time_t lhs, const unsigned long rhs) {
    lhs -= rhs;
    return lhs;
}

#endif
