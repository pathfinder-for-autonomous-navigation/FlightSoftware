#ifndef ADCS_BOX_CONTROLLER_HPP_
#define ADCS_BOX_CONTROLLER_HPP_

#include <ADCS.hpp>
#include "TimedControlTask.hpp"

/**
 * @brief Takes input command statefields and commands the ADCS Box.
 * 
 * Note this CT doesn't do any computing, just actuation
 */
class ADCSBoxController : public TimedControlTask<void>
{
public:
    /**
     * @brief Construct a new ADCSBoxController control task
     * 
     * @param registry input StateField registry
     * @param offset control task offset
     * @param _adcs the input adcs system
     */
    ADCSBoxController(StateFieldRegistry &registry, unsigned int offset, Devices::ADCS &_adcs);

    /** ADCS Driver. **/
    Devices::ADCS& adcs_system;

    /**
    * @brief Gets inputs from the ADCS box and dumps them into the state
    * fields listed below.
    */
    void execute() override;

protected:
    /**
    * @brief Commands to actuate on the ADCS Box
    */
    ReadableStateField<bool>* adcs_mode_fp;
};

#endif
