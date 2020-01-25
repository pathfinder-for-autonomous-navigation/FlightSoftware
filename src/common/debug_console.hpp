#ifndef DEBUG_CONSOLE_HPP_
#define DEBUG_CONSOLE_HPP_

#include <map>
#include <cassert>
#include "StateField.hpp"
#include "StateFieldRegistry.hpp"

#ifdef DESKTOP
    #include <chrono>
    #include <memory>
    #include <thread>
    #include <concurrentqueue.h>
#else
    #include <Arduino.h>
#endif

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
    static void printf(severity s, const char *format, ...);

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
#ifndef DESKTOP
    static unsigned int _start_time;
#else
    static std::chrono::steady_clock::time_point _start_time;
#endif

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
    static unsigned int _get_elapsed_time();

    /**
     * @brief Prints a message in JSON format to the debug console.
     *
     * @param s
     * @param msg
     */
    static void _print_json_msg(severity s, const char *msg);

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

#ifdef DESKTOP
    /**
     * @brief Thread-safe queue for inputs found by reader thread.
     */
    moodycamel::ConcurrentQueue<std::string> unprocessed_inputs;

    /**
     * @brief True if debug console is currently running.
     */
    bool running = false;

    /**
     * @brief Collects input from stdin and passes it to process_commands via
     * a thread-safe queue.
     */
    void _reader();

    std::shared_ptr<std::thread> reader_thd;
#endif

    /**
     * @brief Destroy the debug console object
     */
    ~debug_console();
};

/**
 * @brief Useful type definition to reduce the length of debugging print statements.
 */
typedef debug_console::severity debug_severity;

/**
 * @brief Asserts an error. This aborts the program if running on a computer and
 * does not if running on a Teensy. It prints a helpful error message in any case.
 * 
 * @param condition The condition to test for. If false, the errors are printed.
 * @param format Format string for error.
 * @param ... Format parameters.
 */
#define assert_msg(condition, format, ...) \
    assert(condition); \
    debug_console::printf(debug_severity::error, format, __VA_ARGS__); \
}

#endif
