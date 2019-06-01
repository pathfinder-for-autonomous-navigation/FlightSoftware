#include "debug_console.hpp"
#include <Arduino.h>
#include <ChRt.h>

debug_console::debug_console() : _silenced_threads() {}

unsigned int debug_console::_get_elapsed_time() {
    return ST2MS(chVTTimeElapsedSinceX(_start_time));
}

void debug_console::begin() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);

    _start_time = chVTGetSystemTimeX();

    while (!Serial) {}
}

void debug_console::silence_thread(thread_t* thd) {
    _silenced_threads.emplace(thd);
}

void debug_console::printf(const char* format, ...) {
    thread_t* cur_thd = chThdGetSelfX();
    if (_silenced_threads.find(cur_thd) != _silenced_threads.end()) return;

    char buf[1024];
    va_list args;
    va_start( args, format );
    vsnprintf(buf, sizeof(buf), format, args);
    chSysLock();
        Serial.printf("{\"t\":%d,\"thd\":\"%s\",\"msg\":\"%s\"}\n", 
            _get_elapsed_time(), chThdGetSelfX()->name, buf);
        Serial.flush();
    chSysUnlock();
    va_end( args );
}

void debug_console::println(const char* str) {
    thread_t* cur_thd = chThdGetSelfX();
    if (_silenced_threads.find(cur_thd) != _silenced_threads.end()) return;

    chSysLock();
        Serial.printf("{\"t\":%d,\"thd\":\"%s\",\"msg\":\"%s\"}\n",
            _get_elapsed_time(), chThdGetSelfX()->name, str);
    chSysUnlock();
}

void debug_console::blink_led() {
    digitalWrite(13, HIGH);
    chThdSleepMilliseconds(500);
    digitalWrite(13, LOW);
    chThdSleepMilliseconds(500);
}