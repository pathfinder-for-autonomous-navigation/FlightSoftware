#include "Piksi.hpp"

using namespace Devices;

// Initialize callback nodes so that C++ doesn't complain about them not being
// defined.
sbp_msg_callbacks_node_t Piksi::_startup_callback_node;
sbp_msg_callbacks_node_t Piksi::_log_callback_node;
sbp_msg_callbacks_node_t Piksi::_gps_time_callback_node;
sbp_msg_callbacks_node_t Piksi::_dops_callback_node;
sbp_msg_callbacks_node_t Piksi::_pos_ecef_callback_node;
sbp_msg_callbacks_node_t Piksi::_baseline_ecef_callback_node;
sbp_msg_callbacks_node_t Piksi::_vel_ecef_callback_node;
sbp_msg_callbacks_node_t Piksi::_base_pos_ecef_callback_node;
sbp_msg_callbacks_node_t Piksi::_iar_callback_node;
sbp_msg_callbacks_node_t Piksi::_settings_read_resp_callback_node;
sbp_msg_callbacks_node_t Piksi::_heartbeat_callback_node;
sbp_msg_callbacks_node_t Piksi::_uart_state_callback_node;
sbp_msg_callbacks_node_t Piksi::_user_data_callback_node;

#ifndef DESKTOP
Piksi::Piksi(const std::string &name, HardwareSerial &serial_port)
    : Device(name), _serial_port(serial_port) {}
#else
Piksi::Piksi(const std::string &name) {}
    //: Device(name) {}
//Piksi::Piksi();
#endif


bool Piksi::setup() {
    #ifndef DESKTOP
    _serial_port.begin(BAUD_RATE);
    #endif

    clear_log();
    _heartbeat.flags = 1;  // By default, let there be an error in the system.

    sbp_state_init(&_sbp_state);
    sbp_state_set_io_context(&_sbp_state, this);

    // Register all necessary callbacks for data reads--specification provided in
    // sbp.c
    unsigned char registration_successful = 0;
    registration_successful |= sbp_register_callback(
        &_sbp_state, SBP_MSG_LOG, &Piksi::_log_callback, this, &Piksi::_log_callback_node);
    registration_successful |=
        sbp_register_callback(&_sbp_state, SBP_MSG_GPS_TIME, &Piksi::_gps_time_callback, this,
                              &Piksi::_gps_time_callback_node);
    registration_successful |= sbp_register_callback(
        &_sbp_state, SBP_MSG_DOPS, &Piksi::_dops_callback, this, &Piksi::_dops_callback_node);
    registration_successful |=
        sbp_register_callback(&_sbp_state, SBP_MSG_POS_ECEF, &Piksi::_pos_ecef_callback, this,
                              &Piksi::_pos_ecef_callback_node);
    registration_successful |=
        sbp_register_callback(&_sbp_state, SBP_MSG_BASELINE_ECEF, &Piksi::_baseline_ecef_callback,
                              this, &Piksi::_baseline_ecef_callback_node);
    registration_successful |=
        sbp_register_callback(&_sbp_state, SBP_MSG_VEL_ECEF, &Piksi::_vel_ecef_callback, this,
                              &Piksi::_vel_ecef_callback_node);
    registration_successful |=
        sbp_register_callback(&_sbp_state, SBP_MSG_BASE_POS_ECEF, &Piksi::_base_pos_ecef_callback,
                              this, &Piksi::_base_pos_ecef_callback_node);
    registration_successful |= sbp_register_callback(
        &_sbp_state, SBP_MSG_IAR_STATE, &Piksi::_iar_callback, this, &Piksi::_iar_callback_node);
    registration_successful |= sbp_register_callback(&_sbp_state, SBP_MSG_SETTINGS_READ_RESP,
                                                     &Piksi::_settings_read_resp_callback, this,
                                                     &Piksi::_settings_read_resp_callback_node);
    registration_successful |=
        sbp_register_callback(&_sbp_state, SBP_MSG_HEARTBEAT, &Piksi::_heartbeat_callback, this,
                              &Piksi::_heartbeat_callback_node);
    registration_successful |=
        sbp_register_callback(&_sbp_state, SBP_MSG_STARTUP, &Piksi::_startup_callback, this,
                              &Piksi::_startup_callback_node);
    registration_successful |=
        sbp_register_callback(&_sbp_state, SBP_MSG_UART_STATE, &Piksi::_uart_state_callback, this,
                              &Piksi::_uart_state_callback_node);
    registration_successful |=
        sbp_register_callback(&_sbp_state, SBP_MSG_USER_DATA, &Piksi::_user_data_callback, this,
                              &Piksi::_user_data_callback_node);

    return (registration_successful == 0);
}

