#include <unity.h>
#include <string>
#include <vector>
#include "../lib/Drivers/QLocate.hpp"
#include "../test_quake/quake_common.h"
#include "core_pins.h"
#include "usb_serial.h"
/*
    ISU AT Command Reference pg 105
    Note: AT+SBDWB returns one of the 4 responses above (0, 1, 2, 3) with 0 indicating success. In
    all cases except response 1, the response is followed by result code ‘OK’. 
*/

// name, port, pin number, timeout
Devices::QLocate q("Test_Quake_No_Network", &Serial3, Devices::QLocate::DEFAULT_NR_PIN, Devices::QLocate::DEFAULT_TIMEOUT);

// Test initializing SBD session with Quake 
// Expecting no network connection
void test_sbdix_no_network(void) {

    // Start SBD session
    TEST_ASSERT_EQUAL(0, q.run_sbdix()); // Expect 0 unless SBD session already running

    // Cannot send messages when we are in SDB session
    std::string testString("Test SBDIX already running");
    int statusCode = q.sbdwb(testString.c_str(), testString.length());
    TEST_ASSERT_EQUAL(-1, statusCode);

    // End SBD session
    while(!Serial3.available());
    delay(100);
    while(!Serial3.available());
    // SBD session should still be running
    TEST_ASSERT_TRUE(q.sbdix_is_running());
    TEST_ASSERT_EQUAL(0, q.end_sbdix()); // Expect 0 since sbdix should return response codes always

    const int *_pRes = q.get_sbdix_response();
    sbdix_r_t *pRes = (sbdix_r_t *)(_pRes);

    // Expect no network
    TEST_ASSERT_EQUAL(MO_NO_NETWORK, pRes->MO_status);
    // Expect 2 because will not be able to check mailbox
    TEST_ASSERT_EQUAL(MT_MSG_ERR, pRes->MT_status);
}

int main(void) {
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    q.setup();
    UNITY_BEGIN();
    RUN_TEST(test_sbdix_no_network);
    UNITY_END();
    return 0;
}

