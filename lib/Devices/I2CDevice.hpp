/** @file I2CDevice.cpp
 * @author Tanishq Aggarwal
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains declaration for I2CDevice interface, which standardizes the
 * access
 * and control of I2C-based hardware peripherals.
 */

#ifndef PAN_DEVICES_I2CDEVICE_HPP_
#define PAN_DEVICES_I2CDEVICE_HPP_

/*   The I2CDevice class is intended as an abstract class form which all i2c
 * devices will be derived.
 *   It provides per device i2c fucntionality as opposed to per bus making code
 * and device managment much simpler. Each device specifically has it's own
 * timeout value in milliseconds, i2c bus address, i2c bus reference, and error
 * history state variables.
 *   The class also wraps common i2c_t3 wire function calls to manage the device
 * device specific variables (error tracking, device specific timeouts, etc.).
 * See https://github.com/nox771/i2c_t3 for more information about the teensy
 * i2c library.
 *   The error history state variables operate in the following manner. Each
 * device has two variables tracking errors. One tracks recent history and is a
 * boolean that holds true if an error has occurred since the last call to
 * i2c_pop_errors() and holds false otherwise. The second variable counts
 * consecutive communication failures with the device and is updated with a call
 * to i2c_pop_errors(). i2c_pop_errors should therefore only be called once
 * during a communication with an i2c device (this includes configuration,
 * requesting, and recieving that is needed to obtain data). Otherwise,
 * i2c_peek_errors() can be used to check the recent history of i2c errors.
 *   For consistency, all member functions of this class are prefixed with i2c_.
 */

#include "Device.hpp"

/* Note that I2C_AUTO_RETRY should be enabled - I2CDevice makes no calls to
 * resetBus internally.
 */
#ifdef DESKTOP
typedef unsigned int i2c_stop;
typedef unsigned int i2c_t3;
#define I2C_STOP 0
#define I2C_NOSTOP 0
#else
#include <i2c_t3.h>
#endif

/** \namespace Devices **/
namespace Devices {

/* The number of times an i2c communication can fail before the device is
 * considered not functional. */
#define I2CDEVICE_DISABLE_AT 3
/** \class I2CDevice
 *  @brief Abstract class from which all i2c devices will be derived. **/
class I2CDevice : public Device {
   public:
    /** @brief Attempts to call i2c_ping up to I2CDEVICE_DISABLE_AT times.
     *  @returns true if a succesful ping happened and false otherwise. **/
    bool setup() override;
    /** @brief The device is considered to be functional if less than
     *         I2CDEVICE_DISABLE_AT errors have occurred in a row.
     *  @returns True if device is working properly, false otherwise.. **/
    bool is_functional() override;
    /** @brief Wipes error history variables clean. Must be called from a function
     *         that overrides this. **/
    void reset() override;
    /** @brief Forces error history variables to broken state. Must be called from
     *         a function that overrides this. **/
    void disable() override;
    /** @brief Sets this device's I2C timeout in milliseconds. **/
    inline void i2c_set_timeout(unsigned long i2c_timeout);
    /** @brief  Gets the current value of i2c_timeout in milliseconds.
     *  @return current i2c_timeout in milliseconds **/
    inline unsigned long i2c_get_timeout() const;
    /** @brief Returns whether or not the current data from the device should be
     *         considered valid. For this to be true, the consecutive
     *         communication failure variables must be zero.
     *  @return true if data is valid and false otherwise. **/
    inline bool i2c_data_is_valid() const;

