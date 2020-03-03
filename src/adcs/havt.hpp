//
// src/adcs/havt.hpp
// FlightSoftware
//
// Contributors:
//   Shihao Cao  sfc72@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef SRC_ADCS_HAVT_HPP_
#define SRC_ADCS_HAVT_HPP_

#include "constants.hpp" // include necessary so that havt::max_device is defined
#include "dev/Device.hpp" // for dev_ptrs

#include <bitset>

namespace adcs {
namespace havt {

/**
 * @brief An array of pointers to all the dev::Device's on the ADCSBox.
 * 
 */
extern dev::Device* dev_ptrs[havt::max_devices];

/**
 * @brief internal_table is just a nicer object to work with than an uint32_t
 * 
 * It is intended store identical information as the read havt register at all times
 */
extern std::bitset<havt::max_devices> internal_table;

/**
 * @brief Uses dev_ptrs to call is_functional() of each device and store into internal_table
 * 
 */
void update_read_table();

/**
 * @brief Commands a reset() for a device if that bit is high
 */
void execute_cmd_reset_table(const std::bitset<havt::max_devices>& cmd_reset_table);

/**
 * @brief Commands a disable() for a device if that bit is high
 */
void execute_cmd_disable_table(const std::bitset<havt::max_devices>& cmd_disable_table);

}  // namespace havt
}  // namespace adcs

#endif