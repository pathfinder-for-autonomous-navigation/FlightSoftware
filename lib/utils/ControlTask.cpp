#include "ControlTask.hpp"

Task::Task(const std::string& name) : Nameable(name) {}

StateHandler::StateHandler(std::string& name, 
                           debug_console& dbg) :
    ControlTask<unsigned int>(name, dbg) {
}

TransitionHandler::TransitionHandler(std::string& name, 
                                     debug_console& dbg) :
    ControlTask<void>(name, dbg) { }