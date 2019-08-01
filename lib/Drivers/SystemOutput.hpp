#ifndef SYSTEM_OUTPUT_HPP_
#define SYSTEM_OUTPUT_HPP_

#include <Arduino.h>
#include <Device.hpp>

namespace Devices {
class SystemOutput : public Device {
   public:
    SystemOutput(const std::string &n, HardwareSerial &sp);

    bool setup() override;
    bool is_functional() override;
    void reset() override;
    void disable() override;  // Sets Piksi's power consumption to a minimum

    void send_impulse(double (&impulse)[3]);
    void send_propagated_position(double (&propagated_position)[3]);
    void send_propagated_velocity(double (&propagated_velocity)[3]);
    void send_propagated_other_position(double (&propagated_other_position)[3]);
    void send_propagated_other_velocity(double (&propagated_other_velocity)[3]);

   private:
    HardwareSerial &_serial_port;
};
}

#endif