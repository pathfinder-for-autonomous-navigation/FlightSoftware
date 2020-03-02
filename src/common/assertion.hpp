#ifndef assertion_hpp_
#define assertion_hpp_

#include <string>
#include <exception>
#include <type_traits>

/**
 * @brief Asserts a runtime error.
 * 
 * Assertion errors should not be used inside the main control loop; they should
 * only either be used inside initialization/constructor functions for Flight Software,
 * or anywhere inside ground software.
 * 
 * The assertion has differing behavior in differing environments:
 * - In flight, all assertions have no behavior.
 * - In HITL/HOOTL, the assertion causes a program crash and a printing of an error message
 *   to the debug console.
 * - In unit testing mode, the assertion throws an std::exception, which can be caught.
 *   This makes it easy to verify that constructors fail correctly if they are supplied
 *   invalid inputs.
 * 
 * @param condition The condition to test for. If false, the errors are printed.
 * @param exception The std::exception containing the error message.
 */
template<typename EXCEPTION>
void pan_assert(bool condition, const char* err) noexcept(false) {
    static_assert(std::is_base_of<std::logic_error, EXCEPTION>::value,
        "Must provide an logic_error exception type as the template argument.");

    if (!(condition)) {
        #if defined(UNIT_TEST)
            throw EXCEPTION(err);
        #elif defined(FUNCTIONAL_TEST)
            debug_console::printf(debug_severity::error, err);
            assert(false);
        #endif
    }
}

#endif
