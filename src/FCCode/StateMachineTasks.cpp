#include "StateMachineTasks.hpp"

StateHandler::StateHandler(const std::string& name, StateFieldRegistry& r, bool only_once) : 
    StateFieldRegistryReader<unsigned int>(name, r),
    only_execute_once(only_once),
    has_executed(false) {}