void Piksi::write_default_settings() {
    // Output controls.
    const char *gpgll_output_rate = "nmea\000gpgll msg rate\0000";
    const char *gpgsv_output_rate = "nmea\000gpgsv msg rate\0000";
    const char *gprmc_output_rate = "nmea\000gprmc msg rate\0000";
    const char *gpvtg_output_rate = "nmea\000gpvtg msg rate\0000";
    const char *obs_msg_max_size = "sbp\000obs msg max size\000102";  // This is a default specified
                                                                      // by SwiftNav
    const char *obs_output_rate = "solution\000output every n obs\0001";
    const char *soln_frequency =
        "solution\000soln freq\00010";  // This is a default specified by SwiftNav
    const char *broadcast_surveyed_position = "surveyed position\000broadcast\000true";
    // RTK solution settings.
    const char *dgnss_solution_mode = "solution\000dgnss solution mode\000TimeMatched";
    // Hardware watchdog.
    const char *enable_hardware_watchdog = "system monitor\000watchdog\000true";
    // Heartbeat output rate.
    const char *heartbeat_period = "system monitor\000heartbeat period milliseconds\00010000";
    // Telemetry radio settings
    const char *telemetry_settings =
        "telemetry radio\000configuration "
        "string\000AT&F,ATS1=57,ATS2=64,ATS5=0,AT&W,"
        "ATZ";

    // UART A (radio) settings.
    const char *uart_a_output_mask =
        "uart uarta\000sbp message mask\0000x0840";  // MSG_OBS | MSG_USER_DATA
    const char *uart_a_configure_radio_on_boot =
        "uart uarta\000configure telemetry radio on boot\000true";
    const char *uart_a_baudrate = "uart uarta\000baudrate\00057600";  // This is a default specified
                                                                      // by SwiftNav

    // UART B (controller) output mask.
    const char *uart_b_output_mask =
        "uart uarta\000sbp message mask\0000xffff";  // Every message is passed to
                                                     // controller
    const char *uart_b_configure_radio_on_boot =
        "uart uarta\000configure telemetry radio on boot\000false";
    const char *uart_b_baudrate = "uart uarta\000baudrate\000115200";  // This is a default
                                                                       // specified by SwiftNav

    // Now write the settings to the device!
    const char *settings[] = {gpgll_output_rate,
                              gpgsv_output_rate,
                              gprmc_output_rate,
                              gpvtg_output_rate,
                              obs_msg_max_size,
                              obs_output_rate,
                              soln_frequency,
                              broadcast_surveyed_position,
                              dgnss_solution_mode,
                              enable_hardware_watchdog,
                              heartbeat_period,
                              telemetry_settings,
                              uart_a_output_mask,
                              uart_a_configure_radio_on_boot,
                              uart_a_baudrate,
                              uart_b_output_mask,
                              uart_b_configure_radio_on_boot,
                              uart_b_baudrate};
    for (unsigned char i = 0; i < sizeof(settings) / sizeof(std::string); i++) {
        msg_settings_write_t setting;
        memcpy(setting.setting, settings[i], sizeof(settings[i]) / sizeof(char));
        settings_write(setting);
    }
}

bool Piksi::is_functional() {
    return is_system_healthy() && is_system_io_healthy() && is_swiftnap_healthy() &&
           is_antenna_healthy();
}

void Piksi::reset() { piksi_reset(); }

void Piksi::disable() {
    // Do nothing; we really don't want to disable Piksi
}

void Piksi::get_gps_time(msg_gps_time_t *time) {
    time->wn = _gps_time.wn;
    time->tow = _gps_time.tow;
}

unsigned int Piksi::get_dops_tow() { return _dops.tow; }
unsigned short int Piksi::get_dops_geometric() { return _dops.gdop; }
unsigned short int Piksi::get_dops_position() { return _dops.pdop; }
unsigned short int Piksi::get_dops_time() { return _dops.tdop; }
unsigned short int Piksi::get_dops_horizontal() { return _dops.hdop; }
unsigned short int Piksi::get_dops_vertical() { return _dops.vdop; }

void Piksi::get_pos_ecef(std::array<double, 3> *position) {
    (*position)[0] = _pos_ecef.x;
    (*position)[1] = _pos_ecef.y;
    (*position)[2] = _pos_ecef.z;
}

void Piksi::get_pos_ecef(unsigned int *tow, std::array<double, 3> *position) {
    (*tow) = _pos_ecef.tow;
    (*position)[0] = _pos_ecef.x;
    (*position)[1] = _pos_ecef.y;
    (*position)[2] = _pos_ecef.z;
}

