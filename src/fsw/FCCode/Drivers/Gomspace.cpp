#include "Gomspace.hpp"
#include "../Devices/I2CDevice.hpp"
#include <cstring>

// Builtins provided by GCC for endian flipping.

#ifndef DESKTOP
#define __bswap_16(x) ((unsigned short int)__builtin_bswap16(x))
#define __bswap_32(x) ((unsigned int)__builtin_bswap32(x))
#endif

using namespace Devices;

#ifdef DESKTOP
#define I2C_INITIALIZATION I2CDevice("gomspace", 10000)
#else
#define I2C_INITIALIZATION I2CDevice("gomspace", Gomspace::wire, Gomspace::address, 10000)
#endif

Gomspace::Gomspace(Gomspace::eps_hk_t *hk_data, Gomspace::eps_config_t *config_data,
                   Gomspace::eps_config2_t *config2_data)
    : I2C_INITIALIZATION,
      hk(hk_data),
      gspace_config(config_data),
      gspace_config2(config2_data)
{
    hk_vi = (eps_hk_vi_t *)((unsigned char *)hk + 0);
    hk_out = (eps_hk_out_t *)((unsigned char *)hk_vi + sizeof(eps_hk_vi_t));
    hk_wdt = (eps_hk_wdt_t *)((unsigned char *)hk_out + sizeof(eps_hk_out_t));
    hk_basic = (eps_hk_basic_t *)((unsigned char *)hk_wdt + sizeof(eps_hk_wdt_t));
}

#undef I2C_INITIALIZATION

bool Gomspace::setup() { return I2CDevice::setup(); }

void Gomspace::reset() {
    I2CDevice::reset();
    reboot();
}

bool Gomspace::i2c_ping() { return ping(0xFF); }

bool Gomspace::get_hk() {
    unsigned char PORT_BYTE = 0x08;
    unsigned char CMD_TYPE_BYTE = 0x00;
    unsigned char command[2] = {PORT_BYTE, CMD_TYPE_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    size_t struct_size = sizeof(eps_hk_t);
    #ifndef DESKTOP
    unsigned char buffer[struct_size + 2];
    #endif

    i2c_request_from((struct_size + 2), I2C_STOP);
    #ifndef DESKTOP
    i2c_read(buffer, struct_size + 2);
    #endif

    // FOR DEBUGGING
    // for(unsigned char i = 0; i < sizeof(buffer); i++) {
    //     Serial.printf("%d ", buffer[i]);
    // }
    // Serial.println();
    i2c_finish();

    #ifndef DESKTOP
    if (buffer[0] != PORT_BYTE || buffer[1] != 0)
        return false;
    else {
        
        memcpy((unsigned char *)hk, buffer + 2, struct_size);
        // Flip endianness of all values
        _hk_vi_endian_flip();
        _hk_out_endian_flip();
        _hk_wdt_endian_flip();
        _hk_basic_endian_flip();
        
        return true;
    }
    #else
    return true;
    #endif
}

bool Gomspace::get_hk_vi() {
    unsigned char PORT_BYTE = 0x08;
    unsigned char CMD_TYPE_BYTE = 0x01;
    unsigned char command[2] = {PORT_BYTE, CMD_TYPE_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    size_t struct_size = sizeof(eps_hk_vi_t);
    #ifndef DESKTOP
    unsigned char buffer[struct_size + 2];
    #endif

    i2c_request_from((struct_size + 2), I2C_STOP);
    #ifndef DESKTOP
    i2c_read(buffer, struct_size + 2);
    #endif

    #ifndef DESKTOP
    if (buffer[1] != 0)
        return false;
    else {
        
        memcpy((unsigned char *)hk_vi, buffer + 2, struct_size);
        _hk_vi_endian_flip();
        return true;
    }
    #else
    return true;
    #endif
}

bool Gomspace::get_hk_out() {
    unsigned char PORT_BYTE = 0x08;
    unsigned char CMD_TYPE_BYTE = 0x02;
    unsigned char command[2] = {PORT_BYTE, CMD_TYPE_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    size_t struct_size = sizeof(eps_hk_out_t);
    #ifndef DESKTOP
    unsigned char buffer[struct_size + 2];
    #endif

    i2c_request_from((struct_size + 2), I2C_STOP);
    #ifndef DESKTOP
    i2c_read(buffer, struct_size + 2);
    #endif

    #ifndef DESKTOP
    if (buffer[1] != 0)
        return false;
    else {
        memcpy((unsigned char *)hk_out, buffer + 2, struct_size);
        _hk_out_endian_flip();
        return true;
    }
    #else
    return true;
    #endif
}

bool Gomspace::get_hk_wdt() {
    unsigned char PORT_BYTE = 0x08;
    unsigned char CMD_TYPE_BYTE = 0x03;
    unsigned char command[2] = {PORT_BYTE, CMD_TYPE_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    size_t struct_size = sizeof(eps_hk_wdt_t);
    #ifndef DESKTOP
    unsigned char buffer[struct_size + 2] = {0};
    #endif

    i2c_request_from((struct_size + 2), I2C_STOP);
    #ifndef DESKTOP
    i2c_read(buffer, struct_size + 2);
    #endif

    #ifndef DESKTOP
    if (buffer[1] != 0)
        return false;
    else {
        memcpy((unsigned char *)hk_wdt, buffer + 2, struct_size);
        _hk_wdt_endian_flip();
        return true;
    }
    #else
    return true;
    #endif
}

bool Gomspace::get_hk_basic() {
    unsigned char PORT_BYTE = 0x08;
    unsigned char CMD_TYPE_BYTE = 0x04;
    unsigned char command[2] = {PORT_BYTE, CMD_TYPE_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    size_t struct_size = sizeof(eps_hk_basic_t);
    #ifndef DESKTOP
    unsigned char buffer[struct_size + 2];
    #endif

    i2c_request_from((struct_size + 2), I2C_STOP);
    #ifndef DESKTOP
    i2c_read(buffer, struct_size + 2);
    #endif

    #ifndef DESKTOP
    if (buffer[1] != 0)
        return false;
    else {
        memcpy((unsigned char *)hk_basic, buffer + 2, struct_size);
        _hk_basic_endian_flip();
        return true;
    }
    #else
    return true;
    #endif
}

bool Gomspace::set_output(unsigned char output_byte) {
    unsigned char PORT_BYTE = 0x09;
    unsigned char command[2] = {PORT_BYTE, output_byte};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    #ifdef DESKTOP
    for (int i=0; i<8; i++){
        hk->output[i]=output_byte;
    }
    return true;
    #endif

    return _check_for_error(PORT_BYTE);
}

bool Gomspace::set_single_output(unsigned char channel, unsigned char value, short int delay) {
    if (channel > 5) return false;  // Disallow turning on heater with this command.
    if (value > 1) return false;    // Precondition check to avoid radiation bit flips
    // All values must be sent in big-endian order!
    #ifndef DESKTOP
    delay = __bswap_16(delay);
    #endif

    unsigned char PORT_BYTE = 0x0A;
    unsigned char command[5] = {PORT_BYTE, channel, value, (unsigned char)(delay >> 8),
                                (unsigned char)(delay)};
    i2c_begin_transmission();
    i2c_write(command, 5);
    i2c_end_transmission(I2C_NOSTOP);

    #ifdef DESKTOP
    hk->output[channel]=value;
    return true;
    #endif

    return _check_for_error(PORT_BYTE);
}

bool Gomspace::set_pv_volt(unsigned short int voltage1, unsigned short int voltage2,
                           unsigned short int voltage3) {
    // All values must be sent in big-endian order!
    #ifndef DESKTOP
    voltage1 = __bswap_16(voltage1);
    voltage2 = __bswap_16(voltage2);
    voltage3 = __bswap_16(voltage3);
    #endif

    unsigned char PORT_BYTE = 0x0B;
    unsigned char command[7] = {PORT_BYTE,
                                (unsigned char)(voltage1 >> 8),
                                (unsigned char)(voltage1),
                                (unsigned char)(voltage2 >> 8),
                                (unsigned char)(voltage2),
                                (unsigned char)(voltage3 >> 8),
                                (unsigned char)(voltage3)};
    i2c_begin_transmission();
    i2c_write(command, 7);
    i2c_end_transmission(I2C_NOSTOP);

    #ifdef DESKTOP
    hk->vboost[0]=voltage1;
    hk->vboost[1]=voltage2;
    hk->vboost[2]=voltage3;
    return true;
    #endif

    return _check_for_error(PORT_BYTE);
}

bool Gomspace::set_pv_auto(unsigned char mode) {
    if (mode > 1) return false;  // Precondition check to avoid radiation bit flips

    unsigned char PORT_BYTE = 0x0C;
    unsigned char command[2] = {PORT_BYTE, mode};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);
    
    #ifdef DESKTOP
    hk->pptmode=mode;
    return true;
    #endif

    return _check_for_error(PORT_BYTE);
}

bool Gomspace::turn_on_heater() { return _set_heater(true); }
bool Gomspace::turn_off_heater() { return _set_heater(false); }

bool Gomspace::_set_heater(bool mode) {
    unsigned char PORT_BYTE = 0x0D, COMMAND = 0x00;
    unsigned char command[4] = {PORT_BYTE, COMMAND, 1, (unsigned char)mode};
    i2c_begin_transmission();
    i2c_write(command, 4);
    i2c_end_transmission(I2C_NOSTOP);

    #ifndef DESKTOP
    unsigned char buffer[4];
    i2c_request_from(4, I2C_STOP);
    i2c_read(buffer, 4);
    if (buffer[0] != PORT_BYTE)
        return false;
    else if (buffer[1] != 0x00)
        return false;
    else if (buffer[3] != mode && 1 == (1 | 2))
        return false;  // If onboard heater was supposed to be turned on/off,
                       // it better have been reported to be turned on/off.
    else
        return true;
    #else
    if (mode==true){
        heater=1;
    }
    else{
        heater=0;
    }
    return true;
    #endif
}

unsigned char Gomspace::get_heater() {
    unsigned char PORT_BYTE = 0x0D;
    unsigned char command[1] = {PORT_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 1);
    i2c_end_transmission(I2C_NOSTOP);

    #ifndef DESKTOP
    unsigned char buffer[4];
    i2c_request_from(4, I2C_STOP);
    i2c_read(buffer, 4);

    if (buffer[0] != PORT_BYTE || buffer[1] != 0)
        return 2;  // If error occurred, return error code.
    // buffer[2] contains 0 or 1, indicating whether BP4 heater is on. We don't
    // care about this value since we don't have a BP4 pack.
    // buffer[3] contains 0 or 1, indicating whether onboard heater is on.
    return buffer[3];
    #else
    return heater;
    #endif
}

bool Gomspace::reset_counters() {
    unsigned char PORT_BYTE = 0x0F;
    unsigned char MAGIC_BYTE = 0x42;
    unsigned char command[2] = {PORT_BYTE, MAGIC_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    #ifdef DESKTOP
    hk->counter_boot=0;
    hk->counter_wdt_i2c=0;
    hk->counter_wdt_gnd=0;
    hk->counter_wdt_csp[0]=0;
    hk->counter_wdt_csp[1]=0;
    return true;
    #endif

    return _check_for_error(PORT_BYTE);
}

bool Gomspace::reset_wdt() {
    unsigned char PORT_BYTE = 0x10;
    unsigned char MAGIC_BYTE = 0x78;
    unsigned char command[2] = {PORT_BYTE, MAGIC_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    return _check_for_error(PORT_BYTE);
}

bool Gomspace::config_get() {
    unsigned char PORT_BYTE = 0x12;
    unsigned char command[1] = {PORT_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 1);
    i2c_end_transmission(I2C_NOSTOP);

    size_t struct_size = sizeof(eps_config_t);
    #ifndef DESKTOP
    unsigned char buffer[struct_size + 2];
    #endif
    i2c_request_from((struct_size + 2), I2C_STOP);
    #ifndef DESKTOP
    i2c_read(buffer, struct_size + 2);
    #endif

    #ifndef DESKTOP
    if (buffer[1] != 0)
        return false;
    else {
        memcpy((unsigned char *)&gspace_config, buffer + 2, struct_size);
        // Flip endianness
        for (unsigned char i = 0; i < 8; i++) {
            gspace_config->output_initial_on_delay[i] =
                __bswap_16(gspace_config->output_initial_on_delay[i]);
            gspace_config->output_initial_off_delay[i] =
                __bswap_16(gspace_config->output_initial_off_delay[i]);
        }
        for (unsigned char i = 0; i < 3; i++) {
            gspace_config->vboost[i] = __bswap_16(gspace_config->vboost[i]);
        }
        return true;
    }
    #else
    return true;
    #endif
}

bool Gomspace::config_set(const eps_config_t &c) {
    unsigned char PORT_BYTE = 0x13;
    unsigned char command[1] = {PORT_BYTE};

    // Flip endianness of all numbers in struct
    unsigned char config_struct[sizeof(eps_config_t)];
    #ifndef DESKTOP
    memcpy(config_struct, (unsigned char *)&c, sizeof(eps_config_t));
    eps_config_t *config_struct_ptr = (eps_config_t *)config_struct;
    for (unsigned char i = 0; i < 8; i++) {
        config_struct_ptr->output_initial_on_delay[i] =
            __bswap_16(config_struct_ptr->output_initial_on_delay[i]);
        config_struct_ptr->output_initial_off_delay[i] =
            __bswap_16(config_struct_ptr->output_initial_off_delay[i]);
    }
    for (unsigned char i = 0; i < 3; i++) {
        config_struct_ptr->vboost[i] = __bswap_16(config_struct_ptr->vboost[i]);
    }
    #endif

    i2c_begin_transmission();
    i2c_write(command, 1);
    i2c_write(config_struct, sizeof(eps_config_t));
    i2c_end_transmission(I2C_NOSTOP);

    return _check_for_error(PORT_BYTE);
}

bool Gomspace::hard_reset() {
    unsigned char PORT_BYTE = 0x14;
    unsigned char command[1] = {PORT_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 1);
    i2c_end_transmission(I2C_NOSTOP);

    #ifdef DESKTOP
    unsigned char mode=1;
    hk->pptmode=mode;
    #endif

    return _check_for_error(PORT_BYTE);
}

bool Gomspace::config2_get() {
    unsigned char PORT_BYTE = 0x16;
    unsigned char command[1] = {PORT_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 1);
    i2c_end_transmission(I2C_NOSTOP);

    size_t struct_size = sizeof(eps_config2_t);
    #ifndef DESKTOP
    unsigned char buffer[struct_size + 2];
    #endif
    i2c_request_from((struct_size + 2), I2C_STOP);
    #ifndef DESKTOP
    i2c_read(buffer, struct_size + 2);

    if (buffer[1] != 0)
        return false;
    else {
        memcpy((unsigned char *)&gspace_config2, buffer + 2, struct_size);
        // Flip endianness
        gspace_config2->batt_maxvoltage = __bswap_16(gspace_config2->batt_maxvoltage);
        gspace_config2->batt_safevoltage = __bswap_16(gspace_config2->batt_safevoltage);
        gspace_config2->batt_criticalvoltage = __bswap_16(gspace_config2->batt_criticalvoltage);
        gspace_config2->batt_normalvoltage = __bswap_16(gspace_config2->batt_normalvoltage);
        return true;
    }
    #else
    return true;
    #endif
}

bool Gomspace::restore_default_config2() {
    unsigned char PORT_BYTE = 0x15;
    unsigned char COMMAND_BYTE = 0x02;
    unsigned char command[2] = {PORT_BYTE, COMMAND_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    if (!_check_for_error(PORT_BYTE)) return false;
    if (!_config2_confirm()) return false;
    return true;
}

bool Gomspace::config2_set(const eps_config2_t &c) {
    unsigned char PORT_BYTE = 0x17;
    unsigned char command[1] = {PORT_BYTE};

    // Flip endianness of all numbers in struct
    unsigned char config2_struct[sizeof(eps_config2_t)];
    #ifndef DESKTOP
    memcpy(config2_struct, (unsigned char *)&c, sizeof(eps_config2_t));
    eps_config2_t *config2_struct_ptr = (eps_config2_t *)config2_struct;
    config2_struct_ptr->batt_criticalvoltage = __bswap_16(config2_struct_ptr->batt_criticalvoltage);
    config2_struct_ptr->batt_maxvoltage = __bswap_16(config2_struct_ptr->batt_maxvoltage);
    config2_struct_ptr->batt_safevoltage = __bswap_16(config2_struct_ptr->batt_safevoltage);
    config2_struct_ptr->batt_normalvoltage = __bswap_16(config2_struct_ptr->batt_normalvoltage);
    #endif

    i2c_begin_transmission();
    i2c_write(command, 1);
    i2c_write(config2_struct, sizeof(eps_config2_t));
    i2c_end_transmission(I2C_NOSTOP);

    if (!_check_for_error(PORT_BYTE)) return false;
    if (!_config2_confirm()) return false;
    return true;
}

bool Gomspace::_config2_confirm() {
    unsigned char PORT_BYTE = 0x15;
    unsigned char COMMAND_BYTE = 0x02;
    unsigned char command[2] = {PORT_BYTE, COMMAND_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    return _check_for_error(PORT_BYTE);
}

bool Gomspace::ping(unsigned char value) {
    unsigned char PORT_BYTE = 0x01;
    unsigned char command[2] = {PORT_BYTE, value};
    i2c_begin_transmission();
    i2c_write(command, 2);
    i2c_end_transmission(I2C_NOSTOP);

    #ifndef DESKTOP
    unsigned char buffer[3];
    #endif
    i2c_request_from(3, I2C_STOP);
    #ifndef DESKTOP
    i2c_read(buffer, 3);

    return (buffer[1] == 0) && (value == buffer[2]);
    #else
    return true;
    #endif
}

void Gomspace::reboot() {
    unsigned char PORT_BYTE = 0x04;
    unsigned char MAGIC[4] = {0x80, 0x07, 0x80, 0x07};
    unsigned char command[1] = {PORT_BYTE};
    i2c_begin_transmission();
    i2c_write(command, 1);
    i2c_write(MAGIC, 4);
    i2c_end_transmission(I2C_STOP);
    #ifdef DESKTOP
    hk->counter_boot=hk->counter_boot+1;
    #endif
}

bool Gomspace::_check_for_error(unsigned char port_byte) {
    #ifndef DESKTOP
    unsigned char buffer[2];
    #endif
    i2c_request_from(2, I2C_STOP);
    #ifndef DESKTOP
    i2c_read(buffer, 2);

    if (buffer[0] == port_byte && buffer[1] == 0) return true;
    return false;
    #else
    return true;
    #endif
    
}

void Gomspace::_hk_vi_endian_flip() {
    #ifndef DESKTOP
    for (unsigned char i = 0; i < 3; i++) {
        hk_vi->vboost[i] = __bswap_16(hk_vi->vboost[i]);
        hk_vi->curin[i] = __bswap_16(hk_vi->curin[i]);
    }
    hk_vi->vbatt = __bswap_16(hk_vi->vbatt);
    hk_vi->cursun = __bswap_16(hk_vi->cursun);
    hk_vi->cursys = __bswap_16(hk_vi->cursys);
    #endif
}

void Gomspace::_hk_out_endian_flip() {
    #ifndef DESKTOP
    for (unsigned char i = 0; i < 6; i++) {
        hk_out->curout[i] = __bswap_16(hk_out->curout[i]);
        hk_out->latchup[i] = __bswap_16(hk_out->latchup[i]);
    }
    for (unsigned char i = 0; i < 8; i++) {
        hk_out->output_on_delta[i] = __bswap_16(hk_out->output_on_delta[i]);
        hk_out->output_off_delta[i] = __bswap_16(hk_out->output_off_delta[i]);
    }
    #endif
}

void Gomspace::_hk_wdt_endian_flip() {
    #ifndef DESKTOP
    hk_wdt->wdt_i2c_time_left = __bswap_32(hk_wdt->wdt_i2c_time_left);
    hk_wdt->wdt_gnd_time_left = __bswap_32(hk_wdt->wdt_gnd_time_left);
    hk_wdt->counter_wdt_i2c = __bswap_32(hk_wdt->counter_wdt_i2c);
    hk_wdt->counter_wdt_gnd = __bswap_32(hk_wdt->counter_wdt_gnd);
    hk_wdt->counter_wdt_csp[0] = __bswap_32(hk_wdt->counter_wdt_csp[0]);
    hk_wdt->counter_wdt_csp[1] = __bswap_32(hk_wdt->counter_wdt_csp[1]);
    #endif
}

void Gomspace::_hk_basic_endian_flip() {
    #ifndef DESKTOP
    for (unsigned char i = 0; i < 6; i++) {
        hk_basic->temp[i] = __bswap_16(hk_basic->temp[i]);
    }
    hk_basic->counter_boot = __bswap_16(hk_basic->counter_boot);
    #endif
}
