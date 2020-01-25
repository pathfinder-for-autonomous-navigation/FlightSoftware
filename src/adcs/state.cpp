//
// src/adcs/state.cpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#include "state.hpp"

namespace adcs {

struct Registers volatile registers = {
  0x0F, // Who am I
  0,    // Endianess
  0,    // ADCS Mode
  0,    // Read Pointer
  // Reaction Wheel Registers
  {
    0,                  // Mode
    {0.0f, 0.0f, 0.0f}, // Command
    0,                  // Command Flag
    1.0f,               // Momentum filter
    1.0f,               // Ramp filter
    {400.0f, -300.0f, 200.0f}, // Speed read
    {0.001f, 0.002f, -0.003f}  // Ramp read
  },
  // Magnetorquer Registers
  {
    0,                  // Mode
    {0.0f, 0.0f, 0.0f}, // Command
    1.0f,               // Magnetic moment limiter
    0                   // Command flag
  },
  // Sun Sensor Registers
  {
    0,                             // Mode
    {0.69f, 0.42f, -.88f},            // Sun vector read
    1.0f,                          // Voltage filter
    {1.0f, 2.0f, 3.0f, 0.0f, 0.0f, // Voltage read
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f,
     1.0f, 2.0f, 3.0f, 0.0f, 0.0f}
  },
  // IMU Registers
  {
    0,                  // Mode
    {0.001f, 0.0008f, -0.0006f}, // Magnetometer read
    {3.0f, 1.5f, -1.2f}, // Gyroscope read
    42.0f,               // Gyroscope temperature read
    1.0f,               // Magnetometer filter
    1.0f,               // Gyroscope filter
    1.0f,               // Gyroscoope temperature filter
    0.0f,               // TODO : Gyroscope temperature K_p
    0.0f,               // TODO : Gyroscope temperature K_i
    0.0f,               // TODO : Gyroscope temperature K_d
    0.0f,               // TODO : Gyroscope desired temperature
  },
  // HAVT Registers
  {
    0xF0F01F1F, // Read register
    0, // Cmd register
    0, // Cmd flag
  }
};
}  // namespace adcs
