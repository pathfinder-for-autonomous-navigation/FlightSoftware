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
    NUM_REBOOTS_L = 0x01,
    DEPLOYMENT = 0x10, // Boolean: wehther or not the system has completed the deployment phase
    DEPLOYMENT_TIMER_1 = 0x11, // 4-byte integer. Each +1 increment represents a second.
    DEPLOYMENT_TIMER_2 = 0x12,
    DEPLOYMENT_TIMER_3 = 0x13,
    DEPLOYMENT_TIMER_4 = 0x14,
    SAFE_HOLD_FLAG = 0x20, // Boolean: whether or not system is currently in safe hold
    SAFE_HOLD_TIMER_1 = 0x21, // 4-byte integer. Each +1 increment represents a second.
    SAFE_HOLD_TIMER_2 = 0x22,
    SAFE_HOLD_TIMER_3 = 0x23,
    SAFE_HOLD_TIMER_4 = 0x24,
    INITIALIZATION_HOLD_FLAG = 0x25, // Boolean: whether or not system is currently in initialization hold
    LAST_DOWNLINK_NUMBER_1 = 0x30, // 4-byte integer; tracks the number of the latest downlink successfully sent
    LAST_DOWNLINK_NUMBER_2 = 0x31, // TODO work into downlink logic
    LAST_DOWNLINK_NUMBER_3 = 0x32,
    LAST_DOWNLINK_NUMBER_4 = 0x33,
    LAST_UPLINK_NUMBER_1 = 0x34, // 4-byte integer; tracks the number of the latest uplink received
    LAST_UPLINK_NUMBER_2 = 0x35, // TODO work into uplink logic
    LAST_UPLINK_NUMBER_3 = 0x36,
    LAST_UPLINK_NUMBER_4 = 0x37
};

//! Prevents multiple-process access of the EEPROM.
extern mutex_t eeprom_lock;

#endif