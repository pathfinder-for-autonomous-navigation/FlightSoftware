#ifndef DOCKING_MOTOR_HPP_
#define DOCKING_MOTOR_HPP_

#include "../Devices/Device.hpp"

namespace Devices {
    /** 
     * @brief Logical interface to docking motor.
     * **/
    class DockingMotor : public Device {
      public:
        //! Enum as a descriptor for whether the docking motor has been activated or not
        enum DOCKINGMOTOR_STATE {
          ON = 0,
          OFF = 1
        };

        //! Default pin for docking motor I1.
        static constexpr unsigned char DEFAULT_MOTOR_I1_PIN = 14;
        //! Default pin for docking motor I2.
        static constexpr unsigned char DEFAULT_MOTOR_I2_PIN = 15;
        //! Default pin for docking motor direction pin.
        static constexpr unsigned char DEFAULT_MOTOR_DIRECTION_PIN = 16;
        //! Default pin for docking motor sleep pin.
        static constexpr unsigned char DEFAULT_MOTOR_SLEEP_PIN = 17;
        //! Default pin for docking motor reset pin.
        static constexpr unsigned char DEFAULT_MOTOR_RESET_PIN = 17;
        //! Default pin for docking motor step.
        static constexpr unsigned char DEFAULT_MOTOR_STEP_PIN = 39;

        /**
         * @brief Construct a new Docking Motor object.
         * 
         * @param i1 I1 pin.
         * @param i2 I2 pin.
         * @param dir Motor direction pin.
         * @param sleep Sleep pin.
         * @param reset Reset pin.
         * @param step Stepper pin.
         */
        DockingMotor(unsigned char i1, 
                     unsigned char i2, 
                     unsigned char dir, 
                     unsigned char sleep, 
                     unsigned char reset, 
                     unsigned char step);

        bool setup() override;
        bool is_functional() override;
        void disable() override;
        void reset() override;
        void single_comp_test() override;
        std::string& name() const override;

        /** @brief Turn the docking motor. **/
        void toggle();
      private:
        //! I1 Pin
        unsigned char i1_pin_;
        //! I1 Pin
        unsigned char i2_pin_;
        //! Pin for selecting docking motor direction.
        unsigned char direction_pin_;
        //! Pin for putting docking motor controller to sleep.
        unsigned char sleep_pin_;
        //! Pin for resetting docking motor contorller.
        unsigned char reset_pin_;
        //! Pin for stepping docking motor.
        unsigned char stepper_pin_;
        
    };
}

#endif