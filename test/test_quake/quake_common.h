#pragma once


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
}
