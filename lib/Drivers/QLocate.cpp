//
//  QLocate.hpp
//  QLocate
//
//  Created by Kyle Krol (kpk63@cornell.edu) on 3/04/18.
//  Pathfinder for Autonomous Navigation
//  Cornell University
//
#include "QLocate.hpp"
#include <Arduino.h>
#define DEBUG_ENABLED
using namespace Devices;

#define ASSERT_STATE(expect) \
    if (CurrentState != expect) return WRONG_STATE;

#define CHECK_PORT_AVAILABLE()   \
    if (!port->available()) {    \
        return PORT_UNAVAILABLE; \
    }

/*! QLocate implementation */
QLocate::QLocate(const std::string &name, HardwareSerial *const port, unsigned char nr_pin,
                 int timeout)
    : Device(name), port(port), timeout(timeout), nr_pin_(nr_pin) {}

bool QLocate::setup() {
    // Initialize class variables
    this->message = QuakeMessage();
    // Configure QLocate port settings
    this->port->begin(19200);
    this->port->setTimeout(timeout);
    CurrentState = IDLE;
    return true;
}

int QLocate::query_is_functional() {
    ASSERT_STATE(IDLE);
    CurrentState = IS_FN;
    return port->printf("AT\r");
}

int QLocate::get_is_functional() {
    ASSERT_STATE(IS_FN);
    CHECK_PORT_AVAILABLE();
    CurrentState = IDLE;
    return consume('OK\r\n');
}

int const *QLocate::get_sbdix_response() { return this->sbdix_r; }

int QLocate::query_config_1() {
    // Config can run in any state
    CurrentState = CONFIG;
    port->clear();
    // Restore factory defaults
    return port->print(F("AT&F0\r"));
}

int QLocate::query_config_2() {
    ASSERT_STATE(CONFIG);
    int status = consume(F("\r\nOK\r\n"));
    if (status != OK) return status;
    // Disable flow control, disable DTR, disabl echo, set numeric rasponses, and
    // disable "RING" alerts
    return port->print(F("AT&K0;&D0;E0;V0;+SBDMTA=0\r"));
}
int QLocate::query_config_3() {
    ASSERT_STATE(CONFIG);
    int status = consume(F("AT&K0;&D0;E0;V0;+SBDMTA=0\r0\r"));
    if (status != OK) return status;
    // Clear QLocate MO and MT buffers
    return port->print(F("AT+SBDD2\r"));
}

int QLocate::end_config() {
    ASSERT_STATE(CONFIG);
    CurrentState = IDLE;
    int status;
    if ((status = consume(F("0\r\n0\r"))) != OK) return status;
    return OK;
}

int QLocate::query_sbdwb_1(int len) {
    ASSERT_STATE(IDLE);
    CurrentState = SBDWB;
    // Clear serial buffer
    port->clear();
    // Request to write binary data
    if (len <= 0 || len > MAX_MSG_SIZE) return WRONG_LENGTH;

    return port->printf("AT+SBDWB=%d\r", len);
}

int QLocate::query_sbdwb_2(char const *c, int len) {
    ASSERT_STATE(SBDWB);
    int status = consume(F("READY\r\n"));
    if (status != OK) return status;

    // Write binary data to QLocate
    port->write(c, len);
    short s = checksum(c, len);
    port->write((char)(s >> 8));
    port->write((char)s);
    port->flush();
#ifdef DEBUG_ENABLED
    Serial.print("        > mes=");
    for (int i = 0; i < len; i++) Serial.print(c[i], HEX);
    Serial.print((char)(s >> 8), HEX);
    Serial.println((char)s, HEX);
#endif
    return OK;
}

int QLocate::get_sbdwb_response() {
    ASSERT_STATE(SBDWB);
    CHECK_PORT_AVAILABLE();
    // Process sbdwb response
    char buf[6];
    memset(buf, '\0', 6);
    // If 5 bytes are not read before timeout,
    //      then expect to read 1\r\n after 60 seconds
    // else expect to read <sbdwb status>\r\n0\r
    int len = port->readBytes(buf, 5);

#ifdef DEBUG_ENABLED
    Serial.print("        > res=");
    for (int i = 0; i < len; i++) Serial.print(buf[i], HEX);
    Serial.println("\n        > return=" + String(*buf));
#endif
    // Assume timeout if we failed to read 5 bytes
    if (len < 5) return TIMEOUT;
    CurrentState = IDLE;
    // SBDWB status is returned in buf[0]
    int status = buf[0] - '0';

    if (buf[1] != '\r' || buf[2] != '\n' || buf[3] != '0' || buf[4] != '\r')
        return UNEXPECTED_RESPONSE;

    return status;
}

