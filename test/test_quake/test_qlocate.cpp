#include <unity.h>
#include <string>
#include <vector>
#include "../lib/Drivers/QLocate.hpp"
#include "../lib/utils/QuakeMessage.hpp"
#include <Arduino.h>

// GSS is responsible for:
//      storing and forwarding messages from the ISU to the Host VA Application 
//      storing messages from the VA Application to forward to the ISU
// ISU communicates with the GSS via the Iridium satellite constellation

// Unset this when we are expected to be able to communicate with Iridium
# define NO_NETWORK

// Driver should support:
// sbdix - initiate an SBD Session Extended
// sbdwb - write binary data to Quake
// sbdrb - read binary data from Quake

// MO with MT Message (Send and receive message in one session)
// 
// AT+SBDWB=351 - Tell Quake that you will send 351 B message
// Receive a READY from Quake
// Send your message followed by 2 byte checksum 
// Receive 0 if message was loaded successfully
// AT+SBDIX - Tell Quake to initiate SBD transfer
// Receive +SBDI: 1, 2173, 1, 87, 429, 0
//      Message was sent succesfully using MOMSN 2173
//      A 429 message received using MTMSN 87
//      No other MT messages queued
// AT+SBDD0 - tell Quake to clear message queue from MO buffer 
// Receive 0 when message buffer cleared successfully 
// AT+SBDRB - tell Quake to transfer message
// Receive 2B length + 429 B message + 2B checksum


// Sbdix response structure
typedef struct sbdix_r {
    // Disposition of mobile originated transaction
    int MO_status; 
    // Sequence number that Quake increments when message successfully sent from Quake to Gateway
    int MOMSN; 
    // Disposition of mobile terminated transaction
    int MT_status; // 0 -> no message to receive, 1 -> msg received, 2-> error
    // Assigned by GSS when forwarding message to Quake (indeterminate value if MT_status = 0)
    int MTMSN;
    // Length in bytes of mobile terminated SBD message received from GSS 
    int MT_length;
    // Count of mobile terminated SBD messages waiting at GSS to transfer to Quake
    int MT_queued;
} sbdix_r_t;

// SBDWB command response statuses
int WRITE_OK = 0;       // message succesfully written to Quake
int WRITE_TIMEOUT = 1;  // insufficent number of bytes transferred in 60 seconds
int BAD_CHECKSUM = 2;   // checksum doesn't match ISU calculated checksum
int WRONG_LENGTH = 3;   // message size not corect

// MT response statuses
const int MT_NO_MSG = 0;
const int MT_MSG_RECV = 1;
const int MT_MSG_ERR = 2;

// MO response statuses
const int MO_OK = 0; // msg transfer ok
const int MO_TOO_BIG = 1; // transfer ok but msg in queue too big for transfer
const int MO_NO_UPDATE = 2; // transfer ok but no location update
const int MO_NO_NETWORK = 32;


/*
    ISU AT Command Reference pg 105
    Note: AT+SBDWB returns one of the 4 responses above (0, 1, 2, 3) with 0 indicating success. In
    all cases except response 1, the response is followed by result code ‘OK’. 
*/

// name, port, pin number, timeout
Devices::QLocate q("QuakeTest", &Serial3, Devices::QLocate::DEFAULT_NR_PIN, Devices::QLocate::DEFAULT_TIMEOUT);


void setUp(void) {
  
}

void tearDown(void) {
    // should clear buffers and restore settings
    q.config();
}

void test_config(void) {
    TEST_ASSERT_EQUAL(1, q.config());
}

// Test that we get the correct error when we have no SBD session running
void test_sbdwb(void) {

    std::string testString ("Test load message in Quake");
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(testString.c_str(), testString.length()));

    // Sending multiple messages should be ok since MO queues the messages

    std::string timeoutMsg ("Test write timeout");
    TEST_ASSERT_EQUAL(WRITE_TIMEOUT, q.sbdwb(timeoutMsg.c_str(), timeoutMsg.length() + 8));

    std::string badCheckMsg ("Test bad checksum42");
    // Subtracting 2 from length should make it use '42' as checksum
    TEST_ASSERT_EQUAL(BAD_CHECKSUM, q.sbdwb(badCheckMsg.c_str(), badCheckMsg.length() - 2));

    // test that we can send the maximum number of bytes 
    std::string maxLenMsg(340, 'a');
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(maxLenMsg.c_str(), maxLenMsg.length));

    // test that we should not be able to send max + 1 number of bytes
    std::string badLenMsg(341, 'Z');
    TEST_ASSERT_EQUAL(WRONG_LENGTH, q.sbdwb(badLenMsg.c_str(), 341));

    // test that we can send the minimum number of bytes
    std::string minLenMsg(1, 'Z');
    TEST_ASSERT_EQUAL(WRITE_OK, q.sbdwb(minLenMsg.c_str(), 1));

    // test that we can not send the minimum number of bytes
    std::string noMsg = "";
    TEST_ASSERT_EQUAL(WRONG_LENGTH, q.sbdwb(noMsg.c_str(), 0));
}

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
    // Expect no message
    TEST_ASSERT_EQUAL(MT_NO_MSG, pRes->MT_status);
}

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
    // TODO: why does sbdrb return 0 when Quake Message fails?
    q.sbdrb(); // not sure what response this should be
    QuakeMessage msg = q.get_message();
    TEST_ASSERT_NOT_NULL(msg.mes);
}

// TODO: need a way to retrieve a message

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
    RUN_TEST(test_config);
    RUN_TEST(test_sbdwb);
#ifdef NO_NETWORK
    RUN_TEST(test_sbdix_no_network);
#endif
#ifndef NO_NETWORK
    RUN_TEST(test_sbdix_with_network);
    RUN_TEST(test_sbdrb_with_network);
#endif
    UNITY_END();
}

void loop() {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}