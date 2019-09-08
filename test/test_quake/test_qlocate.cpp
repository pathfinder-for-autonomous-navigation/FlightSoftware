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
  
}

void tearDown(void) {}

void test_sbdwb_sbdix_not_running(void) {
    // testCases is a vector of pairs <testString, expectedStatus>
    std::vector<std::pair<std::string, int>> testCases{
        std::make_pair("Testing SBDWB No Session", 0),  // expect 0 since sbdix is not running
    };

    for (std::pair<std::string, int> test : testCases) {
        // Retrieve the status code
        const std::string &st = test.first;
        int len = st.length();
        int statusCode = q.sbdwb((test.first).c_str(), len);
        TEST_ASSERT_EQUAL(test.second, statusCode);
    }
}

void test_sbdwb_sbdix_running(void) {
    // testCases is a vector of pairs <testString, expectedStatus>
    std::vector<std::pair<std::string, int>> testCases{
        std::make_pair("Test SBDIX is running", -1),  // expect -1 b/c sbdix is running

    };
    // Start sbdix session
    TEST_ASSERT_EQUAL(0, q.run_sbdix()); // Expect 0 unless sbdix session already running
    for (std::pair<std::string, int> test : testCases) {
        // Retrieve the status code
        int statusCode = q.sbdwb((test.first).c_str(), (test.first).length());
        TEST_ASSERT_EQUAL(test.second, statusCode);
    }
    // End sbdix session
    while(!Serial3.available());
    delay(100);
    while(!Serial3.available());
    TEST_ASSERT_TRUE(q.sbdix_is_running());
    TEST_ASSERT_EQUAL(0, q.end_sbdix()); // Expect 0 since sbdix should return response codes always

    const int *pResp = q.get_sbdix_response();
    sbdix_r_t *pRes = (sbdix_r_t *)(pResp);
    TEST_ASSERT_GREATER_OR_EQUAL(3, pRes->MO_status); // If MO_status [0, 2], then downlink was successful
    TEST_ASSERT_GREATER_OR_EQUAL(3, pRes->MT_status);

}

void test_isFunctional(void){
    TEST_ASSERT_TRUE(q.is_functional());
}

void setup() {
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    q.setup();
    // q.setup();
    while(!Serial);
    UNITY_BEGIN();
    RUN_TEST(test_isFunctional);
    RUN_TEST(test_sbdwb_sbdix_not_running);
    RUN_TEST(test_sbdwb_sbdix_running);
    UNITY_END();
}

void loop() {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}