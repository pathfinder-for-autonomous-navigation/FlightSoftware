//
//  QLocate.hpp
//  QLocate
//
//  Created by Kyle Krol (kpk63@cornell.edu) on 3/04/18.
//  Pathfinder for Autonomous Navigation
//  Cornell University
//
#include "QLocate.hpp"
#ifndef DESKTOP
#include <Arduino.h>
#endif
#ifndef DESKTOP
#define DEBUG_ENABLED
#endif
using namespace Devices;

#define CHECK_PORT_AVAILABLE()                                                 \
  if (!port->available())                                                      \
    return PORT_UNAVAILABLE;

/*! QLocate implementation */
#ifdef DESKTOP
using F = std::string;
QLocate::QLocate() {}
#else
QLocate::QLocate(const std::string &name, HardwareSerial *const port,
                 unsigned char nr_pin, int timeout)
    : Device(name), port(port), timeout(timeout), nr_pin_(nr_pin) {}
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

int QLocate::query_is_functional_1() { return sendCommand("AT\r"); }

int QLocate::get_is_functional() {
  int status = consume(F("0\r"));
  if (status != OK)
    return status;
  return OK;
}

int QLocate::query_config_1() {
#ifndef DESKTOP
  // Config can run in any state
  port->clear();
#endif
  return sendCommand("AT&F0\r");
}

int QLocate::query_config_2() {

#ifndef DESKTOP
  CHECK_PORT_AVAILABLE();
  port->clear(); // we don't care what is returned for factory reset
#endif
  // Disable flow control, disable DTR, disable echo,
  // set numeric responses, and
  // disable "RING" alerts
  return sendCommand("AT&K0;&D0;E0;V0;+SBDMTA=0\r");
}

int QLocate::query_config_3() {
  int status = consume(F("AT&K0;&D0;E0;V0;+SBDMTA=0\r0\r"));
  if (status != OK)
    return status;
  // Clear QLocate MO and MT buffers
  return sendCommand("AT+SBDD2\r");
}

int QLocate::get_config() { return consume(F("0\r\n0\r")); }

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
  int status = consume(F("READY\r\n"));
  if (status != OK)
    return status;

    // Write binary data to QLocate
#ifndef DESKTOP
  if ((size_t)len != port->write(c, len))
    return WRITE_FAIL;
  short s = checksum(c, len);
  if (port->write((char)(s >> 8)) != 1)
    return WRITE_FAIL;
  if (port->write((char)s) != 1)
    return WRITE_FAIL;
  // WARNING: this method blocks
  port->flush();
#endif
#ifdef DEBUG_ENABLED
  Serial.print("        > mes=");
  for (int i = 0; i < len; i++)
    Serial.print(c[i], HEX);
  Serial.print((char)(s >> 8), HEX);
  Serial.println((char)s, HEX);
#endif
  return OK;
}

int QLocate::get_sbdwb() {
#ifdef DESKTOP
  return OK;
#else
  // Process sbdwb response
  char buf[6] = {0};
  int len = 0;
  // If it is a timeout, then port will not be available anyway
  CHECK_PORT_AVAILABLE();
  // expect to read <sbdwb status>\r\n0\r
  len = port->readBytes(buf, 5);

#ifdef DEBUG_ENABLED
  Serial.print("        > res=");
  for (int i = 0; i < len; i++)
    Serial.print(buf[i], HEX);
  Serial.println("\n        > return=" + String(*buf));
#endif
  // SBDWB status is returned in buf[0]
  int status = buf[0] - '0';

  if (len < 3)
    return UNKNOWN;

  if (buf[1] != '\r' || buf[2] != '\n')
    return UNEXPECTED_RESPONSE;

  return status;
#endif
}

// Parses the result buffer of sbdix into sbdix_r
int QLocate::parse_ints(char const *c, int *i) {
  int status = sscanf(c, "%d, %d, %d, %d, %d, %d\r", i, i + 1, i + 2, i + 3,
                      i + 4, i + 5);
  if (status == 6)
    return OK;
  return UNEXPECTED_RESPONSE;
}

int QLocate::query_sbdix_1() {
  // Request sbdix session
  return sendCommand("AT+SBDIX\r");
}

// Requires 15 seconds
int QLocate::get_sbdix() {
#ifdef DESKTOP
  return OK;
#else
  CHECK_PORT_AVAILABLE();
  // Parse SBDIX output
  char buf[75] = {0};
  port->readBytesUntil('\n', buf, 74);
  return parse_ints(buf + 8, sbdix_r);
#endif
}

int QLocate::query_sbdrb_1() { return sendCommand("AT+SBDRB\r"); }

int QLocate::get_sbdrb() {
#ifndef DESKTOP
  CHECK_PORT_AVAILABLE();
  short s = 0;
  // Read the message size
  if (2 != port->readBytes((char *)&s, 2))
    return WRONG_LENGTH;

  size_t size = (s & 0xFF) << 8 | (s >> 8);
#ifdef DEBUG_ENABLED
  Serial.println("sbdrb > recieving message size= " + String(size));
#endif
  memset(mt_message, 0, MAX_MSG_SIZE);
  // Read the message
  if (size != port->readBytes(mt_message, size))
    return UNEXPECTED_RESPONSE; // Quake::Message read
  // Read the checksum
  s = 0;
  if (2 != port->readBytes((char *)&s, 2))
    return UNEXPECTED_RESPONSE;
#ifdef DEBUG_ENABLED
  Serial.printf("Message: [%s]", mt_message);
  Serial.flush();
#endif
  // Verify checksum
  // Possibly just compare s != checksum(message, size)
  if (((s & 0xFF) << 8 | (s >> 8)) != checksum(mt_message, size))
    return BAD_CHECKSUM;
#endif
  return OK;
}

unsigned char QLocate::nr_pin() { return nr_pin_; }

// Read the data at port and make sure it matches expected
int QLocate::consume(String expected) {
#ifdef DESKTOP
  return OK;
#else
  // Return if nothing at the port
  CHECK_PORT_AVAILABLE();
  int expectLength = expected.length();
  char buf[expectLength + 1] = {0};

  // Read current data at port
  int len;
  len = port->readBytes(buf, expectLength);

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
  if (len != expectLength)
    return WRONG_LENGTH;

  // Data read does not match expected data
  if (!expected.equals(String(buf)))
    return UNEXPECTED_RESPONSE;
  return OK;
#endif
}

int QLocate::sendCommand(const char *cmd) {
#ifdef DESKTOP
  return OK;
#else
  // The writer is responsible for clearing the read port
  port->clear();
  // port->print returns the number of characters printed
  if (port->print(F(cmd)) != 0)
    return OK;
  return WRITE_FAIL;
#endif
}

// Calculate checksum
short QLocate::checksum(char const *c, int len) {
  short s = 0;
  char const *const cf = c + len;
  while (c < cf)
    s += *(c++);
  return s;
}

#ifndef DESKTOP
void QLocate::disable() {
  // Do nothing; we really don't want to disable Quake
}
#endif