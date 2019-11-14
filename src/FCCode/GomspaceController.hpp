#pragma once

#ifndef GOMSPACE_CONTROLLER_HPP_
#define GOMSPACE_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <TimedControlTask.hpp>
#include "../../lib/Drivers/Gomspace.hpp"

class GomspaceController : public TimedControlTask<void> {
   public:
    
    /**
     * @brief Construct a new Gomspace Controller object
     * 
     * @param registry 
     * @param offset
     * @param gs 
     */
    GomspaceController(StateFieldRegistry& registry, unsigned int offset,
        Devices::Gomspace &gs);

    /**
     * @brief 
     */
    void execute() override;

   protected:
    Devices::Gomspace &gs;

    
};

#endif