// Since we are timing out, attempt to catch the three bytes: 1\r\n
int QLocate::get_sbdwb_timeout() {
    ASSERT_STATE(SBDWB);
    CHECK_PORT_AVAILABLE();
    CurrentState = IDLE;
    return consume('1\r\n');
}

// Parses the result buffer of sbdix into sbdix_r
int QLocate::parse_ints(char const *c, int *i) {
    int status = sscanf(c, "%d, %d, %d, %d, %d, %d\r", i, i + 1, i + 2, i + 3, i + 4, i + 5);
    if (status == 6) return OK;
    return UNEXPECTED_RESPONSE;
}

int QLocate::query_sbdix() {
    ASSERT_STATE(IDLE);
    CurrentState = SBDIX;
    // Request sbdix session
    return port->print(F("AT+SBDIX\r"));
}

// Requires 15 seconds
int QLocate::get_sbdix() {
    // Ensure sbdix is sbdix is running and data is available
    ASSERT_STATE(SBDIX);
    CHECK_PORT_AVAILABLE();
    CurrentState = IDLE;
    // Parse quake output
    char buf[75];
    memset(buf, '\0', 75);
    port->readBytesUntil('\n', buf, 74);
    port->clear();
    return parse_ints(buf + 8, sbdix_r);
}

int QLocate::query_sbdrb() {
    ASSERT_STATE(IDLE);
    CurrentState = SBDRB;
    // Ensure no ongoing sbdix session and flush buffer
    port->clear();
    // Request data
    return port->print(F("AT+SBDRB\r"));
}

int QLocate::get_sbdrb() {
    ASSERT_STATE(SBDRB);
    CHECK_PORT_AVAILABLE();
    CurrentState = IDLE;
    // Capture incoming data and check checksum
    short s = 0;
    if (2 != port->readBytes((char *)&s, 2)) return WRONG_LENGTH;

    unsigned short size = (s & 0xFF) << 8 | (s >> 8);
#ifdef DEBUG_ENABLED
    Serial.println("sbdrb > recieving message size= " + String(size));
#endif
    memset(message.mes, '\0', MAX_MSG_SIZE);

    // Attempt to read the message
    if (size + 2 != (unsigned short)port->readBytes(message.mes, size + 2))
        return WRONG_LENGTH;  // Quake::Message read fails

    // Clear read port
    port->clear();

    // Verify checksum
    s = checksum(message.mes, size);
    if (((s & 0xFF) << 8 | (s >> 8)) != *(short *)(message.mes + size))
        return BAD_CHECKSUM;  // Checksum error detected

#ifdef DEBUG_ENABLED
    // for (int i = 0; i < 340; i++) {
    //     Serial.printf("[%c]", message.mes[i]);
    // }
    Serial.printf("Message: [%s]", message.mes);
    Serial.flush();
#endif
    return OK;
}

unsigned char QLocate::nr_pin() { return nr_pin_; }

QuakeMessage &QLocate::get_message() { return message; }

// Read the data at port and make sure it matches expected
int QLocate::consume(String expected) {
    // Return if nothing at the port
    CHECK_PORT_AVAILABLE();

    int expectLength = expected.length();
    char buf[expectLength + 1];
    memset(buf, '\0', expectLength + 1);

    // Read current data at port
    int len = port->readBytes(buf, expectLength);
    buf[len] = '\0';

    // Clear the port in case there was more data at the port than expected
    port->clear();

    // Data read does not match expected data
    if (!expected.equals(String(buf))) return UNEXPECTED_RESPONSE;

    return OK;
}

// Calculate checksum
short QLocate::checksum(char const *c, int len) {
    short s = 0;
    char const *const cf = c + len;
    while (c < cf) s += *(c++);
    return s;
}

int QLocate::GetCurrentState() { return CurrentState; }

void QLocate::disable() {
    // Do nothing; we really don't want to disable Quake
}