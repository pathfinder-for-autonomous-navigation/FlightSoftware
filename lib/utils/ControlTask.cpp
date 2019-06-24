#include "ControlTask.hpp"

StateHandler::StateHandler(const std::string& name, debug_console& dbg, bool only_once) : 
    ControlTask<unsigned int>(name, dbg),
    only_execute_once(only_once),
    has_executed(false) {}