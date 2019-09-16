//
//  QLocate.hpp
//  QLocate
//
//  Created by Kyle Krol (kpk63@cornell.edu) on 3/04/18.
//  Pathfinder for Autonomous Navigation
//  Cornell University
//
// #define DEBUG_ENABLED
#include "QLocate.hpp"
#include <Arduino.h>

using namespace Devices;

/*! QLocate implementation */  // -----------------------------------------------

QLocate::QLocate(const std::string &name, HardwareSerial *const port, unsigned char nr_pin,
                 int timeout)
    : Device(name), port(port), timeout(timeout), nr_pin_(nr_pin) {}

bool QLocate::setup() {
    // Initialize class variables
    this->sbdix_running = false;
    this->message = QuakeMessage();
    // Configure QLocate port settings
    this->port->begin(19200);
    this->port->setTimeout(timeout);
    return true;
}

bool QLocate::is_functional() {
    port->printf("AT\r");
    // delayMicroseconds(100);
    delay(10);
    if (!port->available())
        return false;
    else
        return true;
}

void QLocate::reset() { config(); }
void QLocate::disable() {
    // Do nothing; we really don't want to disable Quake
}

bool QLocate::sbdix_is_running() { return this->sbdix_running; }

int const *QLocate::get_sbdix_response() { return this->sbdix_r; }

// returns 0 on success
int QLocate::config() {
    // Ensure no ongoing sbdix session
    if (sbdix_running) return -1;
    // Clear serial buffer
    port->clear();
    // Restore factory defaults
    port->print(F("AT&F0\r"));
    int success = consume(F("\r\nOK\r\n"));
    if (success != 0) {
        port->clear();
        port->print(F("AT&F0\r"));
        success = consume(F("\r\nOK\r\n"));
    }
    if (success == -1) {
#ifdef DEBUG_ENABLED
        Serial.println("config > AT&F0 status=" + String(success));
        Serial.flush();
#endif
        return success;
    }
#ifdef DEBUG_ENABLED
    Serial.println("config > AT&F0 status=" + String(success));
    Serial.flush();
#endif
    // Disable flow control, disable DTR, disabl echo, set numeric rasponses, and
    // disable "RING" alerts
    port->print(F("AT&K0;&D0;E0;V0;+SBDMTA=0\r"));
    delay(10);
    success |= consume(F("AT&K0;&D0;E0;V0;+SBDMTA=0\r0\r"));
#ifdef DEBUG_ENABLED
    Serial.println("       > AT&K0;&D0;E0;V0;+SBDMTA=0 status=" + String(success));
    Serial.flush();
#endif
    // Clear QLocate MO and MT buffers
    port->print(F("AT+SBDD2\r"));
    success |= consume(F("0\r\n0\r"));
#ifdef DEBUG_ENABLED
    Serial.println("       > AT+SBDD2 status=" + String(success));
    Serial.flush();
#endif
    return success;
}

int QLocate::sbdwb(char const *c, int len) {
    // Ensure no ongoing sbdix session
    if (sbdix_running) return -1;
    // Clear serial buffer
    port->clear();
    // Request to write binary data
    // port->print(F("AT+SBDWB="));
    if (len <= 0 || len > 340)
        return 3;
    port->printf("AT+SBDWB=%d\r", len);
    // port->write('\r');
    int code = consume(F("READY\r\n"));
#ifdef DEBUG_ENABLED
    Serial.println("load_mo > write_req_res= " + String(code));
    Serial.flush();
#endif
    if (code != 0) return code;
    
    // Write binary data to QLocate
    port->write(c, len);
    short s = checksum(c, len);
    // Serial.printf("Checksum: [%c %c]\n", s>>8, s);
    port->write((char)(s >> 8));
    port->write((char)s);
    port->flush();
#ifdef DEBUG_ENABLED
    Serial.print("        > mes=");
    for (int i = 0; i < len; i++) Serial.print(c[i], HEX);
    Serial.print((char)(s >> 8), HEX);
    Serial.println((char)s, HEX);
    Serial.flush();
#endif
    // Process QLocate response
    char buf[4];
    len = port->readBytes(buf, 3);
    buf[len] = '\0';
#ifdef DEBUG_ENABLED
    Serial.print("        > res=");
    for (int i = 0; i < len; i++) Serial.print(buf[i], HEX);
    Serial.println("\n        > return=" + String(*buf));
    Serial.flush();
#endif  
    // Serial.printf("\nBUF[%d %d %d]\n", buf[0], buf[1], buf[2]);
    // Serial.flush();
    Serial.flush();
    if (buf[1] != '\r' || buf[2] != '\n') return -1;
    return buf[0] - '0';
}

