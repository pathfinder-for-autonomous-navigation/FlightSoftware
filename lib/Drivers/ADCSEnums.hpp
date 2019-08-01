//
// include/global.hpp
// ADCS
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

#ifndef PAN_ADCS_INCLUDE_GLOBAL_HPP_
#define PAN_ADCS_INCLUDE_GLOBAL_HPP_

inline namespace global_1_0 {
/** \enum Mode
 *  @brief Outlines all modes of the ADCS system. **/
enum Mode : unsigned char {
    /** @brief Passive, non-actuating mode for the ADCS system. **/
    PASSIVE = 0,
    /** @brief Active, actuating mode for the ADCS system. **/
    ACTIVE = 1
};

/** \enum RWAMode
 *  @brief Outlines all modes of the reaction wheels. **/
enum RWAMode : unsigned char {
    /** **/
    DISABLED = 0,
    /** @brief Speed controlled mode. **/
    SPEED_CTRL = 1,
    /** @brief Acceleration controlled mode. **/
    ACCEL_CTRL = 2
};

/** \enum SSAMode
 *  @brief OUtlines all modes of the sun sensors. **/
enum SSAMode : unsigned char {
    /** @brief Conversion to sun vector in progress. **/
    IN_PROGRESS,
    /** @brief Succesful conversion has been completed. **/
    COMPLETE,
    /** @brief Conversion failed to determine a reliable sun vector. **/
    FAILURE
};

/** \enum CommandFlag
 *  @brief Outlines the flag values for an updated actuation command. **/
enum CommandFlag : unsigned char {
    /** @brief No new actuation command present. **/
    OLD_COMMAND = 0,
    /** @brief New actutation command is present. **/
    NEW_COMMAND
};

/** \struct ReactionWheelRegisters
 *  @brief Outlines all i2c registers for reaction wheel inputs and outputs. **/
struct ReactionWheelRegisters {
    /** @brief Specifies the reaction wheels operating mode. **/
    unsigned char mode;
    /** @brief Reaction wheel commanded **/
    float cmd[3];
    /** @brief Reaction wheel actutation command flag. **/
    unsigned char cmd_flg;
    /** @brief Motor speed read exponential filter constant. **/
    float speed_flt;
    /** @brief Motor ramp read exponential filter constant. **/
    float ramp_flt;
    /** @brief Current speed command in rpm. **/
    float speed_cmd_rd[3];
    /** @brief Current speed value for the motors in rpm. **/
    float speed_rd[3];
    /** @brief Current ramp value for the motors in volts. **/
    float ramp_rd[3];
};

/** \struct MagneticTorquerRegisters
 *  @brief Outlines all i2c registers for the magnetic torque rod inputs and
 *         outputs. **/
struct MagneticTorquerRegisters {
    /** @brief Commanded magnetic moment in Am^2. **/
    float cmd[3];
    /** @brief MTR actutation command flag. **/
    unsigned char cmd_flg;
};

/** \struct SunSensorRegisters
 *  @brief Outlines all i2c registers for sun sensor inputs and outputs. **/
struct SunSensorRegisters {
    /** @brief Sun sensor ADC exponential filter constant. **/
    float adc_flt;
    /** @brief Starts a conversion and signals when one has been complete. **/
    unsigned char mode;
    /** @brief Sun vector result from the most recent conversion. **/
    float vec_rd[3];
    /** @brief Analog to digital conversion reads for each sensor in volts. **/
    float adc_rd[20];
};

/** \struct IMURegisters
 *  @brief Outlines all i2c registers for IMU inputs and outputs. **/
struct IMURegisters {
    /** @brief Gyroscope reading exponential filter constant. **/
    float gyr_flt;
    /** @brief Magnetometer reading exponential filter constant. **/
    float mag_flt;
    /** @brief Gyroscope reading in the body frame in radians/s. **/
    float gyr_rd[3];
    /** @brief Magnetometer reading in the body frame in gauss. **/
    float mag_rd[3];
};

/** \struct Registers
 *  @brief Outlines all i2c register data and addresses. **/
struct Registers {
    /** @brief Sets the mode of the ADCS box. **/
    unsigned char mode;
    /** @brief Sets the read register pointer for data requests. **/
    unsigned short read_ptr;
    /** @brief Reaction wheel registers. **/
    struct ReactionWheelRegisters rwa;
    /** @brief Magnetic torque rode registers. **/
    struct MagneticTorquerRegisters mtr;
    /** @brief Sun sensor registers. **/
    struct SunSensorRegisters ssa;
    /** @brief IMU registers. **/
    struct IMURegisters imu;
    /** @brief Hardware availability table bit string. **/
    unsigned char hat[1];
};

/** @brief External, volatile struct to hold all i2c register information. **/
extern struct Registers volatile reg;

}  // namespace global_1_0

#endif
