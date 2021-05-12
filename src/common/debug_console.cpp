#include "debug_console.hpp"

#include "ConstexprMap.hpp"
#include <ArduinoJson.h>

#include <array>
#include <cstdarg>
#include <cstdint>

#ifdef DESKTOP
    #include <concurrentqueue.h>

    #include <chrono>
    #include <iostream>
    #include <thread>
#else
    #include <Arduino.h>

    static constexpr std::uint8_t LED = 13;
#endif

static constexpr ConstexprMap<debug_console::severity_t, char const *, 8> severity_strs {{{
    {debug_console::severity_t::debug, "DEBUG"},
    {debug_console::severity_t::info, "INFO"},
    {debug_console::severity_t::notice, "NOTICE"},
    {debug_console::severity_t::warning, "WARNING"},
    {debug_console::severity_t::error, "ERROR"},
    {debug_console::severity_t::critical, "CRITICAL"},
    {debug_console::severity_t::alert, "ALERT"},
    {debug_console::severity_t::emergency, "EMERGENCY"},
}}};

static constexpr ConstexprMap<debug_console::state_field_error_t, char const *, 7> state_field_error_strs {{{
    {debug_console::state_field_error_t::invalid_field_name, "invalid field name"},
    {debug_console::state_field_error_t::missing_mode, "missing mode specification"},
    {debug_console::state_field_error_t::invalid_mode_not_char, "mode value is not a character"},
    {debug_console::state_field_error_t::invalid_mode, "mode value is not 'r' or 'w'"},
    {debug_console::state_field_error_t::missing_field_val, "missing value of field to be written"},
    {debug_console::state_field_error_t::invalid_field_val, "field value was invalid"}
}}};

static constexpr ConstexprMap<debug_console::state_cmd_mode_t, char const *, 3> state_cmd_mode_strs {{{
    {debug_console::state_cmd_mode_t::unspecified_mode, "perform unspecified operation with"},
    {debug_console::state_cmd_mode_t::read_mode, "read"},
    {debug_console::state_cmd_mode_t::write_mode, "write"}
}}};

/** @brief True if the debug console has been opened and false otherwise.
 */
static bool is_open = false;

/** @brief Start time of process overall.
 *
 *  In Arduino this is relative to the millis timer and on desktop the system
 *  time.
 */
#ifndef DESKTOP
static unsigned int start_time = 0;
#else
static std::chrono::steady_clock::time_point const start_time =
        std::chrono::steady_clock::now();

/** @brief Thread responsible for reading data from the input stream in HOOTL.
 */
static std::thread reader_thread;

/** @brief Used to signal the reader thread to halt.
 */
static volatile bool reader_thread_is_running = false;

/** @brief Consumer producer queue facilitating communication between the main
 *         thread and reader thread.
 */
static moodycamel::ConcurrentQueue<std::string> unprocessed_inputs;
#endif

unsigned int debug_console::_get_elapsed_time() {
#ifdef DESKTOP
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time).count();
#else
    unsigned int const current_time = millis();

    /* Reset the start time if Serial is unconnected. We do this so that the
     * logging utility on the computer can always produce a correct timestamp.
     */
    if (!Serial) start_time = current_time;

    return current_time - start_time;
#endif
}

void debug_console::_print_json_msg(severity_t severity, const char* msg) {
#ifdef DESKTOP
    DynamicJsonDocument doc(2000);
#else
    StaticJsonDocument<500> doc;
#endif
    doc["t"] = _get_elapsed_time();
    doc["svrty"] = severity_strs[severity];
    doc["msg"] = msg;

#ifdef DESKTOP
    serializeJson(doc, std::cout);
    std::cout << std::endl;
#else
    serializeJson(doc, Serial);
    Serial.println();
#endif
}

