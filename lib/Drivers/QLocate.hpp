//
//  QLocate.hpp
//  QLocate
//
//  Created by Kyle Krol (kpk63@cornell.edu) on 3/04/18.
//  Pathfinder for Autonomous Navigation
//  Cornell University
//

#ifndef QLocate_hpp
#define QLocate_hpp

#ifndef DESKTOP
#include <HardwareSerial.h>
#include "../Devices/Device.hpp"
#else
#include <iostream>
#include <string>
#endif


namespace Devices
{

/*! RETURN values:
 * All methods return one of the following return codes.
 * Nonzero codes indicate failure. 
 * Codes 0 - 3 is consistent with SBDWB codes
 * */
static constexpr int OK = 0;                    // command was succesfully executed, expected response received
static constexpr int TIMEOUT = 1;               // response not received before timeout
static constexpr int BAD_CHECKSUM = 2;          // checksum doesn't match ISU calculated checksum
static constexpr int WRONG_LENGTH = 3;          // message size differs from expected message size
static constexpr int UNEXPECTED_RESPONSE = -20; // actual response does not match expected response
static constexpr int WRITE_FAIL = -30;          // failed to send command (write command to output port)
static constexpr int WRONG_STATE = -40;         // driver is not in the expected state
static constexpr int PORT_UNAVAILABLE =
    -50; // attempt to read port that is not available (no data available)
static constexpr int UNKNOWN = -60;        // unknown errror
static constexpr int WRONG_FN_ORDER = -70; // attempt to execute commands in the wrong order (unexpected order)
                                           // Ex: calling query_config_2() without calling query_config_1()

/**
 * Functions are formated as follows:
 * Methods that send commands are prefixed with "query_" followed by the name
 * of their command
 * Methods that end a command sequence are prefixed with "get_" followed by the name
 * of their command. These methods usually return a status code
 * or retrieve a message.
 *
 * Methods are numbered by the order in which they should be executed.
 * The client is responsible for executing methods in the correct order.
 *
 * The first method of an operation will have a "1" suffix. (Ex: query_sbdwb_1)
 *
 * "query_" methods with a suffix greater than 1 implicitly read some expected
 * response ("READY", "OK", etc.) and then sends another message. The client
 * is responsible for ensuring the correct delay timings between queries.
 * Calling the next "query_" method too early will likely return 
 * PORT_UNAVAILABLE. 
 *
 * "get_" methods end the command sequence of the operation and will set the
 */

/*! Driver class for the QLocate attached to an arbitrary serial port. Supports
 *  the 3 wire communication interface with no ring alerts. The following
 *  communications are supported: sbdrb, sbdix, and sbdwb.
 */
#ifndef DESKTOP
class QLocate : public Device
#else
class QLocate
#endif
{
public:
    /** Default pin # for network ready pin. **/
    static constexpr unsigned char DEFAULT_NR_PIN = 35;
    /** Default timeout for serial communications on device. **/
    static constexpr unsigned int DEFAULT_TIMEOUT = 10;
    /** Maximum size of an MT or MO message **/ 
    static constexpr int MAX_MSG_SIZE = 340;

    /*! Sets the QLocate serial port and serial timeout value. Do not Initialize
     *  the serial port with begin(), it will be done in the constructor.
     */
#ifndef DESKTOP
    QLocate(const std::string &name, HardwareSerial *const port, unsigned char nr_pin, int timeout);
#else 
    using String = std::string;
    QLocate();
#endif
    /*! Sets up QLocate. Initializes state to IDLE */
#ifndef DESKTOP
    bool setup() override;
#else
    bool setup();
#endif

    /*! Sends an AT message to test comms. */
    int query_is_functional_1();

    /*! Attempts to retrieve 'OK' response at port */
    int get_is_functional();

    /*! This manipulates the settings of the QLocate and sets it to communicate
     *  via the 3 pin interface.
     * This method sends the QLocate the following AT
     *  commands:
     *    1. AT&F0       - restores to factory defaults
     *    2. AT&K0       - disable RTS & CTS flow control
     *    3. AT&D0       - ignores the DTR pin
     *    4. ATE0        - disables echo
     *    5. ATV0        - sets responses to numeric mode
     *    6. AT+SBDMTA=0 - disables RING alerts
     *    7. +SBDD2      - clears the QLocate's buffer
     *  Returns: 
     *  OK if it was succesful,
     *  PORT_UNAVAILABLE if no data at port
     */
    int query_config_1();
    int query_config_2();
    int query_config_3();
    int get_config();

    /*! SBDWB operation:
     * Loads a message into the MO buffer to be sent during the next
     * SBD session.
     * The MO buffer can only hold a single message at a time with a
     * max length of 340 bytes.
     */
    /*! Request to load a message of size [len].
     * Returns WRONG_LENGTH if len > 340 */
    virtual int query_sbdwb_1(int len);
    /*! Loads the message into MO Buffer if READY is received. */
    virtual int query_sbdwb_2(char const *c, int len);
    /*! Attempt to retrieve sbdwb status code returned from loading the message */
    virtual int get_sbdwb();

    /*! Initilizes an SBDIX session with the quake.
     *  Returns a OK if the method was successful (driver was IDLE).
     */
    virtual int query_sbdix_1();

    /*! Reads the response to the previous SBDIX session.
     * Returns
     * PORT_UNAVAILABLE if no response has been received
     * OK if successfully received response and wrote to response array
     */
    virtual int get_sbdix();

    /*! Initializes SBDRB session.
     */
    virtual int query_sbdrb_1();

    /*! Reads data from the MT buffer on the QLocate into message.
     * Returns
     * OK on success
     * PORT_UNAVAILABLE for no response
     * WRONG_LENGTH for invalid message size
     * UNEXPECTED_RESPONSE if message does not match message size
     * BAD_CHECKSUM for incorrect message checksum
     */
    virtual int get_sbdrb();

    /*! Returns pin # for Network Ready pin. */
    unsigned char nr_pin();

    /**
     * sbdix command response array of the following format: 
     * +SBDIX:<MO status>,<MOMSN>,<MT status>,<MTMSN>,<MT length>,<MT queued>
     * */
    int sbdix_r[6];

    /**
     * Contains the contents of the Mobile terminated (MT) message 
     * retreived from the last SBDRB session. 
     */
    char mt_message[MAX_MSG_SIZE];

private:

    /*! Serial port designated to the QLocate */
#ifndef DESKTOP
    HardwareSerial *const port;
    int timeout;
#endif

    /*! Attempts to read [expected] from the QLocate's serial port.
     * Clears the port after a reading an amount of bytes equal to 
     * the length of [expected]. 
     * Returns:
     * OK if the expected response is read
     * UNEXPECTED_RESPONSE if an unexpected response is read
     * PORT_UNAVAILABLE if no response is read
     */
    int consume(String expected);


    /*! Returns a message checksum according to the Iridium requirements */
    short checksum(char const *c, int len);

    /*! Network ready pin (unused) */
    unsigned char nr_pin_;

    /** ! Parses the data returned from requesting SBD transfer (AT+SBDIX)
     * Example:
     * char const *c = "SBDI: 1, 2173, 1, 87, 429, 0";
     * This function would parse c into
     * i = {1, 2173, 1, 87, 429, 0};
     */
    int parse_ints(char const *c, int *i);

    /**
     * Clears the read port and writes the string to the port
     * Returns
     * OK if at least one byte is written
     * WRITE_FAIL if no bytes were written */
    int sendCommand(const char *);

    /** Does nothing */
#ifndef DESKTOP
    void disable() override;
#endif
};
} 
#endif
