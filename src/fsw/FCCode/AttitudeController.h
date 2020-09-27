//
// Created by athena on 9/27/20.
//

#ifndef FLIGHTSOFTWARE_ATTITUDECONTROLLER_H
#define FLIGHTSOFTWARE_ATTITUDECONTROLLER_H

#include "TimedControlTask.hpp"

class AttitudeController : public TimedControlTask<void>
{
public:
    AttitudeController(StateFieldRegistry& registry, unsigned int offset);

    void execute() override;

protected:

};


#endif //FLIGHTSOFTWARE_ATTITUDECONTROLLER_H