void debug_console::_print_error_state_field(char const *field_name,
        state_cmd_mode_t mode, state_field_error_t error_code) {
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

void debug_console::open() {
    if (is_open) return;

#ifdef DESKTOP
    std::cin.tie(nullptr);
    reader_thread_is_running = true;
    reader_thread = std::thread([&]() -> void {
        std::string input;
        while (reader_thread_is_running) {
            std::getline(std::cin, input);
            unprocessed_inputs.enqueue(input);
        }
    });
#else
    Serial.begin(115200);
    pinMode(LED, OUTPUT);
#endif
    is_open = true;
}

void debug_console::close() {
#ifdef DESKTOP
    if (!is_open) return;

    reader_thread_is_running = false;
    if (reader_thread.joinable()) reader_thread.join();
#endif
}

void debug_console::printf(severity_t severity, const char* fmt, ...) {
    if (!is_open) return;

    char buf[100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    _print_json_msg(severity, buf);
    va_end(args);
}

void debug_console::printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf(severity_t::info, fmt, args);
    va_end(args);
}

void debug_console::println(severity_t severity, char const *msg) {
    if (!is_open) return;
    _print_json_msg(severity, msg);
}

void debug_console::println(char const *msg) {
    println(severity_t::info, msg);
}

void debug_console::blink_led() {
    if (!is_open) return;
#ifndef DESKTOP
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
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

void debug_console::process_commands(const StateFieldRegistry& registry) {
    TRACKED_CONSTANT_C(size_t, SERIAL_BUF_SIZE, 512);
    char buf[SERIAL_BUF_SIZE] = {0};

#ifdef DESKTOP
    std::string input;
    bool found_input = unprocessed_inputs.try_dequeue(input);
    if (!found_input) return;
    input.copy(buf, sizeof(buf));
#else
    char lastchar = '?';
    size_t i = 0;
    if(Serial.available()){ // if there are bytes to be processed
        while(i < SERIAL_BUF_SIZE && lastchar != '\n'){ // loop while we are under buffer limit and we haven't seen end line
            if(Serial.available()){ // only log to buffer if there are bytes available
                lastchar = Serial.read();
                buf[i] = lastchar;
                i++;
            }
        }
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
        if (msg_mode.as<unsigned char>() != 'u' && field.isNull()) continue;

        const char* field_name = field.as<const char*>();
        if (msg_mode.isNull()) {
            _print_error_state_field(field_name, state_cmd_mode_t::unspecified_mode, state_field_error_t::missing_mode);
            continue;
        }

        if (!msg_mode.is<unsigned char>()) {
            _print_error_state_field(field_name, state_cmd_mode_t::unspecified_mode, state_field_error_t::invalid_mode_not_char);
            continue;
        }
        const unsigned char mode = msg_mode.as<unsigned char>();

        // If data is ok, proceed with state field reading/writing
        switch (mode) {
            case 'r': {
                ReadableStateFieldBase* field_ptr = 
                    registry.find_readable_field(field_name);
                if (!field_ptr) {
                    _print_error_state_field(field_name, state_cmd_mode_t::read_mode, state_field_error_t::invalid_field_name);
                    break;
                } else {
                    print_state_field(*field_ptr);
                }
            } break;
            case 'w': {
                JsonVariant field_val = msgs[i]["val"];
                if (field_val.isNull()) {
                    _print_error_state_field(field_name, state_cmd_mode_t::write_mode, state_field_error_t::missing_field_val);
                    break;
                }

                // We allow writing to readable state fields in the debug console, so
                // that input values can be simmed.
                ReadableStateFieldBase* field_ptr = registry.find_readable_field(field_name);
                if (!field_ptr) {
                    _print_error_state_field(field_name, state_cmd_mode_t::write_mode, state_field_error_t::invalid_field_name);
                    break;
                }

                const char* field_val_serialized = field_val.as<const char*>();
                const bool processed_val = field_ptr->deserialize(field_val_serialized);
                if (!processed_val) {
                    _print_error_state_field(field_name, state_cmd_mode_t::write_mode, state_field_error_t::invalid_field_val);
                    break;
                }

                print_state_field(*field_ptr);

            } break;
            case 'u': {
                // Get the internal fields that hold the mt message and its length
                InternalStateField<char*>* radio_mt_packet_fp = static_cast<InternalStateField<char*>*>(registry.find_internal_field("uplink.ptr"));
                InternalStateField<size_t>* radio_mt_packet_len_fp = static_cast<InternalStateField<size_t>*>(registry.find_internal_field("uplink.len"));
                
                // Get the uplink packet and packet length from the device
                size_t uplink_packet_len = msgs[i]["length"];
                JsonVariant packet = msgs[i]["val"]; 

                // Get the uplink packet as a char pointer. 
                // Add "\x" to the end to allow us to parse the uplink string
                static char uplink_packet[300];
                memset(uplink_packet, 0, 300);
                strcpy(uplink_packet, packet);
                strcat(uplink_packet, "\\x");

                // The data array holds the decimal values of the hex string. For example,
                // if the uplink contains "\x4c", the data array will hold 67. 
                char data[uplink_packet_len];

                // Parse the uplink packet; convert it from a hex string to an array of integers.
                // Token holds a single hex value in uplink string; can't possibly be longer than the string length of the packet.
                char token[strlen(packet)];
                char* uplink_packet_ptr = uplink_packet;
                for (i=0; i<uplink_packet_len; i++) {
                    // Get the hex string (i.e "4c") and put it in the token char array
                    uplink_packet_ptr+=2;
                    memset(token, 0, uplink_packet_len); // Clear the token array
                    for (size_t idx = 0; uplink_packet_ptr[0] != '\\'; idx++, uplink_packet_ptr++) {
                        token[idx] = uplink_packet_ptr[0];
                    }

                    // Get the decimal value of the token/hex string (i.e 67) and add it to data array
                    data[i] = (char) strtol ((char*)token, NULL, 16);
                }
                
                // Clear the MT buffer
                size_t size = sizeof(radio_mt_packet_fp->get());
                memset(radio_mt_packet_fp->get(), 0, size);

                // Move the uplink data into the MT buffer so that it can be processed on
                // the next cycle by Uplink Consumer.
                memcpy(radio_mt_packet_fp->get(), data, uplink_packet_len);
                radio_mt_packet_len_fp->set(uplink_packet_len);

                #ifdef DESKTOP
                    DynamicJsonDocument doc(500);
                #else
                    StaticJsonDocument<200> doc;
                #endif
                    doc["t"] = _get_elapsed_time();
                    doc["uplink"] = packet;
                    doc["len"] = uplink_packet_len;
                #ifdef DESKTOP
                    serializeJson(doc, std::cout);
                    std::cout << std::endl << std::flush;
                #else
                    serializeJson(doc, Serial);
                    Serial.println();
                #endif

            } break;
            default: {
                _print_error_state_field(field_name, state_cmd_mode_t::read_mode, state_field_error_t::invalid_mode);
            }
        }
    }
}
