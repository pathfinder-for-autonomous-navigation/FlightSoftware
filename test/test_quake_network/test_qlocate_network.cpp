#include <unity.h>
#include <string>
#include <vector>
#include "../lib/Drivers/QLocate.hpp"
#include "../lib/utils/QuakeMessage.hpp"
#include <Arduino.h>
#include "../test_quake/quake_common.h"

/*
    ISU AT Command Reference pg 105
    Note: AT+SBDWB returns one of the 4 responses above (0, 1, 2, 3) with 0 indicating success. In
    all cases except response 1, the response is followed by result code ‘OK’. 
*/

// name, port, pin number, timeout
Devices::QLocate q("Test_Quake_With_Network", &Serial3, Devices::QLocate::DEFAULT_NR_PIN, Devices::QLocate::DEFAULT_TIMEOUT);

void test_sbdix_with_network(void) {
    // First, load a message on ISU
    std::string testString("Send this message to ISU");
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(testString.c_str(), testString.length()));
    // Start SBD session
    TEST_ASSERT_EQUAL(0, q.run_sbdix());
    while(!Serial3.available());
    delay(100);
    while(!Serial3.available());
    // End SBD session
    TEST_ASSERT_EQUAL(0, q.end_sbdix());
    // Get SBDI response
    const int *_pRes = q.get_sbdix_response();
    sbdix_r_t *pRes = (sbdix_r_t *)(_pRes);
    // If MO_status [0, 2], then downlink was successful
    TEST_ASSERT_EQUAL(MO_OK, pRes->MO_status);  
}

void test_sbdrb_with_network(void){
    // We expect there to be a message for us
    // First, load a message on ISU
    std::string testString("Test Receive Message");
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(testString.c_str(), testString.length()));
    TEST_ASSERT_EQUAL(0, q.run_sbdix());
    while(!Serial3.available());
    delay(100);
    while(!Serial3.available());
    TEST_ASSERT_EQUAL(0, q.end_sbdix());

    // Get SBDI response
    const int *_pRes = q.get_sbdix_response();
    sbdix_r_t *pRes = (sbdix_r_t *)(_pRes);
    // If MO_status [0, 2], then downlink was successful
    TEST_ASSERT_EQUAL(MO_OK, pRes->MO_status);  
    // Test that we have a message
    TEST_ASSERT_GREATER_OR_EQUAL(1, pRes->MT_length);

    // Read message
    TEST_ASSERT_EQUAL(0, q.sbdrb());
    QuakeMessage msg = q.get_message();
    TEST_ASSERT_NOT_NULL(msg.mes);
}

// TODO: need a way to get messages

void setup() {
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    q.setup();
    while(!Serial);
    UNITY_BEGIN();
    RUN_TEST(test_sbdix_with_network);
    RUN_TEST(test_sbdrb_with_network);
    UNITY_END();
}

