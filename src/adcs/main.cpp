//
// src/adcs/main.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

//
// TODO :
// Complete i2c function documentation.
// Add in HAT write capability.
// Add endianess flag.
// Set slave address.
// Serial logging support.
// Make everything in the body frame and add in transformation matrix
// Add delay to allow gyro to start up
//   15 ms till communication can begin
//   wait 70 ms to take the first read
//   discard 3 samples at 52 Hz 
//

#include "constants.hpp"
#include "havt.hpp"
#include "havt_devices.hpp"
#include "imu.hpp"
#include "mtr.hpp"
#include "rwa.hpp"
#include "ssa.hpp"
#include "state.hpp"
#include "state_controller.hpp"
#include "utl/logging.hpp"

#include <Arduino.h>
#include <i2c_t3.h>
#include <lin.hpp>

#include <bitset>

using namespace adcs;

// TODO : Look into the proper initialization for the slave i2c bus and whether
//        a clock frequency needs to be included

/** @class InterruptSafe
 *  Simple RAII based mechanism to protect critical code. */
class InterruptSafe {
 public:
  InterruptSafe() { noInterrupts(); }
  ~InterruptSafe() { interrupts(); }
};

void setup() {
  LOG_init(9600)

#if LOG_LEVEL >= LOG_LEVEL_ERROR
  delay(5000);
#endif

  LOG_INFO_header
  LOG_INFO_println("Logging interface initialized with logging level "
      + String(LOG_LEVEL))

  // Initialize master I2C busses
  Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_37_38, I2C_PULLUP_EXT, 400000);
  Wire2.begin(I2C_MASTER, 0x00, I2C_PINS_3_4, I2C_PULLUP_EXT, 400000);

  LOG_INFO_header
  LOG_INFO_printlnF("Initialized sensor I2C busses")

  LOG_INFO_header
  LOG_INFO_printlnF("Pausing for 200 ms for sensors to initialize")

  // Wait for sensors to boot up
  delay(200);

  LOG_INFO_header
  LOG_INFO_printlnF("Complete")

  // Initialize all modules
  imu::setup();
  mtr::setup();
  rwa::setup();
  ssa::setup();

  loop();

  LOG_INFO_header
  LOG_INFO_printlnF("Module initialization complete")

  // Initialize slave I2C bus with address 0x4E
  Wire.begin(I2C_SLAVE, 0x4E);
  Wire.onReceive(umb::on_i2c_recieve);
  Wire.onRequest(umb::on_i2c_request);

  LOG_INFO_header
  LOG_INFO_printlnF("Umbilical I2C interface initialized")

  LOG_WARN_header
  LOG_WARN_printlnF("Initialization process complete; entering main loop")

}

void update_havt() {

  //update internal table
  havt::update_read_table();

  //set register to the internal table.
  registers.havt.read_table = (unsigned int)havt::internal_table.to_ulong();

  //if new command, actuate on reset_table
  if(registers.havt.cmd_reset_flg == CMDFlag::UPDATED){
    // Attempt atomic copy of the havt command reset
    registers.havt.cmd_reset_flg = CMDFlag::OUTDATED;
    unsigned int command_int = registers.havt.cmd_reset_table;

    // Actuate if the copy was atomic
    if (registers.havt.cmd_reset_flg == CMDFlag::OUTDATED){
      std::bitset<havt::max_devices> temp_command_table(command_int);
      havt::execute_cmd_reset_table(temp_command_table);
    }
  }

  //if new command, execute distable table
  if(registers.havt.cmd_disable_flg == CMDFlag::UPDATED){
    registers.havt.cmd_disable_flg = CMDFlag::OUTDATED;
    unsigned int command_int = registers.havt.cmd_disable_table;
    if (registers.havt.cmd_disable_flg == CMDFlag::OUTDATED){
      std::bitset<havt::max_devices> temp_command_table(command_int);
      havt::execute_cmd_disable_table(temp_command_table);
    }
  }
}

void update_imu() {
  // Update imu readings and copy into the proper registers
  imu::update_sensors(registers.imu.mag1_mode,
      registers.imu.mag2_mode, registers.imu.mag_flt, registers.imu.gyr_flt,
      registers.imu.gyr_desired_temp, registers.imu.gyr_temp_flt,
      registers.imu.gyr_temp_pwm);

  // Copy results into the global registers
  { InterruptSafe safe;
    registers.imu.mag1_rd[0] = imu::mag1_rd(0);
    registers.imu.mag1_rd[1] = imu::mag1_rd(1);
    registers.imu.mag1_rd[2] = imu::mag1_rd(2);
    registers.imu.mag2_rd[0] = imu::mag2_rd(0);
    registers.imu.mag2_rd[1] = imu::mag2_rd(1);
    registers.imu.mag2_rd[2] = imu::mag2_rd(2);
    registers.imu.gyr_rd[0] = imu::gyr_rd(0);
    registers.imu.gyr_rd[1] = imu::gyr_rd(1);
    registers.imu.gyr_rd[2] = imu::gyr_rd(2);
  }
  registers.imu.mag1_mode = IMUMAGMode::IMU_MAG_NORMAL;
  registers.imu.mag2_mode = IMUMAGMode::IMU_MAG_NORMAL;
  registers.imu.gyr_temp_rd = imu::gyr_temp_rd;
}

// TODO : Test that this controller works as expected

