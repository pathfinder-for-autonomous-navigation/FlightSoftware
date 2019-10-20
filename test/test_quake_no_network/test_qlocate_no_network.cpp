#include <unity.h>
#include <string>
#include <vector>
#include "../lib/Drivers/QLocate.hpp"
#include "../test_quake/quake_common.h"
#include "core_pins.h"
#include "usb_serial.h"

#define DEFAULT_DELAY 10
// name, port, pin number, timeout
Devices::QLocate q("Test_Quake_No_Network", &Serial3, Devices::QLocate::DEFAULT_NR_PIN,
                   Devices::QLocate::DEFAULT_TIMEOUT);

// Test initializing SBD session with Quake
// Expecting no network connection
void test_sbdix_no_network(void) {
    // Start SBD session
    TEST_ASSERT_EQUAL(0, q.query_sbdix_1()); // Expect 0
    TEST_ASSERT_EQUAL(Devices::SBDIX, q.GetCurrentState());

    // Test that querying sbdwb doesn't work and our state remains in sbdix
    TEST_ASSERT_EQUAL(Devices::WRONG_STATE, q.query_sbdwb_1(1));
    TEST_ASSERT_EQUAL(Devices::SBDIX, q.GetCurrentState());

    // Port should be unavailable at this point
    TEST_ASSERT_EQUAL(Devices::PORT_UNAVAILABLE, q.get_sbdix());
    // Wait to talk to Iridium
    while (!Serial3.available())
        ;
    delay(100);
    // This is odd, what exactly is being printed? 
    while (!Serial3.available())
        ;
    // Expect 0 since sbdix should return response codes always
    TEST_ASSERT_EQUAL(0, q.get_sbdix()); 
    TEST_ASSERT_EQUAL(Devices::IDLE, q.GetCurrentState());

    const int *_pRes = q.get_sbdix_response();
    sbdix_r_t *pRes = (sbdix_r_t *)(_pRes);

    // Expect no network
    // Example: +SBDIX: 32, 8, 2, 0, 0, 0\r\n0\r
    TEST_ASSERT_EQUAL(MO_NO_NETWORK, pRes->MO_status);
    // Expect MT_MSG_ERR (2) because will not be able to check mailbox
    TEST_ASSERT_EQUAL(MT_MSG_ERR, pRes->MT_status);
}

void test_recover_with_config(void) {
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.query_sbdwb_1(2));

    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(0, q.query_sbdwb_2("L", 1));
    
    TEST_ASSERT_EQUAL(Devices::PORT_UNAVAILABLE, q.get_sbdwb());
    delay(1000*30);

    // Test allowing config to be called whenever
    TEST_ASSERT_EQUAL(Devices::SBDWB, q.GetCurrentState());
    TEST_ASSERT_EQUAL(0, q.query_config_1());
    TEST_ASSERT_EQUAL(Devices::CONFIG, q.GetCurrentState());
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(Devices::SBDWB, q.GetCurrentState());
    TEST_ASSERT_EQUAL(0, q.query_config_2());
    delay(DEFAULT_DELAY);
    TEST_ASSERT_EQUAL(Devices::SBDWB, q.GetCurrentState());
    TEST_ASSERT_EQUAL(0, q.query_config_3());
    TEST_ASSERT_EQUAL(0, q.get_config());
    TEST_ASSERT_EQUAL(Devices::IDLE, q.GetCurrentState());
    delay(1000*30);
    // We hope that calling config() allows us to not have to wait for 1\r\n
    TEST_ASSERT_TRUE(q.bPortAvail());
    delay(1000*10);
    TEST_ASSERT_TRUE(q.bPortAvail());
    delay(1000*30);
    TEST_ASSERT_TRUE(q.bPortAvail());
}

int main(void) {
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    q.setup();
    UNITY_BEGIN();
    RUN_TEST(test_sbdix_no_network);
    RUN_TEST(test_recover_with_config);
    UNITY_END();
    return 0;
}
