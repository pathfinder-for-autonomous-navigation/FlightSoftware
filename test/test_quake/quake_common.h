#pragma once

using callback_fnptr = int (Devices::QLocate::*)();

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

// MT response statuses
const int MT_NO_MSG     = 0;    // No SBD message received from GSS
const int MT_MSG_RECV   = 1;    // SBD message successfully retrieved
const int MT_MSG_ERR    = 2;    // Error occured

// MO response statuses
const int MO_OK         = 0;    // msg transfer ok
const int MO_TOO_BIG    = 1;    // transfer ok but MT message too big for transfer
const int MO_NO_UPDATE  = 2;    // transfer ok but no location update
const int MO_NO_NETWORK = 32;

extern Devices::QLocate q;
void setUp(void) {
}

void tearDown(void) {
}

// length of a ctrl cycle in ms
static const int DEFAULT_CTRL_CYCLE_LENGTH = 120; 
/**
 * [callback] is a function pointer to a member function of QLocate (i.e. get_sbdwb)
 * This function waits [DEFAULT_CTRL_LENGTH] everytime [callback] returns with
 * PORT_UNAVAILABLE.
 * [fnName] is the name of the function that callback points to.
 */
#define count_cycles(callback, fnName) do{\
    int numCycles   = 0;\
    int retCode     = -1;\
    do\
    {\
        delay(DEFAULT_CTRL_CYCLE_LENGTH);\
        retCode = (callback)();\
        ++numCycles;\
    }\
    while (retCode == Devices::PORT_UNAVAILABLE);\
    Serial.printf("Number of cycles before %s: %d\r\n", fnName, numCycles);\
    TEST_ASSERT_LESS_OR_EQUAL(1, retCode);\
    TEST_ASSERT_GREATER_OR_EQUAL(0, retCode);\
}while(0)

/**
 * We already know that SBDWB works, so just run it here
 */
#define run_sbdwb(msg)do{\
    delay(DEFAULT_CTRL_CYCLE_LENGTH);\
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdwb_1(msg.length()));\
    delay(DEFAULT_CTRL_CYCLE_LENGTH);\
    TEST_ASSERT_EQUAL(Devices::OK, q.query_sbdwb_2(msg.c_str(), msg.length()));\
    count_cycles(q.get_sbdwb, "get_sbdwb");\
}while(0)
