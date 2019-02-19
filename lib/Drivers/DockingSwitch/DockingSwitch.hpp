#ifndef DOCKING_SWITCH_HPP_
#define DOCKING_SWITCH_HPP_

#include "../Devices/Device.hpp"

namespace Devices {
    /** 
     * @brief Reports whether or not the docking switch is pressed.
     * **/
    class DockingSwitch : public Device {
      public:
        //! Enum as a descriptor for whether the docking switch is pressed or not
        enum DOCKINGSWITCH_STATE {
          UNPRESSED = 0,
          PRESSED = 1
        };

        //! Default enable pin for DCDC.
        static constexpr unsigned char DEFAULT_SWITCH_PIN = 25; // TODO fix

        /** @brief Default constructor. Loads a set of hardcoded pins into the valve table.**/
        DockingSwitch(unsigned char en);

        bool setup() override;
        bool is_functional() override;
        void disable() override;
        void reset() override;
        void single_comp_test() override;
        std::string& name() const override;

        /** @brief Returns whether or not the docking switch is pressed. **/
        bool pressed();
      private:
        //! Pin to which the docking switch is connected.
        unsigned char switch_pin_; 
    };
}

#endif