//
// src/adcs/dev/LIS2MDLTR.cpp
// FlightSoftware
//
// Contributors:
//   Anusha Choudhury ac978@cornell.edu
//   Kyle Krol        kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "LIS2MDLTR.hpp"

namespace adcs {
namespace dev {

void LIS2MDLTR::setup(i2c_t3 *wire, unsigned long timeout) {
  this->I2CDevice::setup(wire, 0b0011110, timeout);
}

bool LIS2MDLTR::reset() {
  this->I2CDevice::reset();
  while (this->is_functional()) {
    this->i2c_begin_transmission();
    this->i2c_write(REG::CFG_A);
    this->i2c_write(this->sample_rate);
    this->i2c_end_transmission(I2C_NOSTOP);
    this->i2c_begin_transmission();
    this->i2c_write(REG::CFG_C);
    this->i2c_write((uint8_t)(0b10000)); // BDU enable
    this->i2c_end_transmission();
    if (!this->i2c_pop_errors()) return true;
  }
  return false;
}

void LIS2MDLTR::disable() {
  this->I2CDevice::disable();
  this->i2c_begin_transmission();
  this->i2c_write(REG::CFG_A);
  this->i2c_write((uint8_t)(this->sample_rate | 0b11));
  this->i2c_end_transmission();
}

bool LIS2MDLTR::is_ready() {
  this->i2c_begin_transmission();
  this->i2c_write(REG::STATUS);
  this->i2c_end_transmission(I2C_NOSTOP);
  this->i2c_request_from(1);
  return (!this->i2c_pop_errors()) && (this->i2c_read() & 0b1000);
}

bool LIS2MDLTR::read() {
  uint8_t buffer[6];
  // Request measurement data
  this->i2c_begin_transmission();
  this->i2c_write(REG::XOUT_LOW);
  this->i2c_end_transmission(I2C_NOSTOP);
  this->i2c_request_from(6);
  // Read in measurement data
  this->i2c_read(buffer, 6);
  if (this->i2c_pop_errors()) return false;
  // Process measurement data
  this->b_vec[0] = (buffer[0] << 0) | (buffer[1] << 8);
  this->b_vec[1] = (buffer[2] << 0) | (buffer[3] << 8);
  this->b_vec[2] = (buffer[4] << 0) | (buffer[5] << 8);
  return true;
}
}  // namespace dev
}  // namespace adcs

// /*
//     Magnetometer::Magnetometer(i2c_t3& wire,uint8_t addr,unsigned long timeout)
//     :I2CDevice(wire,Magnetometer::ADDR,2000){
//     }*/
    
//     void Magnetometer::new_read_1(){
        
//         this->i2c_begin_transmission();
//         this->i2c_write(Magnetometer::REGISTERS::CFG_A);
//         this->i2c_write(Magnetometer::SINGLE);
//         this->i2c_end_transmission();

//         do{
//             this->i2c_begin_transmission();
//             this->i2c_write(Magnetometer::REGISTERS::STATUS);
//             this->i2c_end_transmission();
//             this->i2c_request_from(1);
//             Magnetometer::stat = this->i2c_read()&8;
//         }
//         while(!(Magnetometer::stat == 8));

//         this->i2c_begin_transmission();
//         this->i2c_write(Magnetometer::REGISTERS::OUT);
//         this->i2c_end_transmission();
//         this->i2c_request_from(6);
//         this->i2c_read(Magnetometer::rdat, 6);
       
//     }

//     void Magnetometer::new_read_2(){
//         this->i2c_begin_transmission();
//         this->i2c_write(Magnetometer::REGISTERS::CFG_A);
//         this->i2c_write(1);
//         this->i2c_end_transmission();
//         do{
//             this->i2c_begin_transmission();
//             this->i2c_write(Magnetometer::REGISTERS::STATUS);
//             this->i2c_end_transmission();
//             this->i2c_request_from(1);
//             Magnetometer::stat = this->i2c_read()&8;
//         }
//         while(!(Magnetometer::stat == 8));

//         this->i2c_begin_transmission();
//         this->i2c_write(Magnetometer::REGISTERS::OUT);
//         this->i2c_end_transmission();
//         this->i2c_request_from(6);
//         this->i2c_read(Magnetometer::sdat, 6);
       
//     }
