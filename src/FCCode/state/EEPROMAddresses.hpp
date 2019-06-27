/** @file EEPROMAddresses.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definitions for EEPROM locations where permanent satellite data is stored.
 */

#include <ChRt.h>

#ifndef EEPROM_ADDRESSES_HPP_
#define EEPROM_ADDRESSES_HPP_

//! Stores mapping between physical EEPROM addresses and logical meaning of data stored at those addresses.
static constexpr unsigned char offset = 0x00; // CHANGE BEFORE FLIGHT to 0x70
enum EEPROM_ADDRESSES {
    NUM_REBOOTS_H = offset + 0x00, // 2 byte integer
    DEPLOYMENT = offset + 0x10, // Boolean: wehther or not the system has completed the deployment phase
    DEPLOYMENT_TIMER = offset + 0x11, // 4-byte integer. Each +1 increment represents a second.
    SAFE_HOLD_FLAG = offset + 0x20, // Boolean: whether or not system is currently in safe hold
    SAFE_HOLD_TIMER = offset + 0x21, // 4-byte integer. Each +1 increment represents an hour.
    INITIALIZATION_HOLD_FLAG = offset + 0x25, // Boolean: whether or not system is currently in initialization hold
    HOURS_SINCE_SBDIX = offset + 0x30,    // 4-byte integer; counts hours since the last communication occurred with the
                                          // satellite
    IS_FOLLOWER = offset + 0x40, // Boolean: whether or not satellite is follower
    FINAL_STATE_FLAG = offset + 0x41, // See enum below for possible values.
    PREFERRED_INTERTANK_VALVE = offset + 0x50, // 0 if main valve is preferred
                                               // 1 if backup valve is preferred
    DEVICE_REBOOTS_PIKSI = offset + 0x60,          // 4-byte integer: number of times Piksi has been rebooted
    DEVICE_REBOOTS_QUAKE = offset + 0x64,          // 4-byte integer: number of times Quake has been rebooted
    DEVICE_REBOOTS_SPIKE_AND_HOLD = offset + 0x68, // 4-byte integer: number of times Spike and Hold has been rebooted
    DEVICE_REBOOTS_ADCS = offset + 0x6C,           // 4-byte integer: number of times ADCS has been rebooted
};

enum FINAL_STATES {
    NO_FINAL_STATE = 0,
    DOCKING = 1,
    DOCKED = 2,
    PAIRED = 3,
    SPACEJUNK = 4
};

//! Prevents multiple-process access of the EEPROM.
extern mutex_t eeprom_lock;

#endif