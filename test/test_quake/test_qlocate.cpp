#include <unity.h>
#include <string>
#include <vector>
#include "../lib/Drivers/QLocate.hpp"
#include "../lib/utils/QuakeMessage.hpp"
#include <Arduino.h>
// Sbdix response structure
typedef struct sbdix_r {
    int MO_status;
    int MOMSN;
    int MT_status;
    int MTMSN;
    int MT_length;
    int MT_queued;
} sbdix_r_t;

Devices::QLocate q("anything", &Serial3, 35, 10);


void setUp(void) {
    Serial.begin(9600);
    Serial.println("Entering setup");
    q.setup();
    Serial.println("Setup done");
}

void tearDown(void) {}

void test_sbdwb_no_session(void) {
    Serial.println("Running test_sbdwb_no_session");
    // testCases is a vector of pairs <testString, expectedStatus>
    std::vector<std::pair<std::string, int>> testCases{
        std::make_pair("noResponseTest", -1),  // expect error code -1
        std::make_pair("test2", -1),           // expect 0
    };

    for (std::pair<std::string, int> test : testCases) {
        // Retrieve the status code
        const std::string &st = test.first;
        int len = st.length();
        int statusCode = q.sbdwb((test.first).c_str(), len);
        TEST_ASSERT_EQUAL(test.second, statusCode);
    }
}

void test_sbdwb(void) {
    Serial.println("Running test_sbdwb");
    // testCases is a vector of pairs <testString, expectedStatus>
    std::vector<std::pair<std::string, int>> testCases{
        std::make_pair("test2", 0),  // expect 0

    };
    // Start sbdix session
    q.run_sbdix();
    Serial.println("run_sbdix done");
    for (std::pair<std::string, int> test : testCases) {
        // Retrieve the status code
        int statusCode = q.sbdwb((test.first).c_str(), (test.first).length());
        TEST_ASSERT_EQUAL(test.second, statusCode);
    }
    // End sbdix session
    q.end_sbdix();
    Serial.println("end_sbdix done");
}

void test_getresponse(void) {
    Serial.println("Running test_getresponse");
    q.run_sbdix();
    TEST_ASSERT_EQUAL(0, q.sbdwb("get_response_test", 17));
    q.end_sbdix();
    Serial.println("Getting sbdix_response");
    const int *pResp = q.get_sbdix_response();
    sbdix_r_t *pRes = (sbdix_r_t *)(pResp);
    TEST_ASSERT_EQUAL(0, pRes->MO_status);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_sbdwb_no_session);
    RUN_TEST(test_sbdwb);
    UNITY_END();
}

void loop() {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}