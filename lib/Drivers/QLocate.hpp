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

#include <HardwareSerial.h>
#include "../Devices/Device.hpp"
#include "../utils/QuakeMessage.hpp"

namespace Devices {

#define MAX_MSG_SIZE 340

/*! RETURN values:
 * All methods return one of the following return codes.
 * Nonzero codes indicate failure
 * Codes 0 - 3 is consistent with SBDWB codes
 * */
static constexpr int OK = 0;                     // command was succesfully executed, expected response received
static constexpr int TIMEOUT = 1;                // response not received before timeout
static constexpr int BAD_CHECKSUM = 2;           // checksum doesn't match ISU calculated checksum
static constexpr int WRONG_LENGTH = 3;           // message size differs from expected message size
static constexpr int UNEXPECTED_RESPONSE = -20;  // actual response does not match expected response
static constexpr int PORT_UNAVAILABLE = -50;     // attempt to read port that is not available (no data available)
static constexpr int WRONG_STATE = -40;          // driver is not in the expected state
static constexpr int WRITE_FAIL = -30;           // failed to send command (write command to output port)
static constexpr int UNKNOWN = -60;              // unknown errror

/**
 * QLocate driver states
 * QLocate is initialized to IDLE in setup()
 * Operations are split into command sequences since many operations require
 * multiple commands. Also, we want to account for differences in timing
 * between sending commands and reading expected responses.
 */
static constexpr int IDLE = 0;
static constexpr int SBDWB = 1;          // SBDWB operation
static constexpr int SBDRB = 2;          // SBDRB operation
static constexpr int SBDIX = 3;          // SBDIX operation
static constexpr int CONFIG = 4;         // Config operation
static constexpr int IS_FUNCTIONAL = 5;  // Is_Functional operation

/**
 * Functions are formated as follows:
 * Methods that send commands are prefixed with "query_" followed by the name
 * of their operation
 * Methods that end an operation are prefixed with "get_" followed by the name
 * of their operation. These methods usually return a status code
 * or retrieve a message.
 *
 * Methods are numbered by the order in which they should be executed.
 * The client is responsible for executing methods in the correct order.
 * The driver only checks that QLocate is in the appropriate state.
 *
 * The first method of an operation will have a "1" suffix. (Ex: query_sbdwb_1)
 * All methods with "1" require that the driver be in an IDLE state and will
 * change the state of the driver iff the method was successful.
 * If the method fails, the driver state is reset to IDLE.
 * The exception to this is the CONFIG command, which may be called from any
 * state since it resets the Quake.
 *
 * "query_" methods with a suffix greater than 1 implicitly reads some expected
 * response ("READY", "OK", etc.) and then sends another message. The client
 * is responsible for ensuring the correct delay timings between queries.
 * The driver must be in the appropriate state. These methods will not change
 * the driver state. Calling the next "query_" method too early will likely
 * return PORT_UNAVAILABLE. Calling these methods in the incorrect state
 * will return WRONG_STATE.
 *
 * "get_" methods end the command sequence of the operation and will set the
 * state of the driver to IDLE provided that it reads data from the port. If
 * the method returns PORT_UNAVAILABLE or TIMEOUT
 * then it will not reset state to IDLE
 */

/*! Driver class for the QLocate attached to an arbitrary serial port. Supports
 *  the 3 wire communication interface with no ring alerts. The following
 *  communications are supported: sbdrb, sbdix, and sbdwb.
 */
class QLocate : public Device {
   public:
    int bPortAvail();
    /** Default pin # for network ready pin. **/
    static constexpr unsigned char DEFAULT_NR_PIN = 35;
    /** Default timeout for serial communications on device. **/
    static constexpr unsigned int DEFAULT_TIMEOUT = 10;

    /*! Sets the QLocate serial port and serial timeout value. Do not Initialize
     *  the serial port with begin(), it will be done in the constructor.
     */
    QLocate(const std::string &name, HardwareSerial *const port, unsigned char nr_pin, int timeout);

    /*! Sets up QLocate. Initializes state to IDLE */
    bool setup() override;

    /*! Sends an AT message to test comms. */
    int query_is_functional_1();

    /*! Attempts to retrieve 'OK' response at port */
    int get_is_functional();

    /*! Returns a pointer to the sbdix response codes */
    int const *get_sbdix_response();

    /*! Returns the MT message as a QuakeMessage object */
    QuakeMessage &get_message();

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
     *
     * Note: Requires 10ms delay between calls
     */
    int query_config_1();
    int query_config_2();
    int query_config_3();
    int get_config();

    /*! SBDWB operation:
     * Loads a message into the MO buffer to be sent during the next
     * sbdix session.
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

    /*! Returns the current status of the driver */
    int GetCurrentState();

   protected:
    /*! sbdix integer response array */
    int sbdix_r[6];

    /*! Mobile terminated (MT) message data */
    QuakeMessage message;

    /*! Serial port designated to the QLocate */
    HardwareSerial *const port;
    int timeout;

    /*! Attempts to read [expected] from the QLocate's serial port.
     * Does not clear the port after a read. 
     * Returns:
     * OK if the expected response is read
     * UNEXPECTED_RESPONSE if an unexpected response is read
     * PORT_UNAVAILABLE if no response is read
     */
    int consume(String expected);

    /*! Returns a message checksum according to the Iridium requirements */
    short checksum(char const *c, int len);

    unsigned char nr_pin_;

   private:
    /**! Represents the current state of the driver. See above for specific states */
    int CurrentState;

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
    void disable() override;
};
}  // namespace Devices
// End QLocate
// -----------------------------------------------------------------------------

#endif
