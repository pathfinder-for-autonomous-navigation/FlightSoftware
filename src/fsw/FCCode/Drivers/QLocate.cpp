//
//  QLocate.hpp
//  QLocate
//
//  Created by Kyle Krol (kpk63@cornell.edu) on 3/04/18.
//  Pathfinder for Autonomous Navigation
//  Cornell University
// 
// Updated 12/23/2019
// Refactored on 1/25/2020
// - all buffers now initialized to zero
// - mt_buffer is cleared at the beginning of sbdrb
// - sbdix buffer cleared at the beginning of sbdix
#include "QLocate.hpp"
#include <cstring>

#ifndef DESKTOP

#include <Arduino.h>

#endif

using namespace Devices;

#define CHECK_PORT_AVAILABLE() \
    if (!port->available()){    \
        return PORT_UNAVAILABLE;}

#ifdef DESKTOP
using F = std::string;
QLocate::QLocate(const std::string &name) : Device(name) {}
#else

QLocate::QLocate(const std::string &name, HardwareSerial *const port, int timeout)
        : Device(name), port(port), timeout(timeout) {}

#endif

bool QLocate::setup() {
#ifndef DESKTOP
    // Configure QLocate port settings
    this->port->begin(19200);
    // Initialize class variables
    this->port->setTimeout(timeout);
#endif
    return true;
}

int QLocate::query_is_functional_1() {
    return sendCommand("AT\r");
}

int QLocate::get_is_functional() {
    return consume(F("0\r"));
}

int QLocate::query_config_1() {
    return sendCommand("AT&F0\r");
}

int QLocate::query_config_2() {
#ifndef DESKTOP
    CHECK_PORT_AVAILABLE()
#endif
    // Disable flow control, disable DTR, disable echo, 
    // set numeric responses, and
    // disable "RING" alerts
    return sendCommand("AT&K0;&D0;E0;V0;+SBDMTA=0\r");
}

int QLocate::query_config_3() {
    int status = consume(F("AT&K0;&D0;E0;V0;+SBDMTA=0\r0\r"));
    // Clear QLocate MO and MT buffers
    return (status != OK) ? status : sendCommand("AT+SBDD2\r");
}

int QLocate::get_config() {
    return consume(F("0\r\n0\r"));
}

int QLocate::query_sbdwb_1(int len) {
    // Check the length of the message.
    if (len <= 0 || len > MAX_MSG_SIZE)
        return WRONG_LENGTH;
#ifndef DESKTOP
    port->clear();
    if (port->printf("AT+SBDWB=%d\r", len) == 0)
        return WRITE_FAIL;
#endif
    return OK;
}

int QLocate::query_sbdwb_2(char const *c, int len) {
#ifndef DESKTOP
    CHECK_PORT_AVAILABLE()
    int status = consume(F("READY\r\n"));
    if (status != OK)
        return status;

    // Write binary data to QLocate
    if ((size_t) len != port->write(c, len))
        return WRITE_FAIL;
    // Compute checksum
    uint16_t s = checksum(c, len);
    // Write checksum first byte
    if (port->write((uint8_t) (s >> 8u)) != 1)
        return WRITE_FAIL;
    // Write checksum second byte
    if (port->write((uint8_t) s) != 1)
        return WRITE_FAIL;
    port->flush();
#endif
    return OK;
}

int QLocate::get_sbdwb() {
#ifdef DESKTOP
    return OK;
#else
    // If it is a timeout, then port will not be available anyway
    CHECK_PORT_AVAILABLE()
    char buf[6]{};
    // expect to read <sbdwb status>\r\n0\r
    int len = port->readBytes(buf, 5);

#ifdef DEBUG_ENABLED
    Serial.print("        > get_SBDWB=");
    for (int i = 0; i < len; i++)
        Serial.print(buf[i], HEX);
    Serial.println("\n        > return=" + String(*buf));
#endif
    // SBDWB status is returned in buf[0]
    int status = buf[0] - '0';

    if (len < 3)
        return UNKNOWN;

    if (buf[1] != '\r' || buf[2] != '\n' || status > 3)
        return UNEXPECTED_RESPONSE;

    return status;
#endif
}

int QLocate::query_sbdix_1() {
    // Clear SBDIX buffer before writing to it
    memset(sbdix_r, 0, 6*sizeof(int));
    // Request sbdix session
    return sendCommand("AT+SBDIX\r");
}

// Requires 15 seconds
int QLocate::get_sbdix() {
#ifdef DESKTOP
    return OK;
#else
    CHECK_PORT_AVAILABLE()
    size_t msg_size = port->available();
    // min message length is 26 excluding the final 0\r
    if (msg_size < 26)
        return PORT_UNAVAILABLE;
    if (msg_size >= 128) // 128 is more than enough
        return UNEXPECTED_RESPONSE;

    char buf[128]{};
    // Parse SBDIX output
    port->readBytes(buf, msg_size);
    return parse_ints(buf + 8, sbdix_r);
#endif
}

// Parses the result buffer of sbdix into sbdix_r
// Example Response "+SBDIX: 3, 8, 2, 0, 0, 0\r\n0\r"
int QLocate::parse_ints(char const *c, int *i) {
    int res = sscanf(c, "%d, %d, %d, %d, %d, %d\r", i, i + 1, i + 2, i + 3, i + 4, i + 5);
    if (res == 6)
        return OK;

#ifdef DEBUG_ENABLED
    Serial.printf("parse_ints: unexpected response %d\n", res);
#endif
    return UNEXPECTED_RESPONSE;
}

int QLocate::query_sbdrb_1() {
    // Clear the mt buffer here so that in case of failure, we will not have the old message
    memset(mt_message, 0, MAX_MSG_SIZE + 1);
    return sendCommand("AT+SBDRB\r");
}

int QLocate::get_sbdrb() {
#ifndef DESKTOP
    CHECK_PORT_AVAILABLE()
    if (should_wait())
        return PORT_UNAVAILABLE;

    size_t msg_size = 256 * port->read() + port->read();
    if (msg_size > MAX_MSG_SIZE)
        return WRONG_LENGTH;

    // make sure port has our message + 2 byte checksum
    if ((size_t) port->available() != msg_size + 2)
        return UNEXPECTED_RESPONSE;

    // we definitely have the correct number of bytes at port
    port->readBytes(mt_message, msg_size);

    // get the checksum
    uint16_t msg_checksum = 256 * port->read() + port->read();

    // check the checksum
    if (msg_checksum == checksum(mt_message, msg_size))
        return OK;

    return BAD_CHECKSUM;
#else
    return OK;
#endif
}

// should_wait() should always be called AFTER CHECK_PORT_AVAILABLE()
bool QLocate::should_wait() {
#ifndef DESKTOP
    if (port->available() == num_bytes_available_last_cycle) {
        num_bytes_available_last_cycle = 0; // reset to 0
        return true;
    }
    num_bytes_available_last_cycle = port->available();
    // if we return true then that means we have not waited a cycle
#endif
    return false;
}

int QLocate::consume(const String &expected) {
#ifdef DESKTOP
    return OK;
#else
    CHECK_PORT_AVAILABLE()
    // If reached here then port->available() != 0
    size_t expected_len = expected.length();

    if (should_wait()) {
        return PORT_UNAVAILABLE;
    }
    // If we have reached here, then port->available() >= expected_len

    // Read the bytes at port into rx_buf
    char rx_buf[expected_len + 1];
    port->readBytes(rx_buf, expected_len);
    rx_buf[expected_len] = 0;

#ifdef DEBUG_ENABLED
    Serial.printf("Consumed[");
    for (size_t i = 0; i < expected_len; i++)
    {
        if (rx_buf[i] == '\r')
            Serial.printf("\\r");
        else if (rx_buf[i] == '\n')
            Serial.printf("\\n");
        else
            Serial.printf("%c", rx_buf[i]);
    }
    Serial.printf("]\n");
    Serial.flush();
#endif
    // Compare expected_len number of chars of rx_buf with expected
    if (!strncmp(rx_buf, expected.c_str(), expected_len))
        return OK;
    return CONSUME_FAIL;
#endif
}

int QLocate::sendCommand(const char *cmd) {
#ifdef DESKTOP
    return OK;
#else
    port->clear();
    // port->print returns the number of characters printed
    return (port->print(F(cmd)) != 0) ? OK : WRITE_FAIL;
#endif
}

// Calculate checksum
uint16_t QLocate::checksum(char const *c, size_t len) {
    uint16_t checksum = 0;
    for (size_t i = 0; i < len; ++i) {
        checksum += (uint16_t) c[i];
    }
    return checksum;
}

void QLocate::disable() {
    // Do nothing; we really don't want to disable Quake
}
