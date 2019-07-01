#ifndef DEBUGGABLE_HPP_
#define DEBUGGABLE_HPP_

#include "debug_console.hpp"

/**
 * @brief Interface for a class that wants to print debug messages to console.
 * 
 */
class Debuggable {
  protected:
    debug_console* _dbg_console;
  public:
    /**
     * @brief Construct a new Debuggable object
     * 
     * @param dbg 
     */
    Debuggable();
};

#endif