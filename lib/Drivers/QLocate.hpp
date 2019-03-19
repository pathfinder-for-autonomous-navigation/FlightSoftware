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

#include "../utils/QuakeMessage.hpp"
#include <HardwareSerial.h>
#include "../Devices/Device.hpp"

namespace Devices {
/*! Driver class for the QLocate attached to an arbitrary serial port. Supports
 *  the 3 wire communication interface with no ring alerts. The following
 *  commands are supported: sbdrb, sbdix, and sbdwb.
 */ // -------------------------------------------------------------------------
  class QLocate : public Device {
    public:
      /** Default pin # for network ready pin. **/
      static constexpr unsigned char DEFAULT_NR_PIN = 35;
      /** Default timeout for serial communications on device. **/
      static constexpr unsigned int DEFAULT_TIMEOUT = 10;

    /*! Sets the QLocate serial port and serial timeout value. Do not Initialize
    *  the serial port with begin(), it will be done in the constructor.
    */
    QLocate(const std::string& name, HardwareSerial *const port, unsigned char nr_pin, int timeout);

    bool setup() override;
    bool is_functional() override;
    void reset() override;
    void disable() override;

    /*! Returns sbdix_running */
    bool sbdix_is_running();

    /*! Indices for use with sbdix responses */
    static const int MO_STATUS = 0;
    static const int MOMSN     = 1;
    static const int MT_STATUS = 2;
    static const int MTMSN     = 3;
    static const int MT_LENGTH = 4;
    static const int MT_QUEUED = 5;

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
    *  The method returns 0 if it was succesful, 1 if it failed, and -1 if no
    *  response was recieved or if there is a current sbdix session.
    */
    int config();

    /*! Prepare a message to be sent during the next sbdix session. The MO buffer
    *  can only hold a single message at a time with a max length of 340 bytes.
    *  The method will return a -1 if no response is received or a sbdwb specific
    *  numeric status code.
    */
    virtual int sbdwb(char const *c, int len);

    /*! Initilizes an sbdix session with the quake. The results of the session can
    *  be obtained with a call to end_sbdix(). Note that while sbdix is in
    *  in session no other commands can be sent to the qlocate. Returns a -1 if
    *  there is a current sbdix session and a 0 if the method was successful.
    */
    virtual int run_sbdix();

    /*! Reads the response to the previous sbdix session. Is no response has been
    *  reveived, the method will return with -1. If the method was succesful and
    *  the sbdix responses were written to the response array, 0 is returned. If
    *  an unexpected response was received, 1 is returned.
    */
    virtual int end_sbdix();

    /*! Reads data from the MT buffer on the QLocate into message. Returns 0 for
    *  success, -1 for no response/running sbdix, and 1 for unexpected input.
    */
    virtual int sbdrb();

    /*! Returns pin # for Network Ready pin. */
    unsigned char nr_pin();

  protected:
    /*! sbdix integer response array */
    int sbdix_r[6];

    /*! Mobile terminated (MT) message data */
    QuakeMessage message;

    /*! Indicates current sbdix session */
    bool sbdix_running;

    /*! Serial port designated to the QLocate */
    HardwareSerial *const port;
    int timeout;

    /*! Attempts to read the expected input res from the QLocate's serial port.
    *  Returns 0 if the expected response is read, 1 if an unexpected response is
    *  read, and -1 if no response is read.
    */
    int consume(String res);

    /*! Returns a message checksum according to the Iridium requirements */
    short checksum(char const *c, int len);

    unsigned char nr_pin_;
  };
}

// End QLocate
// -----------------------------------------------------------------------------

#endif
