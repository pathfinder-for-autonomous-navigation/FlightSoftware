//
// src/adcs/dev/ADS1015.hpp
// FlightSoftware
//
// Contributors:
//   Kyle Krol  kpk63@cornell.edu
//
// Pathfinder for Autonomous Navigation
// Space Systems Design Studio
// Cornell Univeristy
//

// TODO :
// Test version 2_0 with the differential reads as well

#ifndef SRC_ADCS_DEV_ADS1015_HPP_
#define SRC_ADCS_DEV_ADS1015_HPP_

#include "I2CDevice.hpp"

namespace adcs {
namespace dev {

/** @class ADS1015
 *  Single shot conversion driver for the ADS1015 using the alert/ready pin. */
class ADS1015 : public I2CDevice {
 public:
  /** @enum ADDR
   *  Possible address values for the ADS1015. */
  enum ADDR : unsigned char {
    GND = 0x48,
    VDD = 0x49,
    SSDA = 0x4A,
    SSCL = 0x4B
  };
  /** @enum GAIN
   *   Possible gain values for the ADS1015. */
  enum GAIN : unsigned short {
    /** Offers a full range input of =/- 6.144 V. */
    TWO_THIRDS = 0x0000,
    /** Offers a full range input of =/- 4.096 V. */
    ONE = 0x0200,
    /** Offers a full range input of =/- 2.048 V and is the default. */
    TWO = 0x0400,
    /** Offers a full range input of =/- 1.024 V. */
    FOUR = 0x0500,
    /** Offers a full range input of =/- 0.512 V. */
    EIGHT = 0x0800,
    /** Offers a full range input of =/- 0.256 V. */
    SIXTEEN = 0x0A00
  };
  /** @enum SR
   *  Possible sample rates for the ADS1015. */
  enum SR : unsigned short {
    SPS_128 = 0x0000,
    SPS_250 = 0x0020,
    SPS_490 = 0x0040,
    SPS_920 = 0x0060,
    /** Default sample rate. */
    SPS_1600 = 0x0080,
    SPS_2400 = 0x00A0,
    SPS_3300 = 0x00C0
  };
  /** @enum CHANNEL
   *  Possible read channels both single ended and differential. */
  enum CHANNEL : unsigned short {
    DIFFERENTIAL_0_1 = 0x0000,
    DIFFERENTIAL_0_3 = 0x1000,
    DIFFERENTIAL_1_3 = 0x2000,
    DIFFERENTIAL_2_3 = 0x3000,
    SINGLE_0 = 0x4000,
    SINGLE_1 = 0x5000,
    SINGLE_2 = 0x6000,
    SINGLE_3 = 0x7000,
  };
  /** Establishes default settings and sets up the ADC on the given bus with the
   *  specified settings. See the \c I2CDevice class documentation for more
   *  details. */
  void setup(i2c_t3 *wire, uint8_t addr, unsigned int alert_pin, unsigned long timeout = DEV_I2C_TIMEOUT);
  /** Configures the alert pin to signal when a conversion is complete. If this
   *  is succesful, the device is enabled.
   *  @return True if the reset is succesful and false otherwise. */
  virtual bool reset() override;
  /** Sets the gain value of the ADS1015. */
  inline void set_gain(GAIN gain) { this->gain = gain; }
  /** Returns the current gain of the ADS1015.
   *  \returns current gain value. */
  inline GAIN get_gain() const { return this->gain; }
  /** Sets the sample rate of the ADS1015. */
  void set_sample_rate(SR sample_rate);
  /** Returns the current sample rate of the ADS1015.
   *  \returns current sample rate value. */
  inline SR get_sample_rate() const { return this->sample_rate; }
  /** Initializes a read on the specified ADC channel.
   *  \param[in] channel Channel the read is being taken on. */
  void start_read(CHANNEL channel);
  /** Completes an in progress read and stores the result in val.
   *  \param[out] val Location where the result is stored.
   *  \returns true if the read was successful and false otherwise. */
  bool end_read(int16_t &val);
  /** Equivalent to calling start_read and end_read.
   *  \param[in] channel Channel the read is being taken on.
   *  \param[out] val Location where the result is stored.
   *  \returns true if the read was succesful and false otherwise. */
  bool read(CHANNEL channel, int16_t &val);

 private:
  /** ADC conversion complete alert pin. */
  unsigned int alert_pin;
  /** Timestamp of the start of the most recent conversion. */
  unsigned long timestamp;
  /** Sample timeout delay. */
  unsigned int sample_delay;
  /** ADC samples per second. */
  SR sample_rate;
  /** ADC gain value. */
  GAIN gain;
};
}  // namespace dev
}  // namespace adcs

#endif
