#ifndef GPSTIME_HPP_
#define GPSTIME_HPP_

#include <libsbp/navigation.h>

static constexpr uint64_t NANOSECONDS_IN_WEEK = 7 * 24 * 60 * 60 * (uint64_t) (1000000000);

struct gps_time_t {
    uint16_t wn;
    uint32_t tow;
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
    gps_time_t(uint16_t wn, uint32_t tow, uint64_t ns) {
        is_set = true;
        this->wn = wn;
        this->tow = tow;
        this->ns = ns;
    }

    /** Copy constructor **/
    explicit gps_time_t(const uint64_t t) {
        is_set = true;
        wn = t / NANOSECONDS_IN_WEEK;
        tow = (t - (wn * NANOSECONDS_IN_WEEK)) / 1000000;
        ns = (t - (wn * NANOSECONDS_IN_WEEK)) % 1000000;
    }

    // cppcheck-suppress noExplicitConstructor
    gps_time_t(const msg_gps_time_t &t) {
        is_set = true;
        wn = t.wn;
        tow = t.tow;
        ns = t.ns;
    }

    /** Cast to integer operator **/
    explicit operator uint64_t() const {
        return wn * NANOSECONDS_IN_WEEK + tow * 1000000 + ns;
    }

    /** A bunch of equality and comparison operators. **/
    bool operator==(const uint64_t t) const {
        if (!is_set) return false;
        return static_cast<uint64_t>(*this) == t;
    }
    bool operator==(const gps_time_t &t)   const {
        if (!t.is_set) return false;
        const uint64_t t_ns = static_cast<uint64_t>(t);
        return *this == t_ns;
    }
    bool operator<(const uint64_t t)  const {
        if (!is_set) return false;
        return static_cast<uint64_t>(*this) < t;
    }
    bool operator<(const gps_time_t &t)    const {
        if (!t.is_set) return false;
        const uint64_t t_ns = static_cast<uint64_t>(t);
        return *this < t_ns;
    }
    bool operator!=(const uint64_t t) const { return !(*this == t); }
    bool operator!=(const gps_time_t &t)   const { return !(*this == t); }
    bool operator>(const uint64_t t)  const { return !(*this < t || *this == t ); }
    bool operator>(const gps_time_t &t)    const { return !(*this < t || *this == t ); }
    bool operator<=(const uint64_t t) const { return !(*this > t); }
    bool operator<=(const gps_time_t &t)   const { return !(*this > t); }
    bool operator>=(const uint64_t t) const { return !(*this < t); }
    bool operator>=(const gps_time_t &t)   const { return !(*this < t); }

    /** Addition operators **/
    gps_time_t& operator+=(const uint64_t t) {
        is_set = true;
        *this = static_cast<gps_time_t>(static_cast<uint64_t>(*this) + t);
        return *this;
    }
    gps_time_t& operator+=(const gps_time_t &t) {
        *this += static_cast<uint64_t>(t);
        return *this;
    }

    /** Subtraction operator. Unsets the current object if the number of nanoseconds
     * is greater than the current time. **/
    gps_time_t& operator-=(const uint64_t t) {
        if (*this < t) {
            is_set = false;
            return *this;
        }

        is_set = true;
        *this = static_cast<gps_time_t>(static_cast<uint64_t>(*this) - t);
        return *this;
    }

    /** Subtraction operator. Requires t1 is less than "this" or else
     *  the current object becomes unset. **/
    gps_time_t& operator-=(const gps_time_t &t) {
        *this -= static_cast<uint64_t>(t);
        return *this;
    }
};

inline gps_time_t operator+(gps_time_t lhs, const gps_time_t& rhs) {
    lhs += rhs;
    return lhs;
}
inline gps_time_t operator+(gps_time_t lhs, const uint64_t rhs) {
    lhs += rhs;
    return lhs;
}
inline gps_time_t operator-(gps_time_t lhs, const gps_time_t& rhs) {
    lhs -= rhs;
    return lhs;
}
inline gps_time_t operator-(gps_time_t lhs, const uint64_t rhs) {
    lhs -= rhs;
    return lhs;
}

#endif
