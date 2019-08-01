#ifndef QUAKE_MESSAGE_HPP_
#define QUAKE_MESSAGE_HPP_

/*! Holds the data for a mobile terminated (MT) message form the QLocate.
*/  // -----------------------------------------------------------------------
class QuakeMessage {
    friend class QLocate;

   public:
    /*! Empty constructor sets message length to zero */
    QuakeMessage();
    /*! Constructor sets message to desired length */
    QuakeMessage(unsigned int len);
    /*! Copy constructor */
    QuakeMessage(QuakeMessage const &mes);
    /*! Message length in bytes */
    int length;
    /*! Assignment operator */
    QuakeMessage &operator=(QuakeMessage const &mes);
    /*! Allows array style access to the char array */
    char &operator[](int i);
    /*! Allows read-only array style access to the char array */
    char operator[](int i) const;
    /*! Copies the current message to the provided char array */
    void copy_message(char *c) const;
    /*! Message data pointer */
    char mes[340];
};

#endif