   protected:
    /** @brief Attempts a simple communication with the i2c device - e.g. reading
     *         a register with a known value. Returns true if the proper value is
     *         recieved. i2c related errors are queried in setup.
     *  @returns true if the proper value was read and false otherwise **/
    virtual bool i2c_ping() = 0;
    /** @brief Constructs an i2c device on the specified wire, with the given
     *         address, and a default timeout values of 0 - i.e. a timeout never
     *         happens. **/
    #ifndef DESKTOP
    I2CDevice(const std::string &name, i2c_t3 &wire, unsigned char addr, unsigned long timeout = 0);
    #else
    I2CDevice(const std::string &name, unsigned long timeout = 0);
    #endif
    /** @brief Returns true if an error has occurred since the last call to
     *         pop_errors and false otherwise. The recent error history variable
     *         is reset and the consecutive communication failure variables is
     *         incremented if needed.
     *  @returns true if an error has occurred since the last call to pop_errors
     *           and false otherwise. **/
    inline bool i2c_pop_errors();
    /** @brief Identical functionality to pop_errors except the recent error
     *         history variable isn't reset and the consecutive communication
     *         failure variable isn't incrememnted.
     *  @returns true if an error has occurred since the last call to pop_errors
     *           and false otherwise. **/
    inline bool i2c_peek_errors() const;
    /** @brief Sends a message over i2c to this device. The body of the message is
     *         the array data of type T with length len. Any error will be
     *         recorded in the recent error history variable. This is a blocking
     *         call. **/
    template <typename T>
    #ifndef DESKTOP
    void i2c_transmit_data(T const *data, std::size_t len, i2c_stop s = I2C_STOP);
    #else
    void i2c_transmit_data(T const *data, std::size_t len, i2c_stop s = I2C_STOP);
    #endif
    /** @brief Requests and recieves a message over i2c form this device. The
     *         message will be placed in the array data of type T with length len.
     *         The size of the array data will determine how much data is
     *         requested. Any error will be recorded in the recent error history
     *         variable. This is a blocking call. **/
    template <typename T>
    void i2c_receive_data(T *data, std::size_t len, i2c_stop s = I2C_STOP);
    /** @brief See Wire.beginTransmission in i2c_t3. **/
    inline void i2c_begin_transmission();
    /** @brief See Wire.endTransmission in i2c_t3. Any error will be recorded in
     *         the recent error history variable. **/
    inline void i2c_end_transmission(i2c_stop s = I2C_STOP);
    /** @brief See Wire.sendTransmission in i2c_t3. **/
    inline void i2c_send_transmission(i2c_stop s = I2C_STOP);
    /** @brief See Wire.requestFrom in i2c_t3. Any error will be recorded in the
     *         recent error history variable. **/
    inline void i2c_request_from(std::size_t len, i2c_stop s = I2C_STOP);
    /** @brief Request from a sublocation on a device. Will not generate a stop
     * condition at the end of the request. See i2c_request_from.
     *  @param subaddr Subaddress to read from I2C device.
     *  @param len Number of bytes to request. **/
    inline void i2c_request_from_subaddr(unsigned char subaddr, std::size_t len);
    /** @brief Read multiple bytes from a sublocation on a device.
     *  @param subaddr Subaddress to read from I2C device.
     *  @param dest Location to place the data.
     *  @param len Number of bytes to read from I2C device. **/
    inline void i2c_read_from_subaddr(unsigned char subaddr, unsigned char *dest, std::size_t len);
    /** @brief Read one byte from a sublocation on a device.
     *  @param subaddr Subaddress to read from I2C device. **/
    inline unsigned char i2c_read_from_subaddr(unsigned char subaddr);
    /** @brief Write multiple bytes to subaddress on I2C device.
     *  @param subaddr Subaddress to read from I2C device.
     *  @param data Data to be written to be device.
     *  @param len Number of bytes to write to I2C device. **/
    inline void i2c_write_to_subaddr(unsigned char subaddr, const unsigned char data[],
                                     std::size_t len);
    /** @brief Write one byte to subaddress on I2C device.
     *  @param subaddr Subaddress to read from I2C device.
     *  @param data Byte to be written to device. **/
    inline void i2c_write_to_subaddr(unsigned char subaddr, const unsigned char data);
    /** @brief See Wire.sendRequest in i2c_t3. Any error will be recorded in the
     *         recent error history variable. **/
    inline void i2c_send_request(std::size_t len, i2c_stop s = I2C_STOP);
    /** @brief See Wire.done in i2c_t3.
     *  @returns false if a non-blockign call is still running and true otherwise.
     * **/
    inline bool i2c_done() const;
    /** @brief See Wire.finish in i2c_t3. **/
    inline void i2c_finish();
    /** @brief See Wire.write in i2c_t3. Any error will be recorded in the recent
     *         error history variable. **/
    inline void i2c_write(unsigned char data);
    /** @brief Writes data to the outgoing i2c buffer of an arbitrary type T. The
     *         data pointer points to the head of an array of type T with length
     *         len. **/
    template <typename T>
    inline void i2c_write(T const *data, std::size_t len);
    /** @brief See Wire.available in i2c_t3. **/
    inline uint32_t i2c_available() const;
    /** @brief See Wire.readByte in i2c_t3. Any error will be recorded in the
     *         recent error history variable.
     *  @returns next unsigned char in the incoming i2c buffer. **/
    inline unsigned char i2c_read();
    /** @brief Reads data from the incoming i2c buffer in the specified array of
     *         type T and length len. If the array cannot be filled with the
     *         present available data, an error is recorded.
     * @param data The data array that the data should be read into.
     * @param len The length of the provided data array. **/
    template <typename T>
    inline void i2c_read(T *data, std::size_t len);
    /** @brief See Wire.peekByte in i2c_t3.
     *  @returns next unsigned char in the incoming i2c buffer. **/
    inline unsigned char i2c_peek();

   private:
    #ifndef DESKTOP
        /** Wire associated with this device **/
        i2c_t3 &wire;
        /** I2C address associated with this device **/
        unsigned char const addr;
    #endif
    /** Timeout value associated with this device for wire calls **/
    unsigned long timeout;
    /** Keeps track of consecutive communication errors **/
    uint32_t error_count;
    /** Error history tracker **/
    bool recent_errors;
};
}  // namespace Devices

#include "I2CDevice.inl"

#endif
