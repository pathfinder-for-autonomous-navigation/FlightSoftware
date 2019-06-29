#ifndef DEBUG_CONSOLE_HPP_
#define DEBUG_CONSOLE_HPP_

#include <set>
#include <map>
#include <InitializationRequired.hpp>

/**
 * @brief Provides access to Serial via a convenient wrapper that plays
 * well with ChibiOS.
 * 
 */
class debug_console : public InitializationRequired {
  public:
    // Severity levels based off of https://support.solarwinds.com/SuccessCenter/s/article/Syslog-Severity-levels
    // See the article for an explanation of when to use which severity level.
    enum severity {
      DEBUG,
      INFO,
      NOTICE,
      WARNING,
      ERROR,
      CRITICAL,
      ALERT,
      EMERGENCY
    };
    static std::map<severity, const char *> severity_strs;

    debug_console();

    /**
     * @brief Starts the debug console.
     */
    void init();

    /**@brief Prevent a thread from writing to the debug console. **/
    void silence_thread(thread_t* thd);

    /**
     * @brief Prints a formatted string and prepends the process name at the beginning of the string.
     * The use of a formatted string allows for the easy printing of arbitrary data.
     * @param format The format string specifying how data should be represented.
     * @param ... One or more arguments containing the data to be printed.
     */
    void printf(severity s, const char* format, ...);

    /**
     * @brief Prints a string to console. Computer console automatically appends newline.
     * @param str The string to be printed. 
     */
    void println(severity s, const char* str);

    /** 
     * @brief Blinks an LED at a rate of 1 Hz. 
     */
    void blink_led();
  private:
    mutex_t debug_console_lock;
    std::set<thread_t*> _silenced_threads;
    systime_t _start_time;

    unsigned int _get_elapsed_time();
    bool _print_call_is_from_silenced_thread();
    void _print_json_msg(severity s, const char* msg);
};

typedef debug_console::severity debug_severity;

// TODO add include guards to prevent use of this function during flight environments. This function
// should be used in initialization tests only.
//
#define abort_if_init_fail(initialization, console) {                                             \
    console.printf(debug_severity::ERROR, "Initialization failed at %s:%s.", __FILE__, __LINE__); \
    return false;                                                                                 \
  }

#endif