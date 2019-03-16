#include "master_helpers.hpp"
#include "../../state/state_holder.hpp"
#include "../../state/device_states.hpp"
#include "../constants.hpp"

void Master::apply_uplink_constants(const Comms::Uplink& uplink) {
  std::array<unsigned int, 5> constant_ids {
     (unsigned int) uplink.constant_0_id,
     (unsigned int) uplink.constant_1_id,
     (unsigned int) uplink.constant_2_id,
     (unsigned int) uplink.constant_3_id,
     (unsigned int) uplink.constant_4_id,
  };
  std::array<unsigned int, 5> constant_vals {
     (unsigned int) uplink.constant_0_val,
     (unsigned int) uplink.constant_1_val,
     (unsigned int) uplink.constant_2_val,
     (unsigned int) uplink.constant_3_val,
     (unsigned int) uplink.constant_4_val,
  };
  for(int i = 0; i < constant_ids.size(); i++) {
    unsigned int const_id = constant_ids[i];
    if (const_id > Constants::changeable_constants_map.size()) continue;
    rwMtxWLock(&Constants::changeable_constants_lock);
    *(Constants::changeable_constants_map[const_id]) = constant_vals[i];
    rwMtxWUnlock(&Constants::changeable_constants_lock);
  }
}

void Master::apply_uplink_adcs_hat(const Comms::Uplink& uplink) {
  rwMtxWLock(&State::ADCS::adcs_state_lock);
    State::ADCS::adcs_hat.at("gyroscope").is_functional = uplink.adcs_hat_gyroscope;
    State::ADCS::adcs_hat.at("magnetometer").is_functional = uplink.adcs_hat_magnetometer;
    State::ADCS::adcs_hat.at("magnetorquer_x").is_functional = uplink.adcs_hat_magnetorquer_x;
    State::ADCS::adcs_hat.at("magnetorquer_y").is_functional = uplink.adcs_hat_magnetorquer_y;
    State::ADCS::adcs_hat.at("magnetorquer_z").is_functional = uplink.adcs_hat_magnetorquer_z;
    State::ADCS::adcs_hat.at("motorpot").is_functional = uplink.adcs_hat_motorpot;
    State::ADCS::adcs_hat.at("motor_x").is_functional = uplink.adcs_hat_motor_x;
    State::ADCS::adcs_hat.at("motor_y").is_functional = uplink.adcs_hat_motor_y;
    State::ADCS::adcs_hat.at("motor_z").is_functional = uplink.adcs_hat_motor_z;
    State::ADCS::adcs_hat.at("motor_x_adc").is_functional = uplink.adcs_hat_motor_x_adc;
    State::ADCS::adcs_hat.at("motor_y_adc").is_functional = uplink.adcs_hat_motor_y_adc;
    State::ADCS::adcs_hat.at("motor_z_adc").is_functional = uplink.adcs_hat_motor_z_adc;
    State::ADCS::adcs_hat.at("ssa_adc_1").is_functional = uplink.adcs_hat_ssa_adc_1;
    State::ADCS::adcs_hat.at("ssa_adc_2").is_functional = uplink.adcs_hat_ssa_adc_2;
    State::ADCS::adcs_hat.at("ssa_adc_3").is_functional = uplink.adcs_hat_ssa_adc_3;
    State::ADCS::adcs_hat.at("ssa_adc_4").is_functional = uplink.adcs_hat_ssa_adc_4;
    State::ADCS::adcs_hat.at("ssa_adc_5").is_functional = uplink.adcs_hat_ssa_adc_5;
  rwMtxWUnlock(&State::ADCS::adcs_state_lock);
}

void Master::apply_uplink_fc_hat(const Comms::Uplink& uplink) {
  rwMtxWLock(&State::Hardware::hardware_state_lock);
    State::Hardware::hat.at(&Devices::gomspace()).error_ignored = uplink.fc_hat_gomspace;
    State::Hardware::hat.at(&Devices::piksi()).error_ignored = uplink.fc_hat_piksi;
    State::Hardware::hat.at(&Devices::quake()).error_ignored = uplink.fc_hat_quake;
    State::Hardware::hat.at(&Devices::dcdc()).error_ignored = uplink.fc_hat_dcdc;
    State::Hardware::hat.at(&Devices::spike_and_hold()).error_ignored = uplink.fc_hat_spike_and_hold;
    State::Hardware::hat.at(&Devices::adcs_system()).error_ignored = uplink.fc_hat_adcs_system;
    State::Hardware::hat.at(&Devices::pressure_sensor()).error_ignored = uplink.fc_hat_pressure_sensor;
    State::Hardware::hat.at(&Devices::temp_sensor_inner()).error_ignored = uplink.fc_hat_temp_sensor_inner;
    State::Hardware::hat.at(&Devices::temp_sensor_outer()).error_ignored = uplink.fc_hat_temp_sensor_outer;
    State::Hardware::hat.at(&Devices::docking_motor()).error_ignored = uplink.fc_hat_docking_motor;
    State::Hardware::hat.at(&Devices::docking_switch()).error_ignored = uplink.fc_hat_docking_switch;
  rwMtxWUnlock(&State::Hardware::hardware_state_lock);
}

