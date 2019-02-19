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
    DEPLOYMENT_TIMER_1 = 0x11, // 1-byte integer. Each +1 increment represents a second.
    DEPLOYMENT_TIMER_2 = 0x12, // 1-byte integer. Each +1 increment represents a second.
    DEPLOYMENT_TIMER_3 = 0x13, // 1-byte integer. Each +1 increment represents a second.
    DEPLOYMENT_TIMER_4 = 0x14, // 1-byte integer. Each +1 increment represents a second.
    SAFE_HOLD_FLAG = 0x20, // Boolean: whether or not system is currently in safe hold
    SAFE_HOLD_TIMER_1 = 0x21, // 1-byte integer. Each +1 increment represents a second.
    SAFE_HOLD_TIMER_2 = 0x22, // 1-byte integer. Each +1 increment represents a second.
    SAFE_HOLD_TIMER_3 = 0x23, // 1-byte integer. Each +1 increment represents a second.
    SAFE_HOLD_TIMER_4 = 0x24, // 1-byte integer. Each +1 increment represents a second.
    INITIALIZATION_HOLD_FLAG = 0x30, // Boolean: whether or not system is currently in initialization hold
};

//! Prevents multiple-process access of the EEPROM.
extern mutex_t eeprom_lock;

#endif