unsigned char Piksi::get_pos_ecef_nsats() { return _pos_ecef.n_sats; }
unsigned char Piksi::get_pos_ecef_flags() { return _pos_ecef.flags; }

void Piksi::get_baseline_ecef(std::array<double, 3> *position) {
    (*position)[0] = _baseline_ecef.x;
    (*position)[1] = _baseline_ecef.y;
    (*position)[2] = _baseline_ecef.z;
}

void Piksi::get_baseline_ecef(unsigned int *tow, std::array<double, 3> *position) {
    *tow = _baseline_ecef.tow;
    (*position)[0] = _baseline_ecef.x;
    (*position)[1] = _baseline_ecef.y;
    (*position)[2] = _baseline_ecef.z;
}

unsigned char Piksi::get_baseline_ecef_nsats() { return _baseline_ecef.n_sats; }
unsigned char Piksi::get_baseline_ecef_flags() { return _baseline_ecef.flags; }

void Piksi::get_vel_ecef(std::array<double, 3> *velocity) {
    (*velocity)[0] = _vel_ecef.x;
    (*velocity)[1] = _vel_ecef.y;
    (*velocity)[2] = _vel_ecef.z;
}

void Piksi::get_vel_ecef(unsigned int *tow, std::array<double, 3> *velocity) {
    *tow = _vel_ecef.tow;
    (*velocity)[0] = _vel_ecef.x;
    (*velocity)[1] = _vel_ecef.y;
    (*velocity)[2] = _vel_ecef.z;
}

unsigned char Piksi::get_vel_ecef_nsats() { return _vel_ecef.n_sats; }
unsigned char Piksi::get_vel_ecef_flags() { return _vel_ecef.flags; }

void Piksi::get_base_pos_ecef(std::array<double, 3> *position) {
    (*position)[0] = _pos_ecef.x;
    (*position)[1] = _pos_ecef.y;
    (*position)[2] = _pos_ecef.z;
}

unsigned int Piksi::get_iar() { return _iar.num_hyps; }

char *Piksi::get_settings_read_resp() { return _settings_read_resp.setting; }

unsigned int Piksi::get_heartbeat() { return _heartbeat.flags; }
bool Piksi::is_system_healthy() { return !(_heartbeat.flags & 0x0001); }
bool Piksi::is_system_io_healthy() { return !(_heartbeat.flags & 0x0002); }
bool Piksi::is_swiftnap_healthy() { return !(_heartbeat.flags & 0x0003); }
bool Piksi::is_antenna_healthy() { return !(_heartbeat.flags & 0x0004); }

float Piksi::get_uart_a_tx_throughput() { return _uart_state.uart_a.tx_throughput; }
float Piksi::get_uart_a_rx_throughput() { return _uart_state.uart_a.rx_throughput; }
unsigned short int Piksi::get_uart_a_crc_error_count() {
    return _uart_state.uart_a.crc_error_count;
}
unsigned short int Piksi::get_uart_a_io_error_count() { return _uart_state.uart_a.io_error_count; }
unsigned char Piksi::get_uart_a_tx_buffer_utilization() {
    return _uart_state.uart_a.tx_buffer_level;
}
unsigned char Piksi::get_uart_a_rx_buffer_utilization() {
    return _uart_state.uart_a.rx_buffer_level;
}

float Piksi::get_uart_b_tx_throughput() { return _uart_state.uart_b.tx_throughput; }
float Piksi::get_uart_b_rx_throughput() { return _uart_state.uart_b.rx_throughput; }
unsigned short int Piksi::get_uart_b_crc_error_count() {
    return _uart_state.uart_b.crc_error_count;
}
unsigned short int Piksi::get_uart_b_io_error_count() { return _uart_state.uart_b.io_error_count; }
unsigned char Piksi::get_uart_b_tx_buffer_utilization() {
    return _uart_state.uart_b.tx_buffer_level;
}
unsigned char Piksi::get_uart_b_rx_buffer_utilization() {
    return _uart_state.uart_b.rx_buffer_level;
}

char *Piksi::get_user_data() { return (char *)_user_data.contents; }

void Piksi::settings_save() {
    sbp_send_message(&_sbp_state, SBP_MSG_SETTINGS_SAVE, SBP_SENDER_ID, 0, nullptr,
                     &Piksi::_uart_write);
}
void Piksi::settings_write(const msg_settings_write_t &settings) {
    sbp_send_message(&_sbp_state, SBP_MSG_SETTINGS_WRITE, SBP_SENDER_ID,
                     sizeof(msg_settings_write_t), (unsigned char *)&settings, &Piksi::_uart_write);
}
void Piksi::piksi_reset() {
    sbp_send_message(&_sbp_state, SBP_MSG_RESET, SBP_SENDER_ID, 0, nullptr, &Piksi::_uart_write);
}
void Piksi::send_user_data(const msg_user_data_t &data) {
    sbp_send_message(&_sbp_state, SBP_MSG_USER_DATA, SBP_SENDER_ID, sizeof(msg_user_data_t),
                     (unsigned char *)&data, &Piksi::_uart_write);
}

signed char Piksi::process_buffer() {
    signed char status = ((signed char)sbp_process(&_sbp_state, Piksi::_uart_read));
    return status;
}

unsigned char Piksi::process_buffer_msg_len() {
    unsigned char pre = _sbp_state.msg_len;
    signed char status = ((signed char)sbp_process(&_sbp_state, Piksi::_uart_read));

    if (status == SBP_OK_CALLBACK_EXECUTED || status == SBP_OK_CALLBACK_UNDEFINED)
        return pre;

    return 0;
}

unsigned char Piksi::read_buffer() {
    int initial_bytes = bytes_available();

    if (initial_bytes == 299 || initial_bytes == 333) {
        int msg_len_sum = 0;
        while (bytes_available()) {
            msg_len_sum += process_buffer_msg_len();
        }

        if (initial_bytes == 299 && msg_len_sum != 227) {
            return 2;
        } else if (initial_bytes == 333 && msg_len_sum != 245) {
            return 2;
        } else
            return 0;

    }

    else {
        while (bytes_available()) {
            clear_bytes();
        }
        return 1;
    }
}

u32 Piksi::bytes_available() { 
    #ifndef DESKTOP
    return _serial_port.available(); 
    #endif
    return 0;
}
void Piksi::clear_bytes() { 
    #ifndef DESKTOP
    _serial_port.clear(); 
    #endif
    }
u32 Piksi::_uart_read(u8 *buff, u32 n, void *context) {
    #ifndef DESKTOP
    Piksi *piksi = (Piksi *)context;
    
    HardwareSerial &sp = piksi->_serial_port;

    u32 i;
    for (i = 0; i < n; i++) {
        if (sp.available())
            buff[i] = sp.read();
        else
            break;
    }
    return i;
    #else
    return 0;
    #endif
}

u32 Piksi::_uart_write(u8 *buff, u32 n, void *context) {
    #ifndef DESKTOP

    Piksi *piksi = (Piksi *)context;
    HardwareSerial &sp = piksi->_serial_port;
    u32 i;
    for (i = 0; i < n; i++) {
        if (sp.write(buff[i]) == 0) break;
    }
    return i;
    #else
    return 0;
    #endif
}

void Piksi::_insert_log_msg(u8 msg[]) {
    _latest_log++;
    if (_latest_log >= &_logbook[0] + _logbook_max_size) {
        _latest_log = &_logbook[0];
    }
    memcpy(_latest_log, msg, sizeof(msg_log_t));
    if (_logbook_size < _logbook_max_size) _logbook_size++;
}
void Piksi::dump_log(char *destination) {
    memcpy(destination, (char *)_logbook, _logbook_size * sizeof(msg_log_t));
}
void Piksi::clear_log() {
    _latest_log = &_logbook[0] - 1;
    _logbook_size = 0;
}

void Piksi::_log_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    piksi->_insert_log_msg(msg);
}
void Piksi::_gps_time_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_gps_time)), msg, sizeof(msg_gps_time_t));
}
void Piksi::_dops_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_dops)), msg, sizeof(msg_dops_t));
}
void Piksi::_pos_ecef_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_pos_ecef)), msg, sizeof(msg_pos_ecef_t));
}
void Piksi::_baseline_ecef_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_baseline_ecef)), msg, sizeof(msg_baseline_ecef_t));
}
void Piksi::_vel_ecef_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_vel_ecef)), msg, sizeof(msg_vel_ecef_t));
}
void Piksi::_base_pos_ecef_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_base_pos_ecef)), msg, sizeof(msg_base_pos_ecef_t));
}
void Piksi::_iar_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_iar)), msg, sizeof(msg_iar_state_t));
}
void Piksi::_settings_read_resp_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_base_pos_ecef)), msg, sizeof(msg_settings_read_resp_t));
}
void Piksi::_heartbeat_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_heartbeat)), msg, sizeof(msg_heartbeat_t));
}
void Piksi::_startup_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_startup)), msg, sizeof(msg_startup_t));
}
void Piksi::_uart_state_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_uart_state)), msg, sizeof(msg_uart_state_t));
}
void Piksi::_user_data_callback(u16 sender_id, u8 len, u8 msg[], void *context) {
    Piksi *piksi = (Piksi *)context;
    memcpy((u8 *)(&(piksi->_user_data)), msg, sizeof(msg_user_data_t));
}