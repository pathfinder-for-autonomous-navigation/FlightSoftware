#include <Arduino.h>
#include <unity.h>
#include <string>
#include <vector>
#include "../lib/Drivers/QLocate.hpp"
#include "../test_quake/quake_common.h"
#include "core_pins.h"
#include "usb_serial.h"
// name, port, pin number, timeout
Devices::QLocate q("Test_Quake_With_Network", &Serial3, Devices::QLocate::DEFAULT_NR_PIN,
                   Devices::QLocate::DEFAULT_TIMEOUT);

/*Tests that when we requst to start an SBD session by sending AT+SBDIX, that
 we get the expected response */
void test_sbdix_with_network(void) {

    // Load a message on ISU
    std::string testString("Send this message to ISU");
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(testString.c_str(), testString.length()));
    // Start SBD session
    TEST_ASSERT_EQUAL(0, q.run_sbdix());
    while (!Serial3.available())
        ;
    delay(100);
    while (!Serial3.available())
        ;
    // End SBD session
    TEST_ASSERT_EQUAL(0, q.end_sbdix());
    // Get SBDI response
    const int *_pRes = q.get_sbdix_response();
    sbdix_r_t *pRes = (sbdix_r_t *)(_pRes);
    // If MO_status [0, 2], then downlink was successful
    // But we only pass if we receive a 0
    TEST_ASSERT_EQUAL(MO_OK, pRes->MO_status);
}

/* Tests that we can read messages from MT queue */
void test_sbdrb_with_network(void) {
    // Load a message on ISU
    std::string testString("Test Receive Message");
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(testString.c_str(), testString.length()));
    TEST_ASSERT_EQUAL(0, q.run_sbdix());
    // While loop is here because to account for timing delays
    // when attempting to receive response from SBDIX
    while (!Serial3.available())
        ;
    delay(100);
    while (!Serial3.available())
        ;
    TEST_ASSERT_EQUAL(0, q.end_sbdix());

    // Get SBDI response
    const int *_pRes = q.get_sbdix_response();
    sbdix_r_t *pRes = (sbdix_r_t *)(_pRes);
    // If MO_status [0, 2], then downlink was successful
    // But we only pass if we receive a 0
    TEST_ASSERT_EQUAL(MO_OK, pRes->MO_status);
    // Test that we have a message
    TEST_ASSERT_GREATER_OR_EQUAL(1, pRes->MT_length);
    // Read message
    TEST_ASSERT_EQUAL(0, q.sbdrb());
    QuakeMessage msg = q.get_message();
    TEST_ASSERT_NOT_NULL(msg.mes);
}

// TODO: need a way to get messages
int main(void) {
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    q.setup();
    while (!Serial)
        ;
    UNITY_BEGIN();
    RUN_TEST(test_sbdix_with_network);
    // RUN_TEST(test_sbdrb_with_network);
    UNITY_END();
    return 0;
}
