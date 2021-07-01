#ifndef DEBUG_CONSOLE_HPP_
#define DEBUG_CONSOLE_HPP_

#include <cassert>
#include "StateField.hpp"
#include "StateFieldRegistry.hpp"

class debug_console {
  public:
    // Severity levels based off of
    // https://support.solarwinds.com/SuccessCenter/s/article/Syslog-Severity-levels
    // See the article for an explanation of when to use which severity level.
    enum class severity_t : unsigned char {
        debug, info, notice, warning, error, critical, alert, emergency
    };

    enum class state_field_error_t : unsigned char {
        invalid_field_name, field_is_only_readable, missing_mode,
        invalid_mode_not_char, invalid_mode, missing_field_val,
        invalid_field_val
    };

    enum class state_cmd_mode_t : unsigned char {
        unspecified_mode, read_mode, write_mode
    };

    /** @brief Initializes the debug console.
     *
     *  This function must be called before any other method provided will
     *  function properly.
     */
    static void open();

    /** @brief Closes the debug console.
     *
     *  This function should be called on program termination. This doesn't
     *  matter for HITL but is responsible for cleanly closing a thread in
     *  HOOTL.
     */
    static void close();

    /** @brief Prints a formatted message over the debug console.
     *
     *  @param severity Message severity.
     *  @param fmt      Message format.
     *  @param ...      Formatting arguments.
     */
    static void printf(severity_t severity, char const *fmt, ...);

    /** @brief Prints a formatted message over the debug console.
     *
     *  @param fmt      Message format.
     *  @param ...      Formatting arguments.
     *
     *  The message severity defaults to `info`.
     */
    static void printf(char const *fmt, ...);

    /** @brief Prints a message over the debug console followed by a newline.
     *
     *  @param severity Message severity.
     *  @param msg      Message.
     */
    static void println(severity_t severity, char const *msg);

    /** @brief Prints a message over the debug console followed by a newline.
     *
     *  @param msg Message.
     *
     *  The message severity defaults to `info`.
     */
    static void println(char const *msg);

    /**
     * @brief Blinks an LED at a rate of 1 Hz.
     */
    static void blink_led();

    /**
     * @brief Reads in from the serial buffer to process incoming commands from a
     * computer to read/write to state fields.
     */
#ifdef DESKTOP
    static void process_commands(const StateFieldRegistry &registry, bool blocking);
#else
    static void process_commands(const StateFieldRegistry &registry);
#endif

    /**
     * @brief Helper method to write state fields to the console. State fields might
     * be written to the console if they were requested by the computer or if they're
     * "always-on" debugging telemetry.
     *
     * @param field
     */
    static void print_state_field(const SerializableStateFieldBase &field);

  protected:
    /** @return Returns the elapsed time relative to system time in milliseconds.
     */
    static unsigned int _get_elapsed_time();

    /** @brief Prints a message in JSON format to the debug console.
     *
     *  @param severity Message severity.
     *  @param msg      Message string.
     */
    static void _print_json_msg(severity_t severity, const char* msg);

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
    static void _print_error_state_field(char const *field_name,
            state_cmd_mode_t mode, state_field_error_t error_code);
};

/**
 * @brief Useful type definition to reduce the length of debugging print statements.
 */
using debug_severity = typename debug_console::severity_t;

/**
 * @brief Asserts an error. This aborts the program if running on a computer and
 * does not if running on a Teensy. It prints a helpful error message in any case.
 * 
 * @param condition The condition to test for. If false, the errors are printed.
 * @param format Format string for error.
 * @param ... Format parameters.
 */
#define assert_msg(condition, format, ...) \
    do { \
        assert(condition); \
        debug_console::printf(debug_severity::error, format, __VA_ARGS__); \
    } while (0);

#endif
