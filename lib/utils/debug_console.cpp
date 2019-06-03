#include "debug_console.hpp"
#include <Arduino.h>
#include <ChRt.h>

debug_console::debug_console() : _silenced_threads() {}

unsigned int debug_console::_get_elapsed_time() {
    systime_t current_time = chVTGetSystemTimeX();
    if (!Serial) _start_time = current_time; // Reset time if Serial is unconnected
    unsigned int elapsed_time = ST2MS(current_time - _start_time);
    return elapsed_time;
}

bool debug_console::_print_call_is_from_silenced_thread() {
    thread_t* cur_thd = chThdGetSelfX();
    if (_silenced_threads.find(cur_thd) != _silenced_threads.end()) return true;
    return false;
}

void debug_console::_print_json_msg(const char* msg) {
    chSysLock();
    Serial.printf("{\"t\":%d,"
                   "\"thd\":\"%s\","
                   "\"msg\":\"%s\""
                  "}\n",
        _get_elapsed_time(), chThdGetSelfX()->name, msg);
    chSysUnlock();
}

void debug_console::begin() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);

    Serial.println("Waiting for serial console.");
    while (!Serial);
    _start_time = chVTGetSystemTimeX();
}

void debug_console::silence_thread(thread_t* thd) {
    _silenced_threads.emplace(thd);
}

void debug_console::printf(const char* format, ...) {
    if (_print_call_is_from_silenced_thread()) return;

    char buf[1024];
    va_list args;
    va_start( args, format );
    vsnprintf(buf, sizeof(buf), format, args);
    _print_json_msg(buf);
    va_end( args );
}

void debug_console::println(const char* str) {
    if (_print_call_is_from_silenced_thread()) return;
    _print_json_msg(str);
}

void debug_console::blink_led() {
    digitalWrite(13, HIGH);
    chThdSleepMilliseconds(500);
    digitalWrite(13, LOW);
    chThdSleepMilliseconds(500);
}