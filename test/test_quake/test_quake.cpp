#include <unity.h>
#include <string>
#include <vector>
#include "../lib/Drivers/QLocate.hpp"
#include "../lib/utils/QuakeMessage.hpp"
#include "core_pins.h"
#include "quake_common.h"
#include "usb_serial.h"

#define DEFAULT_DELAY 10

// name, port, pin number, timeout
Devices::QLocate q("Test_Quake", &Serial3, Devices::QLocate::DEFAULT_NR_PIN,
                   Devices::QLocate::DEFAULT_TIMEOUT);

/*! Tests the config function */
void test_config(void) { TEST_ASSERT_EQUAL(0, q.config()); }

/*! Tests SBDWB (loading a message onto the MO queue of the Quake) */
void test_sbdwb(void) {
    std::string testString(66, '~');
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(testString.c_str(), testString.length()));
    delay(DEFAULT_DELAY);
    // Send the same message again to verify that it's possible to do 
    // (it should be, since MO overwrites messages)
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(testString.c_str(), testString.length()));
    delay(DEFAULT_DELAY);

    std::string otherMsg ("Test write other message");
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(otherMsg.c_str(), otherMsg.length()));
    delay(DEFAULT_DELAY);

    // test that we can send the maximum number of bytes
    std::string maxLenMsg(340, 'a');
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(maxLenMsg.c_str(), maxLenMsg.length()));
    delay(DEFAULT_DELAY);
    // test that we should not be able to send max + 1 number of bytes
    std::string badLenMsg(341, 'Z');
    TEST_ASSERT_EQUAL(WRONG_LENGTH, q.sbdwb(badLenMsg.c_str(), 341));
    delay(DEFAULT_DELAY);
    // test that we can send the minimum number of bytes
    std::string minLenMsg(1, 'Z');
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(minLenMsg.c_str(), 1));
    delay(DEFAULT_DELAY);
    // test that we can not send the minimum number of bytes
    std::string noMsg = "";
    TEST_ASSERT_EQUAL(WRONG_LENGTH, q.sbdwb(noMsg.c_str(), 0));
}

/* Sends an AT+ command*/
void test_isFunctional(void) { TEST_ASSERT_TRUE(q.is_functional()); }

int main(void) {
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    q.setup();
    UNITY_BEGIN();
    RUN_TEST(test_isFunctional);
    RUN_TEST(test_config);
    RUN_TEST(test_sbdwb);
    UNITY_END();
    return 0;
}
