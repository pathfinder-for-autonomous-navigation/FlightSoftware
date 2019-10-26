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

#define CHECK_PORT_AVAILABLE() \
    if (!port->available()) return PORT_UNAVAILABLE;

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

int QLocate::query_is_functional_1() {
    ASSERT_STATE(IDLE);
    CurrentState = IS_FUNCTIONAL;
    return sendCommand("AT\r");
}

int QLocate::get_is_functional() {
    ASSERT_STATE(IS_FUNCTIONAL);
    int status = consume(F("0\r"));
    CurrentState = IDLE;
    if (status != OK) return status;
    return OK;
}

int const *QLocate::get_sbdix_response() { return this->sbdix_r; }

int QLocate::query_config_1() {
    // Config can run in any state
    port->clear();
    CurrentState = CONFIG;
    return sendCommand("AT&F0\r");
}

int QLocate::query_config_2() {
    ASSERT_STATE(CONFIG);
    CHECK_PORT_AVAILABLE();
    port->clear();  // we don't care what is returned for factory reset
    // Disable flow control, disable DTR, disabl echo, set numeric rasponses, and
    // disable "RING" alerts
    return sendCommand("AT&K0;&D0;E0;V0;+SBDMTA=0\r");
}

int QLocate::query_config_3() {
    ASSERT_STATE(CONFIG);
    int status = consume(F("AT&K0;&D0;E0;V0;+SBDMTA=0\r0\r"));
    if (status != OK) return status;
    // Clear QLocate MO and MT buffers
    return sendCommand("AT+SBDD2\r");
}

int QLocate::get_config() {
    ASSERT_STATE(CONFIG);
    CurrentState = IDLE;
    return consume(F("0\r\n0\r"));
}

int QLocate::query_sbdwb_1(int len) {
    ASSERT_STATE(IDLE);
    // Check the length of the message.
    if (len <= 0 || len > MAX_MSG_SIZE) return WRONG_LENGTH;
    // Only set the state of if [len] is valid.
    CurrentState = SBDWB;
    port->clear();
    if (port->printf("AT+SBDWB=%d\r", len) == 0) return WRITE_FAIL;
    return OK;
}

int QLocate::query_sbdwb_2(char const *c, int len) {
    ASSERT_STATE(SBDWB);
    int status = consume(F("READY\r\n"));
    if (status != OK) return status;

    // Write binary data to QLocate
    if ((size_t)len != port->write(c, len)) return WRITE_FAIL;
    short s = checksum(c, len);
    if (port->write((char)(s >> 8)) != 1) return WRITE_FAIL;
    if (port->write((char)s) != 1) return WRITE_FAIL;
    // WARNING: this method blocks
    port->flush();
#ifdef DEBUG_ENABLED
    Serial.print("        > mes=");
    for (int i = 0; i < len; i++) Serial.print(c[i], HEX);
    Serial.print((char)(s >> 8), HEX);
    Serial.println((char)s, HEX);
#endif
    return OK;
}

int QLocate::get_sbdwb() {
    ASSERT_STATE(SBDWB);
    // If it is a timeout, then port will not be available anyway
    CHECK_PORT_AVAILABLE();
    // Process sbdwb response
    char buf[6] = {0};
    // expect to read <sbdwb status>\r\n0\r
    int len = port->readBytes(buf, 5);

#ifdef DEBUG_ENABLED
    Serial.print("        > res=");
    for (int i = 0; i < len; i++) Serial.print(buf[i], HEX);
    Serial.println("\n        > return=" + String(*buf));
#endif
    CurrentState = IDLE;
    // SBDWB status is returned in buf[0]
    int status = buf[0] - '0';

    if (len < 3) return UNKNOWN;

    if (buf[1] != '\r' || buf[2] != '\n') return UNEXPECTED_RESPONSE;

    return status;
}

// Parses the result buffer of sbdix into sbdix_r
int QLocate::parse_ints(char const *c, int *i) {
    int status = sscanf(c, "%d, %d, %d, %d, %d, %d\r", i, i + 1, i + 2, i + 3, i + 4, i + 5);
    if (status == 6) return OK;
    return UNEXPECTED_RESPONSE;
}

int QLocate::query_sbdix_1() {
    ASSERT_STATE(IDLE);
    CurrentState = SBDIX;
    // Request sbdix session
    return sendCommand("AT+SBDIX\r");
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
    return parse_ints(buf + 8, sbdix_r);
}

int QLocate::query_sbdrb_1() {
    ASSERT_STATE(IDLE);
    CurrentState = SBDRB;
    // Request data
    return sendCommand("AT+SBDRB\r");
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
    memset(message.mes, 0, MAX_MSG_SIZE);

    // Attempt to read the message
    if (size + 2 != (unsigned short)port->readBytes(message.mes, size + 2))
        return UNEXPECTED_RESPONSE;  // Quake::Message read fails

    // Verify checksum
    s = checksum(message.mes, size);
    if (((s & 0xFF) << 8 | (s >> 8)) != *(short *)(message.mes + size)) return BAD_CHECKSUM;

#ifdef DEBUG_ENABLED
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
    char buf[expectLength + 1] = {0};
    // Read current data at port
    int len = port->readBytes(buf, expectLength);

#ifdef DEBUG_ENABLED
    Serial.printf("Consumed[");
    for (int i = 0; i < len; i++) {
        if (buf[i] == '\r')
            Serial.printf("\\r");
        else if (buf[i] == '\n')
            Serial.printf("\\n");
        else
            Serial.printf("%c", buf[i]);
    }
    Serial.printf("]\n");
    Serial.flush();
#endif
    port->clear();
    // Did not read enough bytes
    if (len != expectLength) return WRONG_LENGTH;

    // Data read does not match expected data
    if (!expected.equals(String(buf))) return UNEXPECTED_RESPONSE;
    return OK;
}

int QLocate::sendCommand(const char *cmd) {
    // The writer is responsible for clearing the read port
    port->clear();
    // port->print returns the number of characters printed
    if (port->print(F(cmd)) != 0) return OK;
    return WRITE_FAIL;
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

int QLocate::bPortAvail() { return port->available(); }