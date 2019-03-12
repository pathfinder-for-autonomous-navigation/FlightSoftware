/** @file EEPROMAddresses.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definitions for EEPROM locations where permanent satellite data is stored.
 */

#include <ChRt.h>

#ifndef EEPROM_ADDRESSES_HPP_
#define EEPROM_ADDRESSES_HPP_

//! Stores mapping between physical EEPROM addresses and logical meaning of data stored at those addresses.
enum EEPROM_ADDRESSES {
    NUM_REBOOTS_H = 0x00, // 2 byte integer
    DEPLOYMENT = 0x10, // Boolean: wehther or not the system has completed the deployment phase
    DEPLOYMENT_TIMER = 0x11, // 4-byte integer. Each +1 increment represents a second.
    SAFE_HOLD_FLAG = 0x20, // Boolean: whether or not system is currently in safe hold
    SAFE_HOLD_TIMER = 0x21, // 4-byte integer. Each +1 increment represents a second.
    INITIALIZATION_HOLD_FLAG = 0x25, // Boolean: whether or not system is currently in initialization hold
    LAST_DOWNLINK_NUMBER = 0x30, // 4-byte integer; tracks the number of the latest downlink successfully sent
                                 // TODO work into downlink logic
    LAST_UPLINK_NUMBER = 0x34, // 4-byte integer; tracks the number of the latest uplink received
    IS_FOLLOWER = 0x40, // Boolean: whether or not satellite is follower
    FINAL_STATE_FLAG = 0x41, // Has value: 0 if satellite is not in a final state
                             //            1 if satellite is in docking mode
                             //            2 if satellite is in docked mode
                             //            3 if satellite is in paired mode
                             //            4 if satellite is in spacejunk mode
    PREFERRED_INTERTANK_VALVE = 0x50, // 0 if main valve is preferred
                                      // 1 if backup valve is preferred
};

//! Prevents multiple-process access of the EEPROM.
extern mutex_t eeprom_lock;

#endif