
#include "core_pins.h"
#include "usb_serial.h"
#include <fsw/FCCode/Drivers/QLocate.hpp>
#include <unity.h>
#include <string>
#include <vector>
#include "../test_quake/quake_common.h"

// name, port, pin number, timeout
Devices::QLocate q("Test_Quake_With_Network", &Serial3,
    Devices::QLocate::DEFAULT_TIMEOUT);

void test_config(void) {
    TEST_ASSERT_EQUAL(Devices::OK, q.query_config_1());
    count_cycles(q.query_config_2, "query_config_2", Devices::OK);
    count_cycles(q.query_config_3, "query_config_3", Devices::OK);
    count_cycles(q.get_config, "get_config", Devices::OK);
}

/**
 * Tests that we can complete an SBD session request (AT+SBDIX)
 * (Downlink Test)
 **/
void test_sbdix_with_network(void)
{
    // Test sending a string with null bytes and weird characters
    delay(DEFAULT_CTRL_CYCLE_LENGTH);
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdwb_1(101));
    delay(DEFAULT_CTRL_CYCLE_LENGTH);
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdwb_2(
        "hello from PAN!AAAAAAAA\tAAAAAAAA\rAAAAAAA\0AAAAAAAAAA\aAAAAAAABBBBBB\0BBBBBBBBBBBBBB\nBBBBBBBBBBBBBBBBBBDD", 
        101));
    delay(DEFAULT_CTRL_CYCLE_LENGTH);
    TEST_ASSERT_EQUAL(Devices::OK, q.get_sbdwb());

    // SBDIX session
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdix_1());
    count_cycles(q.get_sbdix, "get_sbdix", Devices::OK);

    const int *_pRes = q.sbdix_r;
    sbdix_r_t *pRes = (sbdix_r_t *)(_pRes);

    // If MO_status is within [0, 4], then downlink was successful
    TEST_ASSERT_LESS_OR_EQUAL(4, pRes->MO_status);
}

/* Tests that we can read messages from MT queue */
void test_sbdrb_with_network(void)
{
    // SBDIX session
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdix_1());
    count_cycles(q.get_sbdix, "get_sbdix", Devices::OK);

    const int *_pRes = q.sbdix_r;
    sbdix_r_t *pRes = (sbdix_r_t *)(_pRes);

    // If MO_status is within [0, 4], then downlink was successful
    TEST_ASSERT_LESS_OR_EQUAL(4, pRes->MO_status);

    // Make sure that we have a message
    TEST_ASSERT_GREATER_OR_EQUAL(1, pRes->MT_length);
    TEST_ASSERT_EQUAL(MT_MSG_RECV, pRes->MT_status);

    // SBDRB session
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdrb_1());
    count_cycles(q.get_sbdrb, "get_sbdrb", Devices::OK);
    
    char *szMsg = q.mt_message;
#ifdef DEBUG_ENABLED
    digitalWrite(13, HIGH);
    Serial.printf("*** %s ***\n", szMsg);
    for (int i = 0; i < pRes->MT_length; i++)
    {
        Serial.printf("[%c]", szMsg + 1);
    }
    digitalWrite(13, LOW);
#endif
    Serial.printf("Message: %s\n", szMsg);
}

// TODO: need a way to get messages
int main(void)
{
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    while (!Serial)
        ;
    q.setup();
    Serial.printf("Qlocate Network Test\n");
    UNITY_BEGIN();
    RUN_TEST(test_config); // force a config
    RUN_TEST(test_sbdix_with_network);
    RUN_TEST(test_sbdrb_with_network);
    UNITY_END();
    return 0;
}
