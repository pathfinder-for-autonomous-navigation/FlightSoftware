#pragma once
// GSS is responsible for:
//      storing and forwarding messages from the ISU to the Host VA Application 
//      storing messages from the VA Application to forward to the ISU
// ISU communicates with the GSS via the Iridium satellite constellation


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

extern Devices::QLocate q;
void setUp(void) {
}

void tearDown(void) {
    // should clear buffers and restore settings
    q.config();
}
