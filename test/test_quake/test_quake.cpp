#include <unity.h>
#include <string>
#include <vector>
#include "../lib/Drivers/QLocate.hpp"
#include "../lib/utils/QuakeMessage.hpp"
#include <Arduino.h>
#include "quake_common.h"

/*
    ISU AT Command Reference pg 105
    Note: AT+SBDWB returns one of the 4 responses above (0, 1, 2, 3) with 0 indicating success. In
    all cases except response 1, the response is followed by result code ‘OK’. 
*/

// name, port, pin number, timeout
Devices::QLocate q("Test_Quake", &Serial3, Devices::QLocate::DEFAULT_NR_PIN, Devices::QLocate::DEFAULT_TIMEOUT);

void test_config(void) {
    TEST_ASSERT_EQUAL(1, q.config());
}

// Test that we get the correct error when we have no SBD session running
void test_sbdwb(void) {

    std::string testString ("Test load message in Quake");
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(testString.c_str(), testString.length()));

    // Sending multiple messages should be ok since MO queues the messages

    std::string timeoutMsg ("Test write timeout");
    TEST_ASSERT_EQUAL(WRITE_TIMEOUT, q.sbdwb(timeoutMsg.c_str(), timeoutMsg.length() + 8));

    std::string badCheckMsg ("Test bad checksum42");
    // Subtracting 2 from length should make it use '42' as checksum
    TEST_ASSERT_EQUAL(BAD_CHECKSUM, q.sbdwb(badCheckMsg.c_str(), badCheckMsg.length() - 2));

    // test that we can send the maximum number of bytes 
    std::string maxLenMsg(340, 'a');
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(maxLenMsg.c_str(), maxLenMsg.length()));

    // test that we should not be able to send max + 1 number of bytes
    std::string badLenMsg(341, 'Z');
    TEST_ASSERT_EQUAL(WRONG_LENGTH, q.sbdwb(badLenMsg.c_str(), 341));

    // test that we can send the minimum number of bytes
    std::string minLenMsg(1, 'Z');
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(minLenMsg.c_str(), 1));

    // test that we can not send the minimum number of bytes
    std::string noMsg = "";
    TEST_ASSERT_EQUAL(WRONG_LENGTH, q.sbdwb(noMsg.c_str(), 0));
}

void test_isFunctional(void){
    TEST_ASSERT_TRUE(q.is_functional());
}

void setup() {
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    q.setup();
    // q.setup();
    while(!Serial);
    UNITY_BEGIN();
    RUN_TEST(test_isFunctional);
    RUN_TEST(test_config);
    RUN_TEST(test_sbdwb);
    UNITY_END();
}

