#ifndef DEBUG_CONSOLE_HPP_
#define DEBUG_CONSOLE_HPP_

#include <set>
#include <ChRt.h>

/**
 * @brief Provides access to Serial via a convenient wrapper that plays
 * well with ChibiOS.
 * 
 */
class debug_console {
  private:
    std::set<thread_t*> _silenced_threads;
    systime_t _start_time;

    unsigned int _get_elapsed_time();
    bool _print_call_is_from_silenced_thread();
    void _print_json_msg(const char* msg);
  public:
    debug_console();

    /**
     * @brief Starts the debug console.
     */
    void begin();

    /**@brief Prevent a thread from writing to the debug console. **/
    void silence_thread(thread_t* thd);

    /**
     * @brief Prints a formatted string and prepends the process name at the beginning of the string.
     * The use of a formatted string allows for the easy printing of arbitrary data.
     * @param format The format string specifying how data should be represented.
     * @param ... One or more arguments containing the data to be printed.
     */
    void printf(const char* format, ...);

    /**
     * @brief Prints a string to console. Computer console automatically appends newline.
     * @param str The string to be printed. 
     */
    void println(const char* str);

    /** @brief Blinks an LED at a rate of 1 Hz. */
    void blink_led();
};


#endif