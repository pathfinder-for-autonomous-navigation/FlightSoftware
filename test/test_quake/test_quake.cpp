#include <unity.h>
#include <string>
#include <vector>
#include <QLocate.hpp>
#include "core_pins.h"
#include <Arduino.h>
#include "quake_common.h"
#include "usb_serial.h"

// name, port, pin number, timeout
Devices::QLocate q("Test_Quake", &Serial3, 
    Devices::QLocate::DEFAULT_NR_PIN,
    Devices::QLocate::DEFAULT_TIMEOUT);

/*! Tests the config function */
void test_config(void) {
    TEST_ASSERT_EQUAL(Devices::OK, q.query_config_1());
    count_cycles(q.query_config_2, "query_config_2");
    count_cycles(q.query_config_3, "query_config_3");
    count_cycles(q.get_config, "get_config");
}

/*! Tests SBDWB (loading a message onto the MO queue of the Quake) */
void test_sbdwb(void) {
    std::string testString(66, '~');
    run_sbdwb(testString);

    // Write a different message
    std::string otherMsg("Test write other message");
    run_sbdwb(otherMsg);

    // test that we can send the maximum number of bytes
    std::string maxLenMsg(340, 'a');
    run_sbdwb(otherMsg);

    // test that we can send the minimum number of bytes
    std::string minLenMsg(1, 'b');
    run_sbdwb(minLenMsg);
}

void test_timeout(void) {
    delay(DEFAULT_CTRL_CYCLE_LENGTH);
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdwb_1(42));

    // Force a timeout by not sending all the characters
    delay(DEFAULT_CTRL_CYCLE_LENGTH);
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdwb_2("AAAAA", 5));

    delay(DEFAULT_CTRL_CYCLE_LENGTH);
    TEST_ASSERT_EQUAL(Devices::PORT_UNAVAILABLE, q.get_sbdwb());

    // Second time should fail because still waiting
    delay(1000 * 30);
    TEST_ASSERT_EQUAL(Devices::PORT_UNAVAILABLE, q.get_sbdwb());

    // Third time should succeed
    delay(1000 * 30);
    TEST_ASSERT_EQUAL(Devices::TIMEOUT, q.get_sbdwb());
}

void test_bad_lengths(void){
    // test that we should not be able to send max + 1 number of bytes
    delay(DEFAULT_CTRL_CYCLE_LENGTH);
    TEST_ASSERT_EQUAL(Devices::WRONG_LENGTH, q.query_sbdwb_1(341));
    // test that we cannot send zero number of bytes
    delay(DEFAULT_CTRL_CYCLE_LENGTH);
    TEST_ASSERT_EQUAL(Devices::WRONG_LENGTH, q.query_sbdwb_1(0));

}

void test_bad_checksum(void) {
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdwb_1(1));
    // test bad checksum by sending extraneous data
    delay(DEFAULT_CTRL_CYCLE_LENGTH);
    TEST_ASSERT_EQUAL(0, q.query_sbdwb_2("ZZ", 2));
    delay(DEFAULT_CTRL_CYCLE_LENGTH);
    TEST_ASSERT_EQUAL(Devices::BAD_CHECKSUM, q.get_sbdwb());
}

/* Sends an AT+ command*/
void test_isFunctional(void) {
    TEST_ASSERT_EQUAL(0, q.query_is_functional_1());
    delay(DEFAULT_CTRL_CYCLE_LENGTH);
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
    RUN_TEST(test_timeout);
    RUN_TEST(test_bad_lengths);
    RUN_TEST(test_bad_checksum);
    UNITY_END();
    return 0;
}
