#include "debug_console.hpp"
#include <Arduino.h>
#include <ChRt.h>

std::map<debug_severity, const char *> debug_console::severity_strs {
    {debug_severity::DEBUG, "DEBUG"},
    {debug_severity::INFO, "INFO"},
    {debug_severity::NOTICE, "NOTICE"},
    {debug_severity::WARNING, "WARNING"},
    {debug_severity::ERROR, "ERROR"},
    {debug_severity::CRITICAL, "CRITICAL"},
    {debug_severity::ALERT, "ALERT"},
    {debug_severity::EMERGENCY, "EMERGENCY"},
};

debug_console::debug_console() : InitializationRequired(), _silenced_threads() {}

unsigned int debug_console::_get_elapsed_time() {
    systime_t current_time = chVTGetSystemTimeX();
    if (!Serial) {
        /** Reset the start time if Serial is unconnected. We
         * do this so that the logging utility on the computer
         * can always produce a correct timestamp.
        */
        _start_time = current_time;
    }
    unsigned int elapsed_time = ST2MS(current_time - _start_time);
    return elapsed_time;
}

bool debug_console::_print_call_is_from_silenced_thread() {
    thread_t* cur_thd = chThdGetSelfX();
    if (_silenced_threads.find(cur_thd) != _silenced_threads.end()) return true;
    return false;
}

void debug_console::_print_json_msg(severity s, const char* msg) {
    // TODO check if we're within a thread
    chMtxLock(&debug_console_lock);

    Serial.printf("{\"t\":%d,"
                   "\"svrty\":\"%s\","
                   "\"thd\":\"%s\","
                   "\"msg\":\"%s\""
                  "}\n",
        _get_elapsed_time(),
        severity_strs.at(s),
        chThdGetSelfX()->name,
        msg);
    
    // TODO check if we're within a thread
    chMtxUnlock(&debug_console_lock);
}

bool debug_console::init() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);

    Serial.println("Waiting for serial console.");
    while (!Serial);
    _start_time = chVTGetSystemTimeX();
    
    // TODO check if we're within a thread
    chMtxObjectInit(&debug_console_lock);

    return InitializationRequired::init();
}

void debug_console::silence_thread(thread_t* thd) {
    _silenced_threads.emplace(thd);
}

void debug_console::printf(severity s, const char* format, ...) {
    if (_print_call_is_from_silenced_thread()) return;
    char buf[1024];
    va_list args;
    va_start( args, format );
    vsnprintf(buf, sizeof(buf), format, args);
    _print_json_msg(s, buf);
    va_end( args );
}

void debug_console::println(severity s, const char* str) {
    if (_print_call_is_from_silenced_thread()) return;
    _print_json_msg(s, str);
}

void debug_console::blink_led() {
    digitalWrite(13, HIGH);
    chThdSleepMilliseconds(500);
    digitalWrite(13, LOW);
    chThdSleepMilliseconds(500);
}