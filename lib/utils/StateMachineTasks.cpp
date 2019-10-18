#include "StateMachineTasks.hpp"

StateHandler::StateHandler(const std::string &name, StateFieldRegistry &r, bool only_once)
    : ControlTask<unsigned int>(name, r), only_execute_once(only_once) {}