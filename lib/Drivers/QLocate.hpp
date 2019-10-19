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

/*! All methods return one of the following return codes.
 *  All methods return OK if the method was successful.
 *  All methods that attempt to read from port will return PORT_UNAVAILABLE
 *   if the port is not available
 * */

// Return codes
#define OK 0  // message succesfully written/read
// All nonzero return codes indicate failure
#define TIMEOUT 1                // timeout exceeded
#define BAD_CHECKSUM 2           // checksum doesn't match ISU calculated checksum
#define WRONG_LENGTH 3           // message size not correct
#define UNEXPECTED_RESPONSE -20  // Actual response does not match expected response
#define PORT_UNAVAILABLE -50     // Port not available
#define WRONG_STATE -40          // Driver is not in the expected state
#define UNKNOWN -60              // Unknown errror

// QLocate driver states
#define IDLE 0
#define SBDWB 1
#define SBDRB 2
#define SBDIX 3
#define CONFIG 4
#define IS_FN 5

/*! Driver class for the QLocate attached to an arbitrary serial port. Supports
 *  the 3 wire communication interface with no ring alerts. The following
 *  commands are supported: sbdrb, sbdix, and sbdwb.
 */
class QLocate : public Device {
   public:
    /** Default pin # for network ready pin. **/
    static constexpr unsigned char DEFAULT_NR_PIN = 35;
    /** Default timeout for serial communications on device. **/
    static constexpr unsigned int DEFAULT_TIMEOUT = 10;

    /*! Sets the QLocate serial port and serial timeout value. Do not Initialize
     *  the serial port with begin(), it will be done in the constructor.
     */
    QLocate(const std::string &name, HardwareSerial *const port, unsigned char nr_pin, int timeout);

    /*! Sets up QLocate */
    bool setup() override;

    /*! Sends an AT message to test comms. */
    int query_is_functional();

    /*! Attempt to retrieve 'OK' message at port */
    int get_is_functional();

    void disable() override;

    /*! Returns a pointer to the sbdix response codes */
    int const *get_sbdix_response();

    /*! Returns the object containing the MT message */
    QuakeMessage &get_message();

    /*! This manipulates the settings of the QLocate and sets it to communicate
     *  via the 3 pin interface. This method sends the QLocate the following AT
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
    int end_config();

    /*! Prepare a message to be sent during the next sbdix session. The MO buffer
     *  can only hold a single message at a time with a max length of 340 bytes.
     *  Returns:
     *  PORT_UNAVAILABLE if no response is received or
     *  sbdwb specific numeric status code
     */
    /*! Request to load a message of size [len]. Method returns FAIL if
     * len > 340 */
    virtual int query_sbdwb_1(int len);
    /*! Load the message into MO Buffer */
    virtual int query_sbdwb_2(char const *c, int len);
    /*! Retrieve status of loading the message in LastError */
    virtual int get_sbdwb_response();
    /*! Wait for 60 second SBDWB timeout */
    virtual int get_sbdwb_timeout();

    /*! Initilizes an sbdix session with the quake. The results of the session can
     *  be obtained with a call to end_sbdix(). Note that while sbdix is in
     *  in session no other commands can be sent to the qlocate.
     *  Returns a OK if the method was successful.
     */
    virtual int query_sbdix();

    /*! Reads the response to the previous sbdix session. Is no response has been
     *  reveived, the method will return with -1. If the method was succesful and
     *  the sbdix responses were written to the response array, 0 is returned. If
     *  an unexpected response was received, 1 is returned.
     */

    virtual int get_sbdix();

    /*! Makes an SBDRB request
     * Returns SUCCESS as long as no other function is running
     */
    virtual int query_sbdrb();

    /*! Reads data from the MT buffer on the QLocate into message. Returns 0 for
     *  success, -1 for no response/running sbdix, 1 for unexpected input, and
     *  2 for incorrect message size.
     */
    virtual int get_sbdrb();

    /*! Returns pin # for Network Ready pin. */
    unsigned char nr_pin();

   protected:
    /*! sbdix integer response array */
    int sbdix_r[6];

    /*! Mobile terminated (MT) message data */
    QuakeMessage message;

    /*! Serial port designated to the QLocate */
    HardwareSerial *const port;
    int timeout;

    /*! Attempts to read the expected input res from the QLocate's serial port.
     *  Returns 0 if the expected response is read, 1 if an unexpected response is
     *  read, and -1 if no response is read.
     */
    int consume(String expected);

    /*! Returns a message checksum according to the Iridium requirements */
    short checksum(char const *c, int len);

    /*! Get current state code*/
    int GetCurrentState();

    unsigned char nr_pin_;

   private:
    /*!
     *  Represents the current state of QLocate
     *  IDLE 0
     *  SBDWB 1
     *  SBDRB 2
     *  SBDIX 3
     *  CONFIG 4
     *  IS_FN 5
     */
    int CurrentState;

    /*! Parses the data returned from requesting SBD transfer (AT+SBDIX)
    Returns SUCCESS if received 6 numbers
    Example:
        char const *c = "SBDI: 1, 2173, 1, 87, 429, 0";
        This function would parse c into
            i = {1, 2173, 1, 87, 429, 0};
    */
    int parse_ints(char const *c, int *i);
};
}  // namespace Devices

// End QLocate
// -----------------------------------------------------------------------------

#endif
