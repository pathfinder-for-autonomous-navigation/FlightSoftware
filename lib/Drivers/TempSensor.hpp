#ifndef TEMP_SENSOR_HPP_
#define TEMP_SENSOR_HPP_

#include "../Devices/Device.hpp"

namespace Devices {
/**
 * @brief Class for interfacing with a pressure sensor.
 */
class TempSensor : public Device {
   public:
    static constexpr unsigned char DEFAULT_PIN_OUTER_TANK = 21;
    static constexpr unsigned char DEFAULT_PIN_INNER_TANK = 22;

    /** @brief Construct a new Pressure Sensor object.
     * @param pin Pin # that temperature sensor is attached to.
     */
    TempSensor(const std::string &name, unsigned char pin);

    // Device functions
    bool setup() override;
    bool is_functional() override;
    void disable() override;
    void reset() override;

    float get();

   private:
    unsigned char pin_;
};
}  // namespace Devices

#endif