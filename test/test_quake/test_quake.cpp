#include <unity.h>
#include <string>
#include <vector>
#include "../lib/Drivers/QLocate.hpp"
#include "../lib/utils/QuakeMessage.hpp"
#include "core_pins.h"
#include "quake_common.h"
#include "usb_serial.h"

#define DEFAULT_DELAY 100

// name, port, pin number, timeout
Devices::QLocate q("Test_Quake", &Serial3, Devices::QLocate::DEFAULT_NR_PIN,
                   Devices::QLocate::DEFAULT_TIMEOUT);

/*! Tests the config function */
void test_config(void) {
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(Devices::OK, q.query_config_1());
    delay(DEFAULT_DELAY);
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.query_config_2());
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.query_config_3());
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.get_config());
}

/*! Tests SBDWB (loading a message onto the MO queue of the Quake) */
void test_sbdwb(void) {
    std::string testString(66, '~');
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.query_sbdwb_1(testString.length()));
    TEST_ASSERT_EQUAL(Devices::SBDWB, q.GetCurrentState());
    // Test do not allow query when in sbdwb
    TEST_ASSERT_EQUAL(Devices::WRONG_STATE, q.query_sbdix_1());
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.query_sbdwb_2(testString.c_str(), testString.length()));
    delay(DEFAULT_DELAY);
    // Test do not alloq get sbdix when in sbdwb
    TEST_ASSERT_EQUAL(Devices::WRONG_STATE, q.get_sbdix());
    TEST_ASSERT_EQUAL(Devices::OK, q.get_sbdwb());

    // Write a different message
    delay(DEFAULT_DELAY);
    std::string otherMsg("Test write other message");
    TEST_ASSERT_EQUAL(0, q.query_sbdwb_1(otherMsg.length()));
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.query_sbdwb_2(otherMsg.c_str(), otherMsg.length()));
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(Devices::OK, q.get_sbdwb());

    // test that we can send the maximum number of bytes
    std::string maxLenMsg(340, 'a');
    delay(DEFAULT_DELAY);

    TEST_ASSERT_EQUAL(0, q.query_sbdwb_1(maxLenMsg.length()));
    // test timeout responses
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.query_sbdwb_2(maxLenMsg.c_str(), maxLenMsg.length() - 1));
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(Devices::PORT_UNAVAILABLE, q.get_sbdwb());
    TEST_ASSERT_EQUAL(Devices::SBDWB, q.GetCurrentState());
    delay(1000 * 30);
    // Second time should fail because still waiting
    TEST_ASSERT_EQUAL(Devices::PORT_UNAVAILABLE, q.get_sbdwb());
    TEST_ASSERT_EQUAL(Devices::SBDWB, q.GetCurrentState());
    delay(1000 * 30);
    // Third time should succeed
    TEST_ASSERT_EQUAL(Devices::TIMEOUT, q.get_sbdwb());
    TEST_ASSERT_EQUAL(Devices::IDLE, q.GetCurrentState());

    // test that we should not be able to send max + 1 number of bytes
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(Devices::WRONG_LENGTH, q.query_sbdwb_1(341));
    // Make sure state is still IDLE since query_sbdwb_1 failed
    TEST_ASSERT_EQUAL(Devices::IDLE, q.GetCurrentState());
    // test that we can not send zero number of bytes
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(Devices::WRONG_LENGTH, q.query_sbdwb_1(0));
    TEST_ASSERT_EQUAL(Devices::IDLE, q.GetCurrentState());

    // test that we can send the minimum number of bytes
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdwb_1(1));
    // test bad checksum by sending extraneous data
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.query_sbdwb_2("ZZ", 2));
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(Devices::BAD_CHECKSUM, q.get_sbdwb());
    // Check that if bad checksum is encountered, we still reset state to idle
    TEST_ASSERT_EQUAL(Devices::IDLE, q.GetCurrentState());
}

/* Sends an AT+ command*/
void test_isFunctional(void) {
    TEST_ASSERT_EQUAL(0, q.query_is_functional_1());
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.get_is_functional());
}

int main(void) {
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    while (!Serial)
        ;
    q.setup();
    UNITY_BEGIN();
    RUN_TEST(test_config);
    RUN_TEST(test_isFunctional);
    RUN_TEST(test_sbdwb);
    UNITY_END();
    return 0;
}
