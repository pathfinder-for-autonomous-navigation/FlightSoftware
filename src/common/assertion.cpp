#include "assertion.hpp"
#include "debug_console.hpp"

void pan_assert(bool condition, std::exception&& exp) {
    if (!(condition)) {
        #if defined(FUNCTIONAL_TEST)
            debug_console::printf(debug_severity::error, exp.what());
            assert(false);
        #elif defined(UNIT_TEST)
            throw exp;
        #endif
    }
}
