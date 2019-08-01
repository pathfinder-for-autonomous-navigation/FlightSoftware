#ifndef DEBUG_CONSOLE_HPP_
#define DEBUG_CONSOLE_HPP_

#include "ChRt.h"
#include <InitializationRequired.hpp>
#include <map>
#include <set>

/**
 * @brief Provides access to Serial via a convenient wrapper that plays
 * well with ChibiOS.
 *
 */
class debug_console : public InitializationRequired {
public:
  // Severity levels based off of
  // https://support.solarwinds.com/SuccessCenter/s/article/Syslog-Severity-levels
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

  /**
   * Factory for debug console creation that ensures that
   * the console can only be created once.
   */
  static debug_console *create() {
    static debug_console dbg;
    return &dbg;
  }

  /**
   * @brief Starts the debug console.
   */
  bool init();

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

protected:
  // Singleton, so that multiple debug consoles cannot be created.
  debug_console();
  debug_console(const debug_console &);
  debug_console &operator=(const debug_console &);

  systime_t _start_time;

  unsigned int _get_elapsed_time();
  void _print_json_msg(severity s, const char *msg);
};

typedef debug_console::severity debug_severity;

class Debuggable : public debug_console {
public:
  Debuggable() : debug_console() {}
};

/**
 * Macros used for BOOTL cases.
 */

#define AbortIfMsg(initialization, msg, retval)                                \
  if (initialization) {                                                        \
    printf(debug_severity::ERROR, "%s %s:%s.", msg, __FILE__, __LINE__);       \
    return retval;                                                             \
  }

#define AbortIf(initialization, retval)                                        \
  AbortIfMsg(initialization, "Error occurred at ", retval)

#define AbortIfNotMsg(initialization, msg, retval)                             \
  AbortIfMsg(!(initialization), msg, retval)

#define AbortIfNot(initialization, retval)                                     \
  AbortIfNotMsg(initialization, "Error occurred at ", retval)

#define AbortIfInitFail(initialization, retval)                                \
  AbortIfMsg(initialization, "Initialization failed at", retval)

#endif