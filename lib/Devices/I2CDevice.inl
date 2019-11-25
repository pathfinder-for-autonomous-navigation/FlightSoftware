/** @file I2CDevice.cpp
 * @author Tanishq Aggarwal
 * @author Kyle Krol
 * @date 6 Feb 2018
 * @brief Contains implementation for I2CDevice interface, which standardizes the access
 * and control of I2C-based hardware peripherals.
 */

namespace Devices {
inline void I2CDevice::i2c_set_timeout(unsigned long i2c_timeout) {
    this->timeout = i2c_timeout * 1000;
}

inline unsigned long I2CDevice::i2c_get_timeout() const { return this->timeout / 1000; }

inline bool I2CDevice::i2c_data_is_valid() const { return this->error_count == 0; }

inline bool I2CDevice::i2c_pop_errors() {
    bool temp = this->recent_errors;
    this->recent_errors = false;
    return temp;
}

inline bool I2CDevice::i2c_peek_errors() const { return this->recent_errors; }

template <typename T>
void I2CDevice::i2c_transmit_data(T const *data, std::size_t len, i2c_stop s) {
#ifndef DESKTOP
    this->i2c_begin_transmission();
    this->i2c_write(data, len);
    this->i2c_end_transmission(s);
#endif
}

template <typename T>
void I2CDevice::i2c_receive_data(T *data, std::size_t len, i2c_stop s) {
#ifndef DESKTOP
    this->i2c_request_from(len * sizeof(T), s);
    if (this->i2c_peek_errors()) return;
    for (std::size_t i = 0; i < len * sizeof(T); i++) ((unsigned char *)data)[i] = this->i2c_read();
#endif
}

inline void I2CDevice::i2c_begin_transmission() {
#ifndef DESKTOP
    this->wire.beginTransmission(this->addr);
#endif
}

inline void I2CDevice::i2c_end_transmission(i2c_stop s) {
#ifndef DESKTOP
    bool err = (this->wire.endTransmission(s, this->timeout) != 0);
    this->recent_errors = (this->recent_errors || err);
#endif
}

inline void I2CDevice::i2c_send_transmission(i2c_stop s) {
#ifndef DESKTOP
    this->wire.sendTransmission(s);
#endif
}

inline void I2CDevice::i2c_request_from(std::size_t len, i2c_stop s) {
#ifndef DESKTOP
    bool err = (this->wire.requestFrom(this->addr, len, s, this->timeout) == 0);
    this->recent_errors = (this->recent_errors || err);
#endif
}

inline void I2CDevice::i2c_request_from_subaddr(unsigned char subaddr, std::size_t len) {
#ifndef DESKTOP
    i2c_begin_transmission();
    i2c_write(subaddr);
    i2c_end_transmission();
    i2c_request_from(len, I2C_NOSTOP);
#endif
}

inline void I2CDevice::i2c_read_from_subaddr(unsigned char subaddr, unsigned char *dest,
                                             std::size_t len) {
#ifndef DESKTOP
    i2c_request_from_subaddr(subaddr, len);
    i2c_read(dest, len);
    i2c_finish();
#endif
}

inline unsigned char I2CDevice::i2c_read_from_subaddr(unsigned char subaddr) {
    unsigned char byte = 0;
#ifndef DESKTOP
    i2c_read_from_subaddr(subaddr, &byte, 1);
#endif
    return byte;
}

inline void I2CDevice::i2c_write_to_subaddr(unsigned char subaddr, const unsigned char data[],
                                            std::size_t len) {
#ifndef DESKTOP
    i2c_begin_transmission();
    i2c_write(subaddr);
    i2c_write(data, len);
    i2c_end_transmission();
#endif
}

inline void I2CDevice::i2c_write_to_subaddr(unsigned char subaddr, const unsigned char data) {
#ifndef DESKTOP
    unsigned char bytes[] = {data};
    i2c_write_to_subaddr(subaddr, bytes, 1);
#endif
}

inline void I2CDevice::i2c_send_request(std::size_t len, i2c_stop s) {
#ifndef DESKTOP
    this->wire.sendRequest(this->addr, len, s);
#endif
}

inline bool I2CDevice::i2c_done() const {
#ifdef DESKTOP
    return true;
#else
    return (this->wire.done() == 1);
#endif
}

inline void I2CDevice::i2c_finish() {
#ifndef DESKTOP
    bool err = (this->wire.finish(this->timeout) == 0);
    this->recent_errors = (this->recent_errors || err);
#endif
}

inline void I2CDevice::i2c_write(unsigned char data) {
#ifndef DESKTOP
    bool err = (this->wire.write(data) == 0);
    this->recent_errors = (this->recent_errors || err);
#endif
}

template <typename T>
inline void I2CDevice::i2c_write(T const *data, std::size_t len) {
#ifndef DESKTOP
    bool err = (this->wire.write((unsigned char *)data, len * sizeof(T)) == 0);
    this->recent_errors = (this->recent_errors || err);
#endif
}

inline uint32_t I2CDevice::i2c_available() const {
#ifdef DESKTOP
    return 0;
#else
    return this->wire.available();
#endif
}

inline unsigned char I2CDevice::i2c_read() {
#ifdef DESKTOP
    int val = 0;
#else
    int val = this->wire.read();
#endif

    bool err = (val == -1);
    this->recent_errors = (this->recent_errors || err);
    return (unsigned char)(val & 0xFF);
}

template <typename T>
inline void I2CDevice::i2c_read(T *data, std::size_t len) {
#ifndef DESKTOP
    bool err = (this->wire.read((unsigned char *)data, len * sizeof(T)) != len * sizeof(T));
    this->recent_errors = (this->recent_errors || err);
#endif
}

inline unsigned char I2CDevice::i2c_peek() {
#ifdef DESKTOP
    int val = 0;
#else
    int val = this->wire.peek();
#endif

    bool err = (val == -1);
    this->recent_errors = (this->recent_errors || err);
    return (unsigned char)(val & 0xFF);
}
}  // namespace Devices