/** \fn update_mtr
 *  Updates the magnetic torque rod system according to the current status of
 *  the state struct. Actuation only occurs if the ADCS is in active mode, the
 *  magnetic torque rods are enabled, and the command vector was copied
 *  atomically. */
void update_mtr() {
  unsigned char mtr_mode;
  lin::Vector3f mtr_cmd;

  // Check for valid ADCS mode and the current command is new
  if (registers.mode != ADCSMode::ADCS_ACTIVE || registers.mtr.cmd_flg != CMDFlag::UPDATED) return;
  // Attempt atomic copy of the magnetic torque rod mode and command
  registers.mtr.cmd_flg = CMDFlag::OUTDATED;
  mtr_mode = registers.mtr.mode;
  mtr_cmd = {registers.mtr.cmd[0], registers.mtr.cmd[1], registers.mtr.cmd[2]};
  // Actuate if the copy was atomic
  if (registers.mtr.cmd_flg == CMDFlag::OUTDATED)
    mtr::actuate(mtr_mode, mtr_cmd, registers.mtr.moment_limit);
}

/** \fn update_rwa 
 *   */
void update_rwa() {
  unsigned char rwa_mode;
  lin::Vector3f rwa_cmd;

  // Check for valid ADCS mode and the current command is new
  if (registers.mode != ADCSMode::ADCS_ACTIVE || registers.rwa.cmd_flg != CMDFlag::UPDATED)
  {
    rwa_mode = 0; //stop wheels
    rwa::actuate(rwa_mode, rwa_cmd);
    return;
  }
  // Attempt atomic copy of the reaction wheel mode and command
  registers.rwa.cmd_flg = CMDFlag::OUTDATED;
  rwa_mode = registers.rwa.mode;
  rwa_cmd = {registers.rwa.cmd[0], registers.rwa.cmd[1], registers.rwa.cmd[2]};
  // Actuate if the copy was atomic
  if (registers.rwa.cmd_flg == CMDFlag::OUTDATED)
    rwa::actuate(rwa_mode, rwa_cmd);

  // Update reaction wheel readings
  rwa::update_sensors(registers.rwa.momentum_flt, registers.rwa.ramp_flt);

  // Copy results into the global registers
  { InterruptSafe safe;
    registers.rwa.momentum_rd[0] = rwa::speed_rd(0);
    registers.rwa.momentum_rd[1] = rwa::speed_rd(1);
    registers.rwa.momentum_rd[2] = rwa::speed_rd(2);
    registers.rwa.ramp_rd[0] = rwa::ramp_rd(0);
    registers.rwa.ramp_rd[1] = rwa::ramp_rd(1);
    registers.rwa.ramp_rd[2] = rwa::ramp_rd(2);
  }
}

/** \fn update_ssa
 *  Updates the sun sensor voltage measurements and calculates a sun vector if
 *  requested. The sun vector calculation is triggered by the state struct. The
 *  state struct is updated on the completion of a sun vector calculation. */
void update_ssa() {
  lin::Vector3f ssa_sun_vec;
  unsigned char ssa_mode;

  // Update sun sensor readings
  ssa::update_sensors(registers.ssa.voltage_flt);
  for (unsigned int i = 0; i < 20; i++)
    registers.ssa.voltage_rd[i] = ssa::voltages(i);

  // Check if sun vector calculation is requested
  if (registers.ssa.mode == SSAMode::SSA_IN_PROGRESS) {
    ssa_mode = ssa::calculate_sun_vector(ssa_sun_vec);
    { InterruptSafe safe;
      registers.ssa.sun_vec_rd[0] = ssa_sun_vec(0);
      registers.ssa.sun_vec_rd[1] = ssa_sun_vec(1);
      registers.ssa.sun_vec_rd[2] = ssa_sun_vec(2);
      registers.ssa.mode = ssa_mode;
    }
  }
}

#if LOG_LEVEL >= LOG_LEVEL_INFO
static unsigned long cycles = 0;
static unsigned long last_info_time = millis();
#endif

void loop() {
  update_imu();
  update_mtr();
  update_rwa();
  update_ssa();
  update_havt();

#if LOG_LEVEL >= LOG_LEVEL_INFO
  cycles++;

  if (millis() - last_info_time > 1000) {
    last_info_time = millis();

    LOG_INFO_header
    LOG_INFO_println("Heartbeat cycle count " + String(cycles))

    LOG_INFO_header
    LOG_INFO_println("mode     " + String(registers.mode))
    LOG_INFO_header
    LOG_INFO_println("mtr.mode " + String(registers.mtr.mode))
    LOG_INFO_header
    LOG_INFO_println("rwa.mode " + String(registers.rwa.mode))
    LOG_INFO_header
    LOG_INFO_println("ssa.mode " + String(registers.ssa.mode))

    std::bitset<havt::max_devices> temp_bitset(registers.havt.read_table);
    char buffer[34];
    for(int i = 0; i<16; i++){
      if(temp_bitset.test(31-i))
        buffer[i] = '1';
      else
        buffer[i] = '0';
    }
    buffer[16] = ' ';
    for(int i = 16; i<32; i++){
      if(temp_bitset.test(31-i))
        buffer[i+1] = '1';
      else
        buffer[i+1] = '0';
    }
    buffer[33] = '\0';

    LOG_INFO_header
    LOG_INFO_print("havt.read ")
    LOG_INFO_println(buffer)
  }
#endif
}
