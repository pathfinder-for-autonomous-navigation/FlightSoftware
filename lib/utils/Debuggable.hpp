#ifndef DEBUGGABLE_HPP_
#define DEBUGGABLE_HPP_

#include "debug_console.hpp"

class Debuggable {
  protected:
    debug_console& _dbg_console;
  public:
    Debuggable(debug_console& dbg);
};

#endif