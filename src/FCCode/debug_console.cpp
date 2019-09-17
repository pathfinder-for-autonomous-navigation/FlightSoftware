#include "debug_console.hpp"
#include <ArduinoJson.h>
#include <array>
#include <cstdarg>


#ifndef DESKTOP
#include <Arduino.h>
#else
#include <time.h>
#include <iostream>
#endif

std::map<debug_severity, const char*> debug_console::severity_strs{
    {debug_severity::debug, "DEBUG"},   {debug_severity::info, "INFO"},
    {debug_severity::notice, "NOTICE"}, {debug_severity::warning, "WARNING"},
    {debug_severity::error, "ERROR"},   {debug_severity::critical, "CRITICAL"},
    {debug_severity::alert, "ALERT"},   {debug_severity::emergency, "EMERGENCY"},
};

std::map<debug_console::state_field_error, const char*> debug_console::state_field_error_strs{
    {state_field_error::invalid_field_name, "invalid field name"},
    {state_field_error::field_is_only_readable, "field is read-only"},
    {state_field_error::missing_mode, "missing mode specification"},
    {state_field_error::invalid_mode_not_char, "mode value is not a character"},
    {state_field_error::invalid_mode, "mode value is not 'r' or 'w'"},
    {state_field_error::missing_field_val, "missing value of field to be written"},
    {state_field_error::invalid_field_val, "field value was invalid"}};

std::map<debug_console::state_cmd_mode, const char*> debug_console::state_cmd_mode_strs{
    {state_cmd_mode::unspecified_mode, "perform unspecified operation with"},
    {state_cmd_mode::read_mode, "read"},
    {state_cmd_mode::write_mode, "write"},
};

bool debug_console::is_initialized = false;
#ifndef DESKTOP
systime_t debug_console::_start_time = 0;
#else
unsigned int debug_console::_start_time = 0;
#endif

debug_console::debug_console() {}

unsigned int debug_console::_get_elapsed_time() {
#ifdef DESKTOP
    time_t start = time(0);
    double ms_since_start = 1000 * difftime(time(0), start);
    return static_cast<unsigned int>(ms_since_start);
#else
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
#endif
}

void debug_console::_print_json_msg(severity s, const char* msg) {
    StaticJsonDocument<100> doc;
    doc["t"] = _get_elapsed_time();
    doc["svrty"] = severity_strs.at(s);
    doc["msg"] = msg;

#ifdef DESKTOP
    serializeJson(doc, std::cout);
    std::cout << std::endl;
#else
    serializeJson(doc, Serial);
    Serial.println();
#endif
}

void debug_console::init() {
    if (!is_initialized) {
#ifndef DESKTOP
        Serial.begin(115200);
        pinMode(13, OUTPUT);

        Serial.println("Waiting for serial console.");
        while (!Serial)
            ;
        _start_time = chVTGetSystemTimeX();
#endif

        is_initialized = true;
    }
}

void debug_console::printf(severity s, const char* format, ...) {
    if (!is_initialized) return;
    char buf[100];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    _print_json_msg(s, buf);
    va_end(args);
}

void debug_console::println(severity s, const char* str) {
    if (!is_initialized) return;
    _print_json_msg(s, str);
}

void debug_console::blink_led() {
    if (!is_initialized) return;
#ifndef DESKTOP
    digitalWrite(13, HIGH);
    chThdSleepMilliseconds(500);
    digitalWrite(13, LOW);
    chThdSleepMilliseconds(500);
#endif
}

void debug_console::print_state_field(const SerializableStateFieldBase& field) {
    StaticJsonDocument<100> doc;
    doc["t"] = _get_elapsed_time();
    doc["field"] = field.name().c_str();
    doc["val"] = field.print();
#ifdef DESKTOP
    serializeJson(doc, std::cout);
    std::cout << std::endl;
#else
    serializeJson(doc, Serial);
    Serial.println();
#endif
}

void debug_console::_print_error_state_field(const char* field_name,
                                             const debug_console::state_cmd_mode mode,
                                             const debug_console::state_field_error error_code) {
    StaticJsonDocument<100> doc;
    doc["t"] = _get_elapsed_time();
    doc["field"] = field_name;
    doc["mode"] = state_cmd_mode_strs.at(mode);
    doc["err"] = state_field_error_strs.at(error_code);
#ifdef DESKTOP
    serializeJson(doc, std::cout);
    std::cout << std::endl;
#else
    serializeJson(doc, Serial);
    Serial.println();
#endif
}

void debug_console::process_commands(const StateFieldRegistry& registry) {
    constexpr size_t SERIAL_BUF_SIZE = 64;
    char buf[SERIAL_BUF_SIZE] = {0};

#ifdef DESKTOP
    char c;
    for (unsigned int i = 0; i < SERIAL_BUF_SIZE && std::cin.get(c); ++i) {
        buf[i] = c;
    }
#else
    for (size_t i = 0; i < SERIAL_BUF_SIZE && Serial.available(); i++) {
        buf[i] = Serial.read();
    }
#endif

    constexpr size_t MAX_NUM_JSON_MSGS = 5;

    // Get all chunks of the buffer that are complete JSON messages. Read at
    // most five messages from the buffer. (It's unlikely that more than 5 messages
    // can fit within a 64 byte buffer)
    size_t json_msg_starts[MAX_NUM_JSON_MSGS] = {0};
    size_t num_json_msgs_found = 0;
    bool inside_json_msg = false;
    for (size_t i = 0; i < SERIAL_BUF_SIZE && num_json_msgs_found < MAX_NUM_JSON_MSGS; i++) {
        if (buf[i] == '{') {
            inside_json_msg = true;
            json_msg_starts[num_json_msgs_found] = i;
        } else if (inside_json_msg && buf[i] == '}') {
            inside_json_msg = false;
            num_json_msgs_found++;
            // Replace newline after the JSON object with a null character, so
            // that the JSON deserializer can recognize it as the end of a JSON
            // object
            if (i + 1 < SERIAL_BUF_SIZE) {
                buf[i + 1] = '\x00';
            }
        }
    }

    // Deserialize all found JSON messages and check their validity
    std::array<StaticJsonDocument<50>, MAX_NUM_JSON_MSGS> msgs;
    std::array<bool, MAX_NUM_JSON_MSGS> msg_ok;
    for (size_t i = 0; i < num_json_msgs_found; i++) {
        auto result = deserializeJson(msgs[i], &buf[json_msg_starts[i]]);
        if (result == DeserializationError::Ok) {
            msg_ok[i] = true;
        }
    }

    // For all valid messages, modify or print the relevant item in the state
    // field registry
    for (size_t i = 0; i < num_json_msgs_found; i++) {
        if (!msg_ok[i]) continue;
        JsonVariant msg_mode = msgs[i]["mode"];
        JsonVariant field = msgs[i]["field"];

        // Check sanity of data
        if (field.isNull()) continue;

        const char* field_name = field.as<const char*>();
        if (msg_mode.isNull()) {
            _print_error_state_field(field_name, unspecified_mode, missing_mode);
            continue;
        }

        if (!msg_mode.is<unsigned char>()) {
            _print_error_state_field(field_name, unspecified_mode, invalid_mode_not_char);
            continue;
        }
        const unsigned char mode = msg_mode.as<unsigned char>();

        // If data is ok, proceed with state field reading/writing
        switch (mode) {
            case 'r': {
                std::shared_ptr<ReadableStateFieldBase> field_ptr =
                    registry.find_readable_field(field_name);
                if (!field_ptr) {
                    _print_error_state_field(field_name, read_mode, invalid_field_name);
                    break;
                } else
                    print_state_field(*field_ptr);
            } break;
            case 'w': {
                JsonVariant field_val = msgs[i]["val"];
                if (field_val.isNull()) {
                    _print_error_state_field(field_name, write_mode, missing_field_val);
                    break;
                }

                std::shared_ptr<WritableStateFieldBase> field_ptr =
                    registry.find_writable_field(field_name);
                if (!field_ptr) {
                    std::shared_ptr<ReadableStateFieldBase> field_ptr_readable =
                        registry.find_readable_field(field_name);
                    if (field_ptr_readable) {
                        _print_error_state_field(field_name, write_mode, field_is_only_readable);
                    } else {
                        _print_error_state_field(field_name, write_mode, invalid_field_name);
                    }
                    break;
                }

                const char* field_val_serialized = field_val.as<const char*>();
                if (!field_ptr->deserialize(field_val_serialized)) {
                    _print_error_state_field(field_name, write_mode, invalid_field_val);
                    break;
                }
                print_state_field(*field_ptr);

            } break;
            default: {
                _print_error_state_field(field_name, read_mode, invalid_mode);
            }
        }
    }
}
