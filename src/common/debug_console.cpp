#include "debug_console.hpp"
#include <ArduinoJson.h>
#include <array>
#include <cstdarg>

#ifdef DESKTOP
    #include <iostream>
#else
    #include <Arduino.h>
#endif

std::map<debug_severity, const char*> debug_console::severity_strs{
    {debug_severity::debug, "DEBUG"},   {debug_severity::info, "INFO"},
    {debug_severity::notice, "NOTICE"}, {debug_severity::warning, "WARNING"},
    {debug_severity::error, "ERROR"},   {debug_severity::critical, "CRITICAL"},
    {debug_severity::alert, "ALERT"},   {debug_severity::emergency, "EMERGENCY"},
};

std::map<debug_console::state_field_error, const char*> debug_console::state_field_error_strs{
    {state_field_error::invalid_field_name, "invalid field name"},
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
unsigned int debug_console::_start_time = 0;
#else
std::chrono::steady_clock::time_point debug_console::_start_time =
    std::chrono::steady_clock::now();
#endif

debug_console::debug_console() {}

unsigned int debug_console::_get_elapsed_time() {
#ifdef DESKTOP
    std::chrono::milliseconds ms_since_start =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - _start_time);
    return ms_since_start.count();
#else
    unsigned int current_time = millis();
    if (!Serial) {
        /** Reset the start time if Serial is unconnected. We
         * do this so that the logging utility on the computer
         * can always produce a correct timestamp.
         */
        _start_time = current_time;
    }
    unsigned int elapsed_time = current_time - _start_time;
    return elapsed_time;
#endif
}

void debug_console::_print_json_msg(severity s, const char* msg) {
#ifdef DESKTOP
    DynamicJsonDocument doc(2000);
#else
    StaticJsonDocument<500> doc;
#endif
    doc["t"] = _get_elapsed_time();
    doc["svrty"] = severity_strs[s];
    doc["msg"] = msg;

#ifdef DESKTOP
    serializeJson(doc, std::cout);
    std::cout << std::endl << std::flush;
#else
    serializeJson(doc, Serial);
    Serial.println();
#endif
}

void debug_console::init() {
    if (!is_initialized) {
#ifdef DESKTOP
        std::cin.tie(nullptr);
        running = true;
        reader_thd = std::make_shared<std::thread>([this] { this->_reader(); });
#else
        Serial.begin(115200);
        pinMode(13, OUTPUT);

        Serial.println("Waiting for serial console.");
        while (!Serial)
            ;
        _start_time = millis();
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
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
#endif
}

void debug_console::print_state_field(const SerializableStateFieldBase& field) {
#ifdef DESKTOP
    DynamicJsonDocument doc(500);
#else
    StaticJsonDocument<200> doc;
#endif
    doc["t"] = _get_elapsed_time();
    doc["field"] = field.name().c_str();
    doc["val"] = field.print();
#ifdef DESKTOP
    serializeJson(doc, std::cout);
    std::cout << std::endl << std::flush;
#else
    serializeJson(doc, Serial);
    Serial.println();
#endif
}

void debug_console::_print_error_state_field(const char* field_name,
                                             const debug_console::state_cmd_mode mode,
                                             const debug_console::state_field_error error_code) {
#ifdef DESKTOP
    DynamicJsonDocument doc(500);
#else
    StaticJsonDocument<200> doc;
#endif
    doc["t"] = _get_elapsed_time();
    doc["field"] = field_name;
    doc["mode"] = state_cmd_mode_strs[mode];
    doc["err"] = state_field_error_strs[error_code];
#ifdef DESKTOP
    serializeJson(doc, std::cout);
    std::cout << std::endl << std::flush;
#else
    serializeJson(doc, Serial);
    Serial.println();
#endif
}

void debug_console::process_commands(const StateFieldRegistry& registry) {
    TRACKED_CONSTANT_C(size_t, SERIAL_BUF_SIZE, 512);
    char buf[SERIAL_BUF_SIZE] = {0};

#ifdef DESKTOP
    std::string input;
    bool found_input = unprocessed_inputs.try_dequeue(input);
    if (!found_input) return;
    input.copy(buf, sizeof(buf));
#else
    for (size_t i = 0; i < SERIAL_BUF_SIZE && Serial.available(); i++) {
        buf[i] = Serial.read();
    }
#endif

    TRACKED_CONSTANT_C(size_t, MAX_NUM_JSON_MSGS, 5);

    // Get all chunks of the buffer that are complete JSON messages. Read at
    // most five messages from the buffer. (It's unlikely that more than 5 messages
    // can fit within a 512 byte buffer)
    size_t json_msg_starts[MAX_NUM_JSON_MSGS] = {0};
    size_t num_json_msgs_found = 0;
    for (size_t i = 0; i < SERIAL_BUF_SIZE && num_json_msgs_found < MAX_NUM_JSON_MSGS; i++) {
        if (buf[i] == '{') {
            json_msg_starts[num_json_msgs_found] = i;
            num_json_msgs_found++;
        }
    }

    // Deserialize all found JSON messages and check their validity
    std::array<StaticJsonDocument<100>, MAX_NUM_JSON_MSGS> msgs;
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
                ReadableStateFieldBase* field_ptr = 
                    registry.find_readable_field(field_name);
                if (!field_ptr) {
                    _print_error_state_field(field_name, read_mode, invalid_field_name);
                    break;
                } else {
                    print_state_field(*field_ptr);
                }
            } break;
            case 'w': {
                JsonVariant field_val = msgs[i]["val"];
                if (field_val.isNull()) {
                    _print_error_state_field(field_name, write_mode, missing_field_val);
                    break;
                }

                // We allow writing to readable state fields in the debug console, so
                // that input values can be simmed.
                ReadableStateFieldBase* field_ptr = registry.find_readable_field(field_name);
                if (!field_ptr) {
                    _print_error_state_field(field_name, write_mode, invalid_field_name);
                    break;
                }

                const char* field_val_serialized = field_val.as<const char*>();
                const bool processed_val = field_ptr->deserialize(field_val_serialized);
                if (!processed_val) {
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

#ifdef DESKTOP
void debug_console::_reader() {
    while(running) {
        std::string input;
        std::getline(std::cin, input);
        unprocessed_inputs.enqueue(input);
    }
}
#endif

debug_console::~debug_console() {
#ifdef DESKTOP
    if(running) {
        running = false;
        reader_thd->join();
    }
#endif
}