int QLocate::run_sbdix() {
    // Ensure no ongoing sbdix session
    if (sbdix_running) return -1;
    // Request sbdix session
    port->print(F("AT+SBDIX\r"));
    sbdix_running = true;
#ifdef DEBUG_ENABLED
    Serial.println("run_sbdix > return= 0");
    Serial.flush();
#endif
    return 0;
}

// Parses the result buffer of sbdix into sbdix_r
int const COUNT_MAX = 75;
bool parse_ints(char const *c, int *i) {
    Serial.println(c);
    Serial.flush();
    sscanf(c, "%d, %d, %d, %d, %d, %d\r", i, i+1, i+2, i+3, i+4, i+5);
    // int count = 0;
    // while (*c != '\n' && count < COUNT_MAX) {
    //     *i = 0;
    //     while (*c != ',' && *c != '\r' && *c != ' ' && count++ < 75) {
    //         *i = 10 * *i + *c - '0';
    //         count++;
    //         c++;
    //     }
    //     c++;
    //     i++;
    // }
    return 1;
}

int QLocate::end_sbdix() {
    // Ensure sbdix is sbdix is running and data is available
    delay(15000);
    // Serial.println(port->available());
    // Serial.flush();
    if (!sbdix_running || !port->available()) return -1;
    // Parse quake output
    char buf[75];
    port->readBytesUntil('\n', buf, 74);
    Serial.println(buf);
    Serial.flush();
    return (parse_ints(buf + 8, sbdix_r) - 1);
}

int QLocate::sbdrb() {
    // Ensure no ongoing sbdix session and flush buffer
    if (sbdix_running) return -1;
    port->clear();
    // Request data
    port->print(F("AT+SBDRB\r"));
    // Capture incoming data and check checksum
    short s;
    if (2 != port->readBytes((char *)&s, 2)) return 1;  // Quake::Message length read fails
    unsigned short size = (s & 0xFF) << 8 | (s >> 8);
#ifdef DEBUG_ENABLED
    Serial.println("sbdrb > recieving message size= " + String(size));
    Serial.flush();
#endif
    if (size + 2 != (unsigned short)port->readBytes(message.mes, size + 2))
        return 2;  // Quake::Message read fails
    s = checksum(message.mes, size);
    if (((s & 0xFF) << 8 | (s >> 8)) != *(short *)(message.mes + size)) {
        return 1;  // Checksum error detected
#ifdef DEBUG_ENABLED
        Serial.println("");
        Serial.flush();
#endif
    }
    // Format as a string
    message.mes[size] = '\0';
    return 0;
}

unsigned char QLocate::nr_pin() { return nr_pin_; }

QuakeMessage &QLocate::get_message() { return message; }

int QLocate::consume(String res) {

    // Serial.printf("\nTrying to consume:\n[%s]\n", res.c_str());
    // Serial.flush();
    // Read in current port input
    char buf[res.length() + 1];
    while(!port->available());
    int len = port->readBytes(buf, res.length());
    buf[len] = '\0';
    // Determine status code
    // Serial.printf("\nActually consumed:\n[%s]\n", buf);
    // Serial.flush();
    if (len == 0) return -1;
    return (!res.equals(String(buf)) || port->available());
}

short QLocate::checksum(char const *c, int len) {
    short s = 0;
    char const *const cf = c + len;
    while (c < cf) s += *(c++);
    return s;
}
