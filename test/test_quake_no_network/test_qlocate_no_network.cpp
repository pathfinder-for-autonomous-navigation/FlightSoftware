#include "../test_quake/quake_common.h"
#include "core_pins.h"
#include "usb_serial.h"

#include <unity.h>
#include <string>
#include <vector>
#include <QLocate.hpp>

// name, port, pin number, timeout
Devices::QLocate q("Test_Quake_No_Network", &Serial3, 
    Devices::QLocate::DEFAULT_NR_PIN,
    Devices::QLocate::DEFAULT_TIMEOUT);


// Test initializing SBD session with Quake
// Expecting no network connection
void test_sbdix_no_network(void)
{
    // Start SBD session
    TEST_ASSERT_EQUAL(0, q.query_sbdix_1()); // Expect 0

    // Port should be unavailable at this point
    count_cycles(q.get_sbdix, "get_sbdix");

    const int *_pRes = q.sbdix_r;
    sbdix_r_t *pRes = (sbdix_r_t *)(_pRes);

    // Expect no network
    // Example: +SBDIX: 32, 8, 2, 0, 0, 0\r\n0\r
    TEST_ASSERT_EQUAL(MO_NO_NETWORK, pRes->MO_status);

    // Expect MT_MSG_ERR (2) because will not be able to check mailbox
    TEST_ASSERT_EQUAL(MT_MSG_ERR, pRes->MT_status);
}

int main(void)
{
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    q.setup();
    UNITY_BEGIN();
    RUN_TEST(test_sbdix_no_network);
    UNITY_END();
    return 0;
}
