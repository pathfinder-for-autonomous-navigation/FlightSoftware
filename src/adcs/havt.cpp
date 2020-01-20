//
// src/havt.cpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//   Shihao Cao  sfc72@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifdef HAVT_DEBUG
#define DEBUG
#endif

#include <adcs_constants.hpp>
#include <adcs_havt_devices.hpp>

#include <adcs/havt.hpp>
#include <adcs/imu.hpp>
#include <adcs/mtr.hpp>
#include <adcs/rwa.hpp>
#include <adcs/ssa.hpp>
#include <adcs/state.hpp>
#include <adcs/utl/debug.hpp>

namespace havt {

// define internal_table, an easier representation of the read havt register
std::bitset<havt::max_devices> internal_table;

// define dev_ptrs
dev::Device* dev_ptrs[havt::max_devices] = {
  &imu::gyr,
  &imu::mag1,
  &imu::mag2,
  &mtr::mtrs[0],
  &mtr::mtrs[1],
  &mtr::mtrs[2],
  &rwa::potentiometer,
  &rwa::wheels[0],
  &rwa::wheels[1],
  &rwa::wheels[2],
  &rwa::adcs[0],
  &rwa::adcs[1],
  &rwa::adcs[2],
  &ssa::adcs[0],
  &ssa::adcs[1],
  &ssa::adcs[2],
  &ssa::adcs[3],
  &ssa::adcs[4],
  // TODO: add gyro heater addr
};

void update_read_table(){
  //Loop until you reach _LENGTH, yes this effectively limits our max_devices to 31
  for (unsigned int index_int = adcs_havt::Index::IMU_GYR; index_int < adcs_havt::Index::_LENGTH; index_int++ )
  {
    internal_table.set(index_int, dev_ptrs[index_int]->is_functional());
  }

  // Set all extra bits of the internal table to 0, just in case radiation
  for (unsigned int index_int = adcs_havt::Index::_LENGTH; index_int < havt::max_devices; index_int++)
  {
    internal_table.set(index_int, 0);
  }
}

void cmd_device(adcs_havt::Index index, const bool cmd_bit){
  DEBUG_print(String(index) + "," + String(cmd_bit) + " - ")
  
  // if statement prevents attempting commands with nonexistent devices
  if(index<adcs_havt::Index::_LENGTH){
    // command according to cmd_bit
    if(cmd_bit)
      dev_ptrs[index]->reset();
    else{
      dev_ptrs[index]->disable();
    }
  }
}

void execute_cmd_table(const std::bitset<havt::max_devices>& cmd_table){

  // Loop until you reach _LENGTH, yes this effectively limits our max_devices to 31
  for (unsigned int index_int = adcs_havt::Index::IMU_GYR; index_int != adcs_havt::Index::_LENGTH; index_int++ )
  {
    adcs_havt::Index index = static_cast<adcs_havt::Index>(index_int);

    // if the internal table and the cmd_table are different, need to actuate according to cmd_table
    if(internal_table.test(index_int) != cmd_table.test(index_int)){  
      cmd_device(index, cmd_table.test(index_int));
    }
  }

  DEBUG_printlnF("")
  return;
}
}  // namespace havt
