#ifndef PRESSURE_SENSOR_HPP_
#define PRESSURE_SENSOR_HPP_

#include "../Devices/Device.hpp"

namespace Devices {
    /**
     * @brief Class for interfacing with a pressure sensor.
     */
    class PressureSensor : public Device {
      public:
        static constexpr unsigned char DEFAULT_LOW_PRESSURE_PIN = 20;
        static constexpr unsigned char DEFAULT_HIGH_PRESSURE_PIN = 23;

        /** @brief Construct a new Pressure Sensor object
         *  @param low_pressure_pin Pin # attached to the low-gain ADC to measure low pressures.
         *  @param high_pressure_pin Pin # attached to the high-gain ADC to measure high pressures.
         */
        PressureSensor(unsigned char low_pressure_pin, unsigned char high_pressure_pin);

        // Device functions
        bool setup() override;
        bool is_functional() override;
        void disable() override;
        void reset() override;
        std::string& name() const override;

        float get();
      private:
        unsigned char low_pressure_pin_, high_pressure_pin_;
    };
}

#endif