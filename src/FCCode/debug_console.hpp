#ifndef DEBUG_CONSOLE_HPP_
#define DEBUG_CONSOLE_HPP_

#include <ChRt.h>
#include <map>
#include "StateField.hpp"
#include "StateFieldRegistry.hpp"

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
    enum severity { debug, info, notice, warning, error, critical, alert, emergency };
    static std::map<severity, const char *> severity_strs;

    enum state_field_error {
        invalid_field_name,
        field_is_only_readable,
        missing_mode,
        invalid_mode_not_char,
        invalid_mode,
        missing_field_val,
        invalid_field_val
    };
    static std::map<state_field_error, const char *> state_field_error_strs;
    enum state_cmd_mode { unspecified_mode, read_mode, write_mode };
    static std::map<state_cmd_mode, const char *> state_cmd_mode_strs;

    debug_console();

    /**
     * @brief Starts the debug console.
     */
    void init();

    /**
     * @brief Prints a formatted string and prepends the process name at the
     * beginning of the string. The use of a formatted string allows for the easy
     * printing of arbitrary data.
     * @param format The format string specifying how data should be represented.
     * @param ... One or more arguments containing the data to be printed.
     */
    void printf(severity s, const char *format, ...);

    /**
     * @brief Prints a string to console. Computer console automatically appends
     * newline.
     * @param str The string to be printed.
     */
    void println(severity s, const char *str);

    /**
     * @brief Blinks an LED at a rate of 1 Hz.
     */
    void blink_led();

    /**
     * @brief Reads in from the serial buffer to process incoming commands from a
     * computer to read/write to state fields.
     */
    void process_commands(const StateFieldRegistry &registry);

    /**
     * @brief Helper method to write state fields to the console. State fields might
     * be written to the console if they were requested by the computer or if they're
     * "always-on" debugging telemetry.
     *
     * @param field
     */
    void print_state_field(const SerializableStateFieldBase &field);

   protected:
    /**
     * @brief The system time at which the debug connection with the computer was initiated,
     * relative to ChibiOS's initialization time.
     */
    systime_t _start_time;

    /**
     * @brief Checks whether or not the debug console has been initialized. This is a static
     * variable so that the debug console is not forcibly initialized several times (which can
     * happen if multiple ControlTasks initialize the console.)
     */
    static bool is_initialized;

    /**
     * @brief Returns the elapsed time relative to system time.
     *
     * @return unsigned _get_elapsed_time
     */
    unsigned int _get_elapsed_time();

    /**
     * @brief Prints a message in JSON format to the debug console.
     *
     * @param s
     * @param msg
     */
    void _print_json_msg(severity s, const char *msg);

    /**
     * @brief If a read or write command was issued by a simulation computer to this Flight
     * Computer, and the command was malformed or unsuccessful, this function prints an
     * explanation for why the command was unsuccessful.
     *
     * @param field_name The field that the computer tried to read or write.
     * @param mode Specifies mode that was used in accessing the state field (either "read",
     * "write", or "unspecified"). This field is used by the console as part of its error message.
     * @param error The error associated with the computer's request.
     */
    void _print_error_state_field(const char *field_name, const state_cmd_mode mode,
                                  const state_field_error error);
};

/**
 * @brief Useful type definition to reduce the length of debugging print statements.
 */
typedef debug_console::severity debug_severity;

/**
 * Macros used for testing initialization. These statements should NOT be used
 * in production environments and should ONLY used within initialization
 * contexts. A class using these macros must inherit from debug_console.
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