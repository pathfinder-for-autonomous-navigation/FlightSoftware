#include "debug_console.hpp"
#include <usb_serial.h>
#include <array>
#include "ArduinoJson.h"

/**
 * Needed in order to resolve some strange linking error where
 * Serial appears undefined.
 */
usb_serial_class Serial;

std::map<debug_severity, const char*> debug_console::severity_strs{
    {debug_severity::DEBUG, "DEBUG"},   {debug_severity::INFO, "INFO"},
    {debug_severity::NOTICE, "NOTICE"}, {debug_severity::WARNING, "WARNING"},
    {debug_severity::ERROR, "ERROR"},   {debug_severity::CRITICAL, "CRITICAL"},
    {debug_severity::ALERT, "ALERT"},   {debug_severity::EMERGENCY, "EMERGENCY"},
};

bool debug_console::is_initialized = false;

debug_console::debug_console() : _start_time(static_cast<systime_t>(0)) {}

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

void debug_console::_print_json_msg(severity s, const char* msg) {
    StaticJsonDocument<100> doc;
    doc["t"] = _get_elapsed_time();
    doc["svrty"] = severity_strs.at(s);
    doc["msg"] = msg;
    serializeJson(doc, Serial);
    Serial.println();
}

void debug_console::init() {
    if (!is_initialized) {
        Serial.begin(115200);
        pinMode(13, OUTPUT);

        Serial.println("Waiting for serial console.");
        while (!Serial)
            ;
        _start_time = chVTGetSystemTimeX();

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
    digitalWrite(13, HIGH);
    chThdSleepMilliseconds(500);
    digitalWrite(13, LOW);
    chThdSleepMilliseconds(500);
}

void debug_console::print_state_field(const SerializableStateFieldBase& field) {
    StaticJsonDocument<100> doc;
    doc["t"] = _get_elapsed_time();
    doc["field"] = field.name().c_str();
    doc["val"] = field.print();
    serializeJson(doc, Serial);
    Serial.println();
}

void debug_console::_print_error_state_field(
    const char* field_name, const debug_console::state_field_error_code error_code) {
    StaticJsonDocument<100> doc;
    doc["t"] = _get_elapsed_time();
    doc["field"] = field_name;
    doc["err"] = static_cast<unsigned int>(error_code);
    serializeJson(doc, Serial);
    Serial.println();
}

void debug_console::process_commands(const StateFieldRegistry& registry) {
    constexpr size_t SERIAL_BUF_SIZE = 64;
    char buf[SERIAL_BUF_SIZE] = {0};
    for (size_t i = 0; i < SERIAL_BUF_SIZE && Serial.available(); i++) {
        buf[i] = Serial.read();
    }

    // Get all chunks of the buffer that are complete JSON messages. Read at
    // most five messages from the buffer. (It's unlikely that more than 5 messages
    // can fit within a 64 byte buffer)
    size_t json_msg_starts[5] = {0};
    size_t num_json_msgs_found = 0;
    bool inside_json_msg = false;
    for (size_t i = 0; i < SERIAL_BUF_SIZE && num_json_msgs_found < 5; i++) {
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
    if (num_json_msgs_found > 0) Serial.printf("Num messages found: %d\n", num_json_msgs_found);

    // Deserialize all found JSON messages and check their validity
    std::array<StaticJsonDocument<50>, 5> msgs;
    std::array<bool, 5> msg_ok;
    unsigned int num_msgs_ok = 0;
    for (size_t i = 0; i < num_json_msgs_found; i++) {
        auto result = deserializeJson(msgs[i], &buf[json_msg_starts[i]]);
        if (result == DeserializationError::Ok) {
            num_msgs_ok++;
            msg_ok[i] = true;
        }
    }
    if (num_json_msgs_found > 0) Serial.printf("Num messages ok: %d\n", num_msgs_ok);

    // For all valid messages, modify or print the relevant item in the state
    // field registry
    for (size_t i = 0; i < num_json_msgs_found; i++) {
        if (!msg_ok[i]) continue;
        JsonVariant msg_mode = msgs[i]["r/w"];
        JsonVariant field = msgs[i]["field"];

        // Check sanity of data
        if (field.isNull()) continue;

        const char* field_name = field.as<const char*>();
        if (msg_mode.isNull()) {
            _print_error_state_field(field_name, MISSING_MODE);
            continue;
        }

        if (!msg_mode.is<unsigned char>()) {
            _print_error_state_field(field_name, INVALID_MODE_NOT_CHAR);
            continue;
        }
        const unsigned char mode = msg_mode.as<unsigned char>();

        // If data is ok, proceed with state field reading/writing
        switch (mode) {
            case 'r': {
                std::shared_ptr<ReadableStateFieldBase> field_ptr =
                    registry.find_readable_field(field_name);
                if (!field_ptr) {
                    _print_error_state_field(field_name, INVALID_FIELD_NAME);
                    break;
                } else
                    print_state_field(*field_ptr);
            } break;
            case 'w': {
                JsonVariant field_val = msgs[i]["val"];
                if (field_val.isNull()) {
                    _print_error_state_field(field_name, MISSING_FIELD_VAL);
                    break;
                }

                std::shared_ptr<WritableStateFieldBase> field_ptr =
                    registry.find_writable_field(field_name);
                if (!field_ptr) {
                    std::shared_ptr<ReadableStateFieldBase> field_ptr_readable =
                        registry.find_readable_field(field_name);
                    if (field_ptr_readable) {
                        _print_error_state_field(field_name, FIELD_IS_ONLY_READABLE);
                    } else {
                        _print_error_state_field(field_name, INVALID_FIELD_NAME);
                    }
                    break;
                }

                const char* field_val_serialized = field_val.as<const char*>();
                if (!field_ptr->deserialize(field_val_serialized)) {
                    _print_error_state_field(field_name, INVALID_FIELD_VAL);
                    break;
                }
                print_state_field(*field_ptr);

            } break;
            default: {
                _print_error_state_field(field_name, INVALID_MODE);
            }
        }
    }
}