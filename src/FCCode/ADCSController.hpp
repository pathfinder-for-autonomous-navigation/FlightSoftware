#ifndef ADCS_BOX_MONITOR_HPP_
#define ADCS_BOX_MONITOR_HPP_

#include <ADCS.hpp>
#include "TimedControlTask.hpp"

/**
* @brief Gets inputs from the ADCS box and dumps them into the state
* fields listed below.
*/
class ADCSController : public TimedControlTask<void>
{
public:
    /**
     * @brief Construct a new ADCSController control task
     * 
     * @param registry input StateField registry
     * @param offset control task offset
     * @param _adcs the input adcs system
     */
    ADCSController(StateFieldRegistry &registry, unsigned int offset, Devices::ADCS &_adcs);

    /** ADCS Driver. **/
    Devices::ADCS& adcs_system;

    /**
    * @brief Gets inputs from the ADCS box and dumps them into the state
    * fields listed below.
    */
    void execute() override;

protected:
   
};

#endif
