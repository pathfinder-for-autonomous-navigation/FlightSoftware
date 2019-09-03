#ifndef DEBUG_CONSOLE_HPP_
#define DEBUG_CONSOLE_HPP_

#include "StateField.hpp"
#include "StateFieldRegistry.hpp"
#include <map>
#include <set>
#include <ChRt.h>

/**
 * @brief Provides access to Serial via a convenient wrapper that plays
 * well with ChibiOS.
 *
 */
class debug_console {
   public:
    // Severity levels based off of
    // https://support.solarwinds.com/SuccessCenter/s/article/Syslog-Severity-levels
    // See the article for an explanation of when to use which severity level.
    enum severity { DEBUG, INFO, NOTICE, WARNING, ERROR, CRITICAL, ALERT, EMERGENCY };
    static std::map<severity, const char *> severity_strs;

    /**
     * @brief Starts the debug console.
     */
    static bool init();

    /**
     * @brief Prints a formatted string and prepends the process name at the
     * beginning of the string. The use of a formatted string allows for the easy
     * printing of arbitrary data.
     * @param format The format string specifying how data should be represented.
     * @param ... One or more arguments containing the data to be printed.
     */
    static void printf(severity s, const char *format, ...);

    /**
     * @brief Prints a string to console. Computer console automatically appends
     * newline.
     * @param str The string to be printed.
     */
    static void println(severity s, const char *str);

    /**
     * @brief Blinks an LED at a rate of 1 Hz.
     */
    static void blink_led();

    /**
     * @brief Reads in from the serial buffer to process incoming commands from a
     * computer to read/write to state fields.
     */
    static void process_commands(const StateFieldRegistry& registry);

    /**
     * @brief Helper method to write state fields to the console. State fields might
     * be written to the console if they were requested by the computer or if they're
     * "always-on" debugging telemetry.
     * 
     * @param field 
     */
    static void print_state_field(const SerializableStateFieldBase& field);

   protected:
    // Prevent construction.
    debug_console() {}

    static systime_t _start_time;
    static bool is_init;

    static unsigned int _get_elapsed_time();
    static void _print_json_msg(severity s, const char *msg);
};

typedef debug_console::severity debug_severity;

class Debuggable : public debug_console {
   public:
    Debuggable() : debug_console() {}
};

/**
 * Macros used for testing initialization. These statements should NOT be used
 * in production environments and should ONLY used within initialization
 * contexts. The class using these macros must inherit from Debuggable.
 */

#define ReturnIfMsg(condition, msg, retval)                                  \
    if (condition) {                                                         \
        printf(debug_severity::ERROR, "%s %s:%s.", msg, __FILE__, __LINE__); \
        return retval;                                                       \
    }

#define ReturnIfNotMsg(condition, msg, retval) ReturnIfMsg(!(condition), msg, retval)

#define ReturnIf(condition, retval) ReturnIfMsg(condition, "Error occurred at ", retval)

#define ReturnIfNot(condition, retval) ReturnIfNotMsg(!(condition), "Error occurred at ", retval)

#define ReturnIfInitFail(initialization, retval) \
    ReturnIfNotMsg(initialization, "Initialization failed at", retval)

#endif