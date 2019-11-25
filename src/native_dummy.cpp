#include "FCCode/DummyMainControlLoop.hpp"
#include <StateFieldRegistry.hpp>

#ifndef UNIT_TEST
int main() {
  StateFieldRegistry registry;
  DummyMainControlLoop fcp(registry);
  fcp.init();

  while (true) {
    fcp.execute();
  }
  return 0;
}
#endif
