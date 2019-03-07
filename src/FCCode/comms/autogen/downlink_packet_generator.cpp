#include "downlink_packet_generator.hpp"

void Comms::generate_packets(std::bitset<PACKET_SIZE_BITS> (&packets)[NUM_PACKETS], unsigned int downlink_no) {
  unsigned int packet_0_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_0.set(packet_0_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(0);
  for(int i = 0; i < 8; i++) {
    packet_0.set(packet_0_ptr++, packet_num_repr[i]);
  }

  std::bitset<11> bitset_0;
  trim_int(State::read(State::Master::master_state,State::Master::Master_state_lock), 0, 0, &bitset_0);
  for(int i = 0; i < bitset_0.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_0[i]);
  }

  std::bitset<11> bitset_1;
  trim_int(State::read(State::Master::pan_state,State::Master::Master_state_lock), 0, 0, &bitset_1);
  for(int i = 0; i < bitset_1.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_1[i]);
  }

  std::bitset<11> bitset_2;
  trim_int(State::read(State::Master::boot_number,State::Master::Master_state_lock), 0, 0, &bitset_2);
  for(int i = 0; i < bitset_2.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_2[i]);
  }

  std::bitset<11> bitset_3;
  trim_gps_time(State::read(State::Master::last_uplink_time,State::Master::Master_state_lock), &bitset_3);
  for(int i = 0; i < bitset_3.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_3[i]);
  }

  std::bitset<1> bitset_4;
  bitset_4.set(0, State::read(State::Master::was_last_uplink_valid, State::Master::Master_state_lock));
  for(int i = 0; i < bitset_4.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_4[i]);
  }

  std::bitset<1> bitset_5;
  bitset_5.set(0, State::read(State::Master::is_follower, State::Master::Master_state_lock));
  for(int i = 0; i < bitset_5.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_5[i]);
  }

  std::bitset<1> bitset_6;
  bitset_6.set(0, State::read(State::Hardware::hat.at(Devices::gomspace.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_6.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_6[i]);
  }

  std::bitset<1> bitset_7;
  bitset_7.set(0, State::read(State::Hardware::hat.at(Devices::gomspace.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_7.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_7[i]);
  }

  std::bitset<1> bitset_8;
  bitset_8.set(0, State::read(State::Hardware::hat.at(Devices::gomspace.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_8.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_8[i]);
  }

  std::bitset<1> bitset_9;
  bitset_9.set(0, State::read(State::Hardware::hat.at(Devices::gomspace.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_9.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_9[i]);
  }

  std::bitset<1> bitset_10;
  bitset_10.set(0, State::read(State::Hardware::hat.at(Devices::piksi.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_10.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_10[i]);
  }

  std::bitset<1> bitset_11;
  bitset_11.set(0, State::read(State::Hardware::hat.at(Devices::piksi.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_11.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_11[i]);
  }

  std::bitset<1> bitset_12;
  bitset_12.set(0, State::read(State::Hardware::hat.at(Devices::piksi.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_12.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_12[i]);
  }

  std::bitset<1> bitset_13;
  bitset_13.set(0, State::read(State::Hardware::hat.at(Devices::piksi.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_13.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_13[i]);
  }

  std::bitset<1> bitset_14;
  bitset_14.set(0, State::read(State::Hardware::hat.at(Devices::quake.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_14.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_14[i]);
  }

  std::bitset<1> bitset_15;
  bitset_15.set(0, State::read(State::Hardware::hat.at(Devices::quake.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_15.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_15[i]);
  }

  std::bitset<1> bitset_16;
  bitset_16.set(0, State::read(State::Hardware::hat.at(Devices::quake.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_16.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_16[i]);
  }

  std::bitset<1> bitset_17;
  bitset_17.set(0, State::read(State::Hardware::hat.at(Devices::quake.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_17.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_17[i]);
  }

  std::bitset<1> bitset_18;
  bitset_18.set(0, State::read(State::Hardware::hat.at(Devices::dcdc.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_18.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_18[i]);
  }

  std::bitset<1> bitset_19;
  bitset_19.set(0, State::read(State::Hardware::hat.at(Devices::dcdc.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_19.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_19[i]);
  }

  std::bitset<1> bitset_20;
  bitset_20.set(0, State::read(State::Hardware::hat.at(Devices::dcdc.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_20.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_20[i]);
  }

  std::bitset<1> bitset_21;
  bitset_21.set(0, State::read(State::Hardware::hat.at(Devices::dcdc.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_21.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_21[i]);
  }

  std::bitset<1> bitset_22;
  bitset_22.set(0, State::read(State::Hardware::hat.at(Devices::spike_and_hold.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_22.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_22[i]);
  }

  std::bitset<1> bitset_23;
  bitset_23.set(0, State::read(State::Hardware::hat.at(Devices::spike_and_hold.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_23.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_23[i]);
  }

  std::bitset<1> bitset_24;
  bitset_24.set(0, State::read(State::Hardware::hat.at(Devices::spike_and_hold.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_24.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_24[i]);
  }

  std::bitset<1> bitset_25;
  bitset_25.set(0, State::read(State::Hardware::hat.at(Devices::spike_and_hold.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_25.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_25[i]);
  }

  std::bitset<1> bitset_26;
  bitset_26.set(0, State::read(State::Hardware::hat.at(Devices::adcs_system.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_26.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_26[i]);
  }

  std::bitset<1> bitset_27;
  bitset_27.set(0, State::read(State::Hardware::hat.at(Devices::adcs_system.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_27.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_27[i]);
  }

  std::bitset<1> bitset_28;
  bitset_28.set(0, State::read(State::Hardware::hat.at(Devices::adcs_system.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_28.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_28[i]);
  }

  std::bitset<1> bitset_29;
  bitset_29.set(0, State::read(State::Hardware::hat.at(Devices::adcs_system.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_29.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_29[i]);
  }

  std::bitset<1> bitset_30;
  bitset_30.set(0, State::read(State::Hardware::hat.at(Devices::pressure_sensor.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_30.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_30[i]);
  }

  std::bitset<1> bitset_31;
  bitset_31.set(0, State::read(State::Hardware::hat.at(Devices::pressure_sensor.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_31.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_31[i]);
  }

  std::bitset<1> bitset_32;
  bitset_32.set(0, State::read(State::Hardware::hat.at(Devices::pressure_sensor.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_32.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_32[i]);
  }

  std::bitset<1> bitset_33;
  bitset_33.set(0, State::read(State::Hardware::hat.at(Devices::pressure_sensor.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_33.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_33[i]);
  }

  std::bitset<1> bitset_34;
  bitset_34.set(0, State::read(State::Hardware::hat.at(Devices::temp_sensor_inner.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_34.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_34[i]);
  }

  std::bitset<1> bitset_35;
  bitset_35.set(0, State::read(State::Hardware::hat.at(Devices::temp_sensor_inner.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_35.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_35[i]);
  }

  std::bitset<1> bitset_36;
  bitset_36.set(0, State::read(State::Hardware::hat.at(Devices::temp_sensor_inner.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_36.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_36[i]);
  }

  std::bitset<1> bitset_37;
  bitset_37.set(0, State::read(State::Hardware::hat.at(Devices::temp_sensor_inner.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_37.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_37[i]);
  }

  std::bitset<1> bitset_38;
  bitset_38.set(0, State::read(State::Hardware::hat.at(Devices::temp_sensor_outer.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_38.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_38[i]);
  }

  std::bitset<1> bitset_39;
  bitset_39.set(0, State::read(State::Hardware::hat.at(Devices::temp_sensor_outer.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_39.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_39[i]);
  }

  std::bitset<1> bitset_40;
  bitset_40.set(0, State::read(State::Hardware::hat.at(Devices::temp_sensor_outer.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_40.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_40[i]);
  }

  std::bitset<1> bitset_41;
  bitset_41.set(0, State::read(State::Hardware::hat.at(Devices::temp_sensor_outer.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_41.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_41[i]);
  }

  std::bitset<1> bitset_42;
  bitset_42.set(0, State::read(State::Hardware::hat.at(Devices::docking_motor.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_42.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_42[i]);
  }

  std::bitset<1> bitset_43;
  bitset_43.set(0, State::read(State::Hardware::hat.at(Devices::docking_motor.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_43.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_43[i]);
  }

  std::bitset<1> bitset_44;
  bitset_44.set(0, State::read(State::Hardware::hat.at(Devices::docking_motor.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_44.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_44[i]);
  }

  std::bitset<1> bitset_45;
  bitset_45.set(0, State::read(State::Hardware::hat.at(Devices::docking_motor.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_45.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_45[i]);
  }

  std::bitset<1> bitset_46;
  bitset_46.set(0, State::read(State::Hardware::hat.at(Devices::docking_switch.name()).powered_on, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_46.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_46[i]);
  }

  std::bitset<1> bitset_47;
  bitset_47.set(0, State::read(State::Hardware::hat.at(Devices::docking_switch.name()).enabled, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_47.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_47[i]);
  }

  std::bitset<1> bitset_48;
  bitset_48.set(0, State::read(State::Hardware::hat.at(Devices::docking_switch.name()).is_functional, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_48.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_48[i]);
  }

  std::bitset<1> bitset_49;
  bitset_49.set(0, State::read(State::Hardware::hat.at(Devices::docking_switch.name()).error_ignored, State::Hardware::Hardware_state_lock));
  for(int i = 0; i < bitset_49.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_49[i]);
  }

  std::bitset<11> bitset_50;
  trim_int(State::read(State::ADCS::adcs_state,State::ADCS::ADCS_state_lock), 0, 0, &bitset_50);
  for(int i = 0; i < bitset_50.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_50[i]);
  }

  std::bitset<11> bitset_51;
  trim_quaternion(State::read(State::ADCS::cmd_attitude,State::ADCS::ADCS_state_lock), &bitset_51);
  for(int i = 0; i < bitset_51.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_51[i]);
  }

  std::bitset<11> bitset_52;
  trim_quaternion(State::read(State::ADCS::cur_attitude,State::ADCS::ADCS_state_lock), &bitset_52);
  for(int i = 0; i < bitset_52.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_52[i]);
  }

  std::bitset<11> bitset_53;
  trim_vector(State::read(State::ADCS::cur_angular_rate,State::ADCS::ADCS_state_lock), 0, 0, &bitset_53);
  for(int i = 0; i < bitset_53.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_53[i]);
  }

  std::bitset<1> bitset_54;
  bitset_54.set(0, State::read(State::ADCS::is_propulsion_pointing_active, State::ADCS::ADCS_state_lock));
  for(int i = 0; i < bitset_54.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_54[i]);
  }

  std::bitset<1> bitset_55;
  bitset_55.set(0, State::read(State::ADCS::is_sun_vector_determination_working, State::ADCS::ADCS_state_lock));
  for(int i = 0; i < bitset_55.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_55[i]);
  }

  std::bitset<11> bitset_56;
  trim_vector(State::read(State::ADCS::rwa_speeds,State::ADCS::ADCS_state_lock), 0, 0, &bitset_56);
  for(int i = 0; i < bitset_56.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_56[i]);
  }

  std::bitset<11> bitset_57;
  trim_vector(State::read(State::ADCS::rwa_speed_cmds,State::ADCS::ADCS_state_lock), 0, 0, &bitset_57);
  for(int i = 0; i < bitset_57.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_57[i]);
  }

  std::bitset<11> bitset_58;
  trim_vector(State::read(State::ADCS::rwa_ramps,State::ADCS::ADCS_state_lock), 0, 0, &bitset_58);
  for(int i = 0; i < bitset_58.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_58[i]);
  }

  std::bitset<11> bitset_59;
  trim_vector(State::read(State::ADCS::rwa_speeds_rd,State::ADCS::ADCS_state_lock), 0, 0, &bitset_59);
  for(int i = 0; i < bitset_59.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_59[i]);
  }

  std::bitset<11> bitset_60;
  trim_vector(State::read(State::ADCS::rwa_speed_cmds_rd,State::ADCS::ADCS_state_lock), 0, 0, &bitset_60);
  for(int i = 0; i < bitset_60.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_60[i]);
  }

  std::bitset<11> bitset_61;
  trim_vector(State::read(State::ADCS::rwa_ramps_rd,State::ADCS::ADCS_state_lock), 0, 0, &bitset_61);
  for(int i = 0; i < bitset_61.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_61[i]);
  }

  std::bitset<11> bitset_62;
  trim_vector(State::read(State::ADCS::mtr_cmds,State::ADCS::ADCS_state_lock), 0, 0, &bitset_62);
  for(int i = 0; i < bitset_62.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_62[i]);
  }

  std::bitset<11> bitset_63;
  trim_vector(State::read(State::ADCS::ssa_vec,State::ADCS::ADCS_state_lock), 0, 0, &bitset_63);
  for(int i = 0; i < bitset_63.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_63[i]);
  }

  std::bitset<11> bitset_64;
  trim_vector(State::read(State::ADCS::gyro_data,State::ADCS::ADCS_state_lock), 0, 0, &bitset_64);
  for(int i = 0; i < bitset_64.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_64[i]);
  }

  std::bitset<11> bitset_65;
  trim_vector(State::read(State::ADCS::mag_data,State::ADCS::ADCS_state_lock), 0, 0, &bitset_65);
  for(int i = 0; i < bitset_65.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_65[i]);
  }

  std::bitset<11> bitset_66;
  trim_vector(State::read(State::ADCS::ssa_adc_data,State::ADCS::ADCS_state_lock), 0, 0, &bitset_66);
  for(int i = 0; i < bitset_66.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_66[i]);
  }

  std::bitset<11> bitset_67;
  trim_int(State::read(State::Gomspace::gomspace_data.vboost[0],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_67);
  for(int i = 0; i < bitset_67.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_67[i]);
  }

  std::bitset<11> bitset_68;
  trim_int(State::read(State::Gomspace::gomspace_data.vboost[1],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_68);
  for(int i = 0; i < bitset_68.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_68[i]);
  }

  std::bitset<11> bitset_69;
  trim_int(State::read(State::Gomspace::gomspace_data.vboost[2],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_69);
  for(int i = 0; i < bitset_69.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_69[i]);
  }

  std::bitset<11> bitset_70;
  trim_int(State::read(State::Gomspace::gomspace_data.vbatt,State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_70);
  for(int i = 0; i < bitset_70.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_70[i]);
  }

  std::bitset<11> bitset_71;
  trim_int(State::read(State::Gomspace::gomspace_data.curin[0],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_71);
  for(int i = 0; i < bitset_71.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_71[i]);
  }

  std::bitset<11> bitset_72;
  trim_int(State::read(State::Gomspace::gomspace_data.curin[1],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_72);
  for(int i = 0; i < bitset_72.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_72[i]);
  }

  std::bitset<11> bitset_73;
  trim_int(State::read(State::Gomspace::gomspace_data.curin[2],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_73);
  for(int i = 0; i < bitset_73.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_73[i]);
  }

  std::bitset<11> bitset_74;
  trim_int(State::read(State::Gomspace::gomspace_data.cursun,State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_74);
  for(int i = 0; i < bitset_74.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_74[i]);
  }

  std::bitset<11> bitset_75;
  trim_int(State::read(State::Gomspace::gomspace_data.cursys,State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_75);
  for(int i = 0; i < bitset_75.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_75[i]);
  }

  std::bitset<11> bitset_76;
  trim_int(State::read(State::Gomspace::gomspace_data.curout[0],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_76);
  for(int i = 0; i < bitset_76.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_76[i]);
  }

  std::bitset<11> bitset_77;
  trim_int(State::read(State::Gomspace::gomspace_data.curout[1],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_77);
  for(int i = 0; i < bitset_77.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_77[i]);
  }

  std::bitset<11> bitset_78;
  trim_int(State::read(State::Gomspace::gomspace_data.curout[2],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_78);
  for(int i = 0; i < bitset_78.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_78[i]);
  }

  std::bitset<11> bitset_79;
  trim_int(State::read(State::Gomspace::gomspace_data.curout[3],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_79);
  for(int i = 0; i < bitset_79.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_79[i]);
  }

  std::bitset<11> bitset_80;
  trim_int(State::read(State::Gomspace::gomspace_data.curout[4],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_80);
  for(int i = 0; i < bitset_80.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_80[i]);
  }

  std::bitset<11> bitset_81;
  trim_int(State::read(State::Gomspace::gomspace_data.curout[5],State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_81);
  for(int i = 0; i < bitset_81.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_81[i]);
  }

  std::bitset<1> bitset_82;
  bitset_82.set(0, State::read(State::Gomspace::gomspace_data.output[0], State::Gomspace::Gomspace_state_lock));
  for(int i = 0; i < bitset_82.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_82[i]);
  }

  std::bitset<1> bitset_83;
  bitset_83.set(0, State::read(State::Gomspace::gomspace_data.output[1], State::Gomspace::Gomspace_state_lock));
  for(int i = 0; i < bitset_83.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_83[i]);
  }

  std::bitset<1> bitset_84;
  bitset_84.set(0, State::read(State::Gomspace::gomspace_data.output[2], State::Gomspace::Gomspace_state_lock));
  for(int i = 0; i < bitset_84.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_84[i]);
  }

  std::bitset<1> bitset_85;
  bitset_85.set(0, State::read(State::Gomspace::gomspace_data.output[3], State::Gomspace::Gomspace_state_lock));
  for(int i = 0; i < bitset_85.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_85[i]);
  }

  std::bitset<1> bitset_86;
  bitset_86.set(0, State::read(State::Gomspace::gomspace_data.output[4], State::Gomspace::Gomspace_state_lock));
  for(int i = 0; i < bitset_86.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_86[i]);
  }

  std::bitset<1> bitset_87;
  bitset_87.set(0, State::read(State::Gomspace::gomspace_data.output[5], State::Gomspace::Gomspace_state_lock));
  for(int i = 0; i < bitset_87.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_87[i]);
  }

  std::bitset<1> bitset_88;
  bitset_88.set(0, State::read(State::Gomspace::gomspace_data.output[6], State::Gomspace::Gomspace_state_lock));
  for(int i = 0; i < bitset_88.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_88[i]);
  }

  std::bitset<1> bitset_89;
  bitset_89.set(0, State::read(State::Gomspace::gomspace_data.output[7], State::Gomspace::Gomspace_state_lock));
  for(int i = 0; i < bitset_89.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_89[i]);
  }

  std::bitset<11> bitset_90;
  trim_int(State::read(State::Gomspace::gomspace_data.counter_boot,State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_90);
  for(int i = 0; i < bitset_90.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_90[i]);
  }

  std::bitset<11> bitset_91;
  trim_temperature(State::read(State::Gomspace::gomspace_data.temp[0],State::Gomspace::Gomspace_state_lock), &bitset_91);
  for(int i = 0; i < bitset_91.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_91[i]);
  }

  std::bitset<11> bitset_92;
  trim_temperature(State::read(State::Gomspace::gomspace_data.temp[1],State::Gomspace::Gomspace_state_lock), &bitset_92);
  for(int i = 0; i < bitset_92.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_92[i]);
  }

  std::bitset<11> bitset_93;
  trim_temperature(State::read(State::Gomspace::gomspace_data.temp[2],State::Gomspace::Gomspace_state_lock), &bitset_93);
  for(int i = 0; i < bitset_93.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_93[i]);
  }

  std::bitset<11> bitset_94;
  trim_temperature(State::read(State::Gomspace::gomspace_data.temp[3],State::Gomspace::Gomspace_state_lock), &bitset_94);
  for(int i = 0; i < bitset_94.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_94[i]);
  }

  std::bitset<11> bitset_95;
  trim_int(State::read(State::Gomspace::gomspace_data.battmode,State::Gomspace::Gomspace_state_lock), 0, 0, &bitset_95);
  for(int i = 0; i < bitset_95.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_95[i]);
  }

  std::bitset<11> bitset_96;
  trim_int(State::read(State::Propulsion::propulsion_state,State::Propulsion::Propulsion_state_lock), 0, 0, &bitset_96);
  for(int i = 0; i < bitset_96.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_96[i]);
  }

  std::bitset<11> bitset_97;
  trim_vector(State::read(State::Propulsion::delta_v_available,State::Propulsion::Propulsion_state_lock), 0, 0, &bitset_97);
  for(int i = 0; i < bitset_97.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_97[i]);
  }

  std::bitset<1> bitset_98;
  bitset_98.set(0, State::read(State::Propulsion::is_firing_planned, State::Propulsion::Propulsion_state_lock));
  for(int i = 0; i < bitset_98.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_98[i]);
  }

  std::bitset<1> bitset_99;
  bitset_99.set(0, State::read(State::Propulsion::is_firing_planned_by_uplink, State::Propulsion::Propulsion_state_lock));
  for(int i = 0; i < bitset_99.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_99[i]);
  }

  std::bitset<11> bitset_100;
  trim_vector(State::read(State::Propulsion::firing_data.impulse_vector,State::Propulsion::Propulsion_state_lock), 0, 0, &bitset_100);
  for(int i = 0; i < bitset_100.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_100[i]);
  }

  std::bitset<11> bitset_101;
  trim_gps_time(State::read(State::Propulsion::firing_data.time,State::Propulsion::Propulsion_state_lock), &bitset_101);
  for(int i = 0; i < bitset_101.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_101[i]);
  }

  std::bitset<11> bitset_102;
  trim_gps_time(State::read(State::Piksi::recorded_current_time,State::Piksi::Piksi_state_lock), &bitset_102);
  for(int i = 0; i < bitset_102.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_102[i]);
  }

  std::bitset<32> bitset_103;
  trim_int(State::read(State::Piksi::recorded_current_time_collection_timestamp,State::Piksi::Piksi_state_lock), , , &bitset_103);
  for(int i = 0; i < bitset_103.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_103[i]);
  }

  std::bitset<11> bitset_104;
  trim_gps_time(State::read(State::GNC::current_time,State::GNC::GNC_state_lock), &bitset_104);
  for(int i = 0; i < bitset_104.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_104[i]);
  }

  std::bitset<32> bitset_105;
  trim_int(State::read(State::GNC::current_time_collection_timestamp,State::GNC::GNC_state_lock), 0, 0, &bitset_105);
  for(int i = 0; i < bitset_105.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_105[i]);
  }

  std::bitset<1> bitset_106;
  bitset_106.set(0, State::read(State::GNC::has_firing_happened_in_nighttime, State::GNC::GNC_state_lock));
  for(int i = 0; i < bitset_106.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_106[i]);
  }

  std::bitset<11> bitset_107;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[0],StateHistory::ADCS::story::ADCS_state_lock), &bitset_107);
  for(int i = 0; i < bitset_107.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_107[i]);
  }

  std::bitset<11> bitset_108;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[1],StateHistory::ADCS::story::ADCS_state_lock), &bitset_108);
  for(int i = 0; i < bitset_108.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_108[i]);
  }

  std::bitset<11> bitset_109;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[2],StateHistory::ADCS::story::ADCS_state_lock), &bitset_109);
  for(int i = 0; i < bitset_109.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_109[i]);
  }

  std::bitset<11> bitset_110;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[3],StateHistory::ADCS::story::ADCS_state_lock), &bitset_110);
  for(int i = 0; i < bitset_110.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_110[i]);
  }

  std::bitset<11> bitset_111;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[4],StateHistory::ADCS::story::ADCS_state_lock), &bitset_111);
  for(int i = 0; i < bitset_111.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_111[i]);
  }

  std::bitset<11> bitset_112;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[5],StateHistory::ADCS::story::ADCS_state_lock), &bitset_112);
  for(int i = 0; i < bitset_112.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_112[i]);
  }

  std::bitset<11> bitset_113;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[6],StateHistory::ADCS::story::ADCS_state_lock), &bitset_113);
  for(int i = 0; i < bitset_113.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_113[i]);
  }

  std::bitset<11> bitset_114;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[7],StateHistory::ADCS::story::ADCS_state_lock), &bitset_114);
  for(int i = 0; i < bitset_114.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_114[i]);
  }

  std::bitset<11> bitset_115;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[8],StateHistory::ADCS::story::ADCS_state_lock), &bitset_115);
  for(int i = 0; i < bitset_115.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_115[i]);
  }

  std::bitset<11> bitset_116;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[9],StateHistory::ADCS::story::ADCS_state_lock), &bitset_116);
  for(int i = 0; i < bitset_116.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_116[i]);
  }

  std::bitset<11> bitset_117;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[10],StateHistory::ADCS::story::ADCS_state_lock), &bitset_117);
  for(int i = 0; i < bitset_117.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_117[i]);
  }

  std::bitset<11> bitset_118;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[11],StateHistory::ADCS::story::ADCS_state_lock), &bitset_118);
  for(int i = 0; i < bitset_118.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_118[i]);
  }

  std::bitset<11> bitset_119;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[12],StateHistory::ADCS::story::ADCS_state_lock), &bitset_119);
  for(int i = 0; i < bitset_119.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_119[i]);
  }

  std::bitset<11> bitset_120;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[13],StateHistory::ADCS::story::ADCS_state_lock), &bitset_120);
  for(int i = 0; i < bitset_120.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_120[i]);
  }

  std::bitset<11> bitset_121;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[14],StateHistory::ADCS::story::ADCS_state_lock), &bitset_121);
  for(int i = 0; i < bitset_121.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_121[i]);
  }

  std::bitset<11> bitset_122;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[15],StateHistory::ADCS::story::ADCS_state_lock), &bitset_122);
  for(int i = 0; i < bitset_122.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_122[i]);
  }

  std::bitset<11> bitset_123;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[16],StateHistory::ADCS::story::ADCS_state_lock), &bitset_123);
  for(int i = 0; i < bitset_123.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_123[i]);
  }

  std::bitset<11> bitset_124;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[17],StateHistory::ADCS::story::ADCS_state_lock), &bitset_124);
  for(int i = 0; i < bitset_124.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_124[i]);
  }

  std::bitset<11> bitset_125;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[18],StateHistory::ADCS::story::ADCS_state_lock), &bitset_125);
  for(int i = 0; i < bitset_125.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_125[i]);
  }

  std::bitset<11> bitset_126;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_cmd_history[19],StateHistory::ADCS::story::ADCS_state_lock), &bitset_126);
  for(int i = 0; i < bitset_126.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_126[i]);
  }

  std::bitset<11> bitset_127;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[0],StateHistory::ADCS::story::ADCS_state_lock), &bitset_127);
  for(int i = 0; i < bitset_127.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_127[i]);
  }

  std::bitset<11> bitset_128;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[1],StateHistory::ADCS::story::ADCS_state_lock), &bitset_128);
  for(int i = 0; i < bitset_128.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_128[i]);
  }

  std::bitset<11> bitset_129;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[2],StateHistory::ADCS::story::ADCS_state_lock), &bitset_129);
  for(int i = 0; i < bitset_129.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_129[i]);
  }

  std::bitset<11> bitset_130;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[3],StateHistory::ADCS::story::ADCS_state_lock), &bitset_130);
  for(int i = 0; i < bitset_130.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_130[i]);
  }

  std::bitset<11> bitset_131;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[4],StateHistory::ADCS::story::ADCS_state_lock), &bitset_131);
  for(int i = 0; i < bitset_131.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_131[i]);
  }

  std::bitset<11> bitset_132;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[5],StateHistory::ADCS::story::ADCS_state_lock), &bitset_132);
  for(int i = 0; i < bitset_132.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_132[i]);
  }

  std::bitset<11> bitset_133;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[6],StateHistory::ADCS::story::ADCS_state_lock), &bitset_133);
  for(int i = 0; i < bitset_133.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_133[i]);
  }

  std::bitset<11> bitset_134;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[7],StateHistory::ADCS::story::ADCS_state_lock), &bitset_134);
  for(int i = 0; i < bitset_134.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_134[i]);
  }

  std::bitset<11> bitset_135;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[8],StateHistory::ADCS::story::ADCS_state_lock), &bitset_135);
  for(int i = 0; i < bitset_135.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_135[i]);
  }

  std::bitset<11> bitset_136;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[9],StateHistory::ADCS::story::ADCS_state_lock), &bitset_136);
  for(int i = 0; i < bitset_136.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_136[i]);
  }

  std::bitset<11> bitset_137;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[10],StateHistory::ADCS::story::ADCS_state_lock), &bitset_137);
  for(int i = 0; i < bitset_137.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_137[i]);
  }

  std::bitset<11> bitset_138;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[11],StateHistory::ADCS::story::ADCS_state_lock), &bitset_138);
  for(int i = 0; i < bitset_138.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_138[i]);
  }

  std::bitset<11> bitset_139;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[12],StateHistory::ADCS::story::ADCS_state_lock), &bitset_139);
  for(int i = 0; i < bitset_139.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_139[i]);
  }

  std::bitset<11> bitset_140;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[13],StateHistory::ADCS::story::ADCS_state_lock), &bitset_140);
  for(int i = 0; i < bitset_140.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_140[i]);
  }

  std::bitset<11> bitset_141;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[14],StateHistory::ADCS::story::ADCS_state_lock), &bitset_141);
  for(int i = 0; i < bitset_141.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_141[i]);
  }

  std::bitset<11> bitset_142;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[15],StateHistory::ADCS::story::ADCS_state_lock), &bitset_142);
  for(int i = 0; i < bitset_142.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_142[i]);
  }

  std::bitset<11> bitset_143;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[16],StateHistory::ADCS::story::ADCS_state_lock), &bitset_143);
  for(int i = 0; i < bitset_143.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_143[i]);
  }

  std::bitset<11> bitset_144;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[17],StateHistory::ADCS::story::ADCS_state_lock), &bitset_144);
  for(int i = 0; i < bitset_144.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_144[i]);
  }

  std::bitset<11> bitset_145;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[18],StateHistory::ADCS::story::ADCS_state_lock), &bitset_145);
  for(int i = 0; i < bitset_145.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_145[i]);
  }

  std::bitset<11> bitset_146;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_history[19],StateHistory::ADCS::story::ADCS_state_lock), &bitset_146);
  for(int i = 0; i < bitset_146.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_146[i]);
  }

  std::bitset<11> bitset_147;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[0],StateHistory::ADCS::story::ADCS_state_lock), &bitset_147);
  for(int i = 0; i < bitset_147.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_147[i]);
  }

  std::bitset<11> bitset_148;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[1],StateHistory::ADCS::story::ADCS_state_lock), &bitset_148);
  for(int i = 0; i < bitset_148.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_148[i]);
  }

  std::bitset<11> bitset_149;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[2],StateHistory::ADCS::story::ADCS_state_lock), &bitset_149);
  for(int i = 0; i < bitset_149.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_149[i]);
  }

  std::bitset<11> bitset_150;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[3],StateHistory::ADCS::story::ADCS_state_lock), &bitset_150);
  for(int i = 0; i < bitset_150.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_150[i]);
  }

  std::bitset<11> bitset_151;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[4],StateHistory::ADCS::story::ADCS_state_lock), &bitset_151);
  for(int i = 0; i < bitset_151.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_151[i]);
  }

  std::bitset<11> bitset_152;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[5],StateHistory::ADCS::story::ADCS_state_lock), &bitset_152);
  for(int i = 0; i < bitset_152.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_152[i]);
  }

  std::bitset<11> bitset_153;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[6],StateHistory::ADCS::story::ADCS_state_lock), &bitset_153);
  for(int i = 0; i < bitset_153.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_153[i]);
  }

  std::bitset<11> bitset_154;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[7],StateHistory::ADCS::story::ADCS_state_lock), &bitset_154);
  for(int i = 0; i < bitset_154.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_154[i]);
  }

  std::bitset<11> bitset_155;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[8],StateHistory::ADCS::story::ADCS_state_lock), &bitset_155);
  for(int i = 0; i < bitset_155.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_155[i]);
  }

  std::bitset<11> bitset_156;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[9],StateHistory::ADCS::story::ADCS_state_lock), &bitset_156);
  for(int i = 0; i < bitset_156.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_156[i]);
  }

  std::bitset<11> bitset_157;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[10],StateHistory::ADCS::story::ADCS_state_lock), &bitset_157);
  for(int i = 0; i < bitset_157.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_157[i]);
  }

  std::bitset<11> bitset_158;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[11],StateHistory::ADCS::story::ADCS_state_lock), &bitset_158);
  for(int i = 0; i < bitset_158.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_158[i]);
  }

  std::bitset<11> bitset_159;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[12],StateHistory::ADCS::story::ADCS_state_lock), &bitset_159);
  for(int i = 0; i < bitset_159.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_159[i]);
  }

  std::bitset<11> bitset_160;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[13],StateHistory::ADCS::story::ADCS_state_lock), &bitset_160);
  for(int i = 0; i < bitset_160.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_160[i]);
  }

  std::bitset<11> bitset_161;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[14],StateHistory::ADCS::story::ADCS_state_lock), &bitset_161);
  for(int i = 0; i < bitset_161.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_161[i]);
  }

  std::bitset<11> bitset_162;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[15],StateHistory::ADCS::story::ADCS_state_lock), &bitset_162);
  for(int i = 0; i < bitset_162.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_162[i]);
  }

  std::bitset<11> bitset_163;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[16],StateHistory::ADCS::story::ADCS_state_lock), &bitset_163);
  for(int i = 0; i < bitset_163.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_163[i]);
  }

  std::bitset<11> bitset_164;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[17],StateHistory::ADCS::story::ADCS_state_lock), &bitset_164);
  for(int i = 0; i < bitset_164.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_164[i]);
  }

  std::bitset<11> bitset_165;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[18],StateHistory::ADCS::story::ADCS_state_lock), &bitset_165);
  for(int i = 0; i < bitset_165.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_165[i]);
  }

  std::bitset<11> bitset_166;
  trim_quaternion(State::read(StateHistory::ADCS::attitude_fast_history[19],StateHistory::ADCS::story::ADCS_state_lock), &bitset_166);
  for(int i = 0; i < bitset_166.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_166[i]);
  }

  std::bitset<11> bitset_167;
  trim_vector(State::read(StateHistory::ADCS::rate_history[0],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_167);
  for(int i = 0; i < bitset_167.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_167[i]);
  }

  std::bitset<11> bitset_168;
  trim_vector(State::read(StateHistory::ADCS::rate_history[1],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_168);
  for(int i = 0; i < bitset_168.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_168[i]);
  }

  std::bitset<11> bitset_169;
  trim_vector(State::read(StateHistory::ADCS::rate_history[2],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_169);
  for(int i = 0; i < bitset_169.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_169[i]);
  }

  std::bitset<11> bitset_170;
  trim_vector(State::read(StateHistory::ADCS::rate_history[3],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_170);
  for(int i = 0; i < bitset_170.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_170[i]);
  }

  std::bitset<11> bitset_171;
  trim_vector(State::read(StateHistory::ADCS::rate_history[4],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_171);
  for(int i = 0; i < bitset_171.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_171[i]);
  }

  std::bitset<11> bitset_172;
  trim_vector(State::read(StateHistory::ADCS::rate_history[5],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_172);
  for(int i = 0; i < bitset_172.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_172[i]);
  }

  std::bitset<11> bitset_173;
  trim_vector(State::read(StateHistory::ADCS::rate_history[6],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_173);
  for(int i = 0; i < bitset_173.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_173[i]);
  }

  std::bitset<11> bitset_174;
  trim_vector(State::read(StateHistory::ADCS::rate_history[7],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_174);
  for(int i = 0; i < bitset_174.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_174[i]);
  }

  std::bitset<11> bitset_175;
  trim_vector(State::read(StateHistory::ADCS::rate_history[8],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_175);
  for(int i = 0; i < bitset_175.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_175[i]);
  }

  std::bitset<11> bitset_176;
  trim_vector(State::read(StateHistory::ADCS::rate_history[9],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_176);
  for(int i = 0; i < bitset_176.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_176[i]);
  }

  std::bitset<11> bitset_177;
  trim_vector(State::read(StateHistory::ADCS::rate_history[10],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_177);
  for(int i = 0; i < bitset_177.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_177[i]);
  }

  std::bitset<11> bitset_178;
  trim_vector(State::read(StateHistory::ADCS::rate_history[11],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_178);
  for(int i = 0; i < bitset_178.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_178[i]);
  }

  std::bitset<11> bitset_179;
  trim_vector(State::read(StateHistory::ADCS::rate_history[12],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_179);
  for(int i = 0; i < bitset_179.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_179[i]);
  }

  std::bitset<11> bitset_180;
  trim_vector(State::read(StateHistory::ADCS::rate_history[13],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_180);
  for(int i = 0; i < bitset_180.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_180[i]);
  }

  std::bitset<11> bitset_181;
  trim_vector(State::read(StateHistory::ADCS::rate_history[14],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_181);
  for(int i = 0; i < bitset_181.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_181[i]);
  }

  std::bitset<11> bitset_182;
  trim_vector(State::read(StateHistory::ADCS::rate_history[15],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_182);
  for(int i = 0; i < bitset_182.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_182[i]);
  }

  std::bitset<11> bitset_183;
  trim_vector(State::read(StateHistory::ADCS::rate_history[16],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_183);
  for(int i = 0; i < bitset_183.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_183[i]);
  }

  std::bitset<11> bitset_184;
  trim_vector(State::read(StateHistory::ADCS::rate_history[17],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_184);
  for(int i = 0; i < bitset_184.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_184[i]);
  }

  std::bitset<11> bitset_185;
  trim_vector(State::read(StateHistory::ADCS::rate_history[18],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_185);
  for(int i = 0; i < bitset_185.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_185[i]);
  }

  std::bitset<11> bitset_186;
  trim_vector(State::read(StateHistory::ADCS::rate_history[19],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_186);
  for(int i = 0; i < bitset_186.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_186[i]);
  }

  std::bitset<11> bitset_187;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[0],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_187);
  for(int i = 0; i < bitset_187.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_187[i]);
  }

  std::bitset<11> bitset_188;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[1],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_188);
  for(int i = 0; i < bitset_188.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_188[i]);
  }

  std::bitset<11> bitset_189;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[2],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_189);
  for(int i = 0; i < bitset_189.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_189[i]);
  }

  std::bitset<11> bitset_190;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[3],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_190);
  for(int i = 0; i < bitset_190.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_190[i]);
  }

  std::bitset<11> bitset_191;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[4],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_191);
  for(int i = 0; i < bitset_191.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_191[i]);
  }

  std::bitset<11> bitset_192;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[5],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_192);
  for(int i = 0; i < bitset_192.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_192[i]);
  }

  std::bitset<11> bitset_193;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[6],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_193);
  for(int i = 0; i < bitset_193.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_193[i]);
  }

  std::bitset<11> bitset_194;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[7],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_194);
  for(int i = 0; i < bitset_194.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_194[i]);
  }

  std::bitset<11> bitset_195;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[8],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_195);
  for(int i = 0; i < bitset_195.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_195[i]);
  }

  std::bitset<11> bitset_196;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[9],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_196);
  for(int i = 0; i < bitset_196.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_196[i]);
  }

  std::bitset<11> bitset_197;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[10],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_197);
  for(int i = 0; i < bitset_197.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_197[i]);
  }

  std::bitset<11> bitset_198;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[11],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_198);
  for(int i = 0; i < bitset_198.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_198[i]);
  }

  std::bitset<11> bitset_199;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[12],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_199);
  for(int i = 0; i < bitset_199.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_199[i]);
  }

  std::bitset<11> bitset_200;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[13],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_200);
  for(int i = 0; i < bitset_200.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_200[i]);
  }

  std::bitset<11> bitset_201;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[14],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_201);
  for(int i = 0; i < bitset_201.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_201[i]);
  }

  std::bitset<11> bitset_202;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[15],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_202);
  for(int i = 0; i < bitset_202.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_202[i]);
  }

  std::bitset<11> bitset_203;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[16],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_203);
  for(int i = 0; i < bitset_203.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_203[i]);
  }

  std::bitset<11> bitset_204;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[17],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_204);
  for(int i = 0; i < bitset_204.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_204[i]);
  }

  std::bitset<11> bitset_205;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[18],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_205);
  for(int i = 0; i < bitset_205.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_205[i]);
  }

  std::bitset<11> bitset_206;
  trim_vector(State::read(StateHistory::ADCS::rate_fast_history[19],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_206);
  for(int i = 0; i < bitset_206.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_206[i]);
  }

  std::bitset<11> bitset_207;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[0],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_207);
  for(int i = 0; i < bitset_207.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_207[i]);
  }

  std::bitset<11> bitset_208;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[1],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_208);
  for(int i = 0; i < bitset_208.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_208[i]);
  }

  std::bitset<11> bitset_209;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[2],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_209);
  for(int i = 0; i < bitset_209.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_209[i]);
  }

  std::bitset<11> bitset_210;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[3],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_210);
  for(int i = 0; i < bitset_210.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_210[i]);
  }

  std::bitset<11> bitset_211;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[4],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_211);
  for(int i = 0; i < bitset_211.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_211[i]);
  }

  std::bitset<11> bitset_212;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[5],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_212);
  for(int i = 0; i < bitset_212.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_212[i]);
  }

  std::bitset<11> bitset_213;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[6],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_213);
  for(int i = 0; i < bitset_213.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_213[i]);
  }

  std::bitset<11> bitset_214;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[7],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_214);
  for(int i = 0; i < bitset_214.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_214[i]);
  }

  std::bitset<11> bitset_215;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[8],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_215);
  for(int i = 0; i < bitset_215.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_215[i]);
  }

  std::bitset<11> bitset_216;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[9],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_216);
  for(int i = 0; i < bitset_216.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_216[i]);
  }

  std::bitset<11> bitset_217;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[10],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_217);
  for(int i = 0; i < bitset_217.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_217[i]);
  }

  std::bitset<11> bitset_218;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[11],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_218);
  for(int i = 0; i < bitset_218.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_218[i]);
  }

  std::bitset<11> bitset_219;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[12],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_219);
  for(int i = 0; i < bitset_219.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_219[i]);
  }

  std::bitset<11> bitset_220;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[13],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_220);
  for(int i = 0; i < bitset_220.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_220[i]);
  }

  std::bitset<11> bitset_221;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[14],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_221);
  for(int i = 0; i < bitset_221.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_221[i]);
  }

  std::bitset<11> bitset_222;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[15],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_222);
  for(int i = 0; i < bitset_222.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_222[i]);
  }

  std::bitset<11> bitset_223;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[16],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_223);
  for(int i = 0; i < bitset_223.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_223[i]);
  }

  std::bitset<11> bitset_224;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[17],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_224);
  for(int i = 0; i < bitset_224.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_224[i]);
  }

  std::bitset<11> bitset_225;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[18],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_225);
  for(int i = 0; i < bitset_225.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_225[i]);
  }

  std::bitset<11> bitset_226;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_history[19],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_226);
  for(int i = 0; i < bitset_226.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_226[i]);
  }

  std::bitset<11> bitset_227;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[0],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_227);
  for(int i = 0; i < bitset_227.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_227[i]);
  }

  std::bitset<11> bitset_228;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[1],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_228);
  for(int i = 0; i < bitset_228.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_228[i]);
  }

  std::bitset<11> bitset_229;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[2],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_229);
  for(int i = 0; i < bitset_229.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_229[i]);
  }

  std::bitset<11> bitset_230;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[3],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_230);
  for(int i = 0; i < bitset_230.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_230[i]);
  }

  std::bitset<11> bitset_231;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[4],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_231);
  for(int i = 0; i < bitset_231.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_231[i]);
  }

  std::bitset<11> bitset_232;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[5],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_232);
  for(int i = 0; i < bitset_232.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_232[i]);
  }

  std::bitset<11> bitset_233;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[6],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_233);
  for(int i = 0; i < bitset_233.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_233[i]);
  }

  std::bitset<11> bitset_234;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[7],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_234);
  for(int i = 0; i < bitset_234.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_234[i]);
  }

  std::bitset<11> bitset_235;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[8],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_235);
  for(int i = 0; i < bitset_235.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_235[i]);
  }

  std::bitset<11> bitset_236;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[9],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_236);
  for(int i = 0; i < bitset_236.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_236[i]);
  }

  std::bitset<11> bitset_237;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[10],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_237);
  for(int i = 0; i < bitset_237.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_237[i]);
  }

  std::bitset<11> bitset_238;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[11],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_238);
  for(int i = 0; i < bitset_238.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_238[i]);
  }

  std::bitset<11> bitset_239;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[12],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_239);
  for(int i = 0; i < bitset_239.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_239[i]);
  }

  std::bitset<11> bitset_240;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[13],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_240);
  for(int i = 0; i < bitset_240.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_240[i]);
  }

  std::bitset<11> bitset_241;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[14],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_241);
  for(int i = 0; i < bitset_241.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_241[i]);
  }

  std::bitset<11> bitset_242;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[15],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_242);
  for(int i = 0; i < bitset_242.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_242[i]);
  }

  std::bitset<11> bitset_243;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[16],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_243);
  for(int i = 0; i < bitset_243.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_243[i]);
  }

  std::bitset<11> bitset_244;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[17],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_244);
  for(int i = 0; i < bitset_244.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_244[i]);
  }

  std::bitset<11> bitset_245;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[18],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_245);
  for(int i = 0; i < bitset_245.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_245[i]);
  }

  std::bitset<11> bitset_246;
  trim_vector(State::read(StateHistory::ADCS::spacecraft_L_fast_history[19],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_246);
  for(int i = 0; i < bitset_246.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_246[i]);
  }

  std::bitset<11> bitset_247;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[0],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_247);
  for(int i = 0; i < bitset_247.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_247[i]);
  }

  std::bitset<11> bitset_248;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[1],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_248);
  for(int i = 0; i < bitset_248.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_248[i]);
  }

  std::bitset<11> bitset_249;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[2],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_249);
  for(int i = 0; i < bitset_249.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_249[i]);
  }

  std::bitset<11> bitset_250;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[3],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_250);
  for(int i = 0; i < bitset_250.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_250[i]);
  }

  std::bitset<11> bitset_251;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[4],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_251);
  for(int i = 0; i < bitset_251.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_251[i]);
  }

  std::bitset<11> bitset_252;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[5],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_252);
  for(int i = 0; i < bitset_252.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_252[i]);
  }

  std::bitset<11> bitset_253;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[6],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_253);
  for(int i = 0; i < bitset_253.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_253[i]);
  }

  std::bitset<11> bitset_254;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[7],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_254);
  for(int i = 0; i < bitset_254.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_254[i]);
  }

  std::bitset<11> bitset_255;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[8],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_255);
  for(int i = 0; i < bitset_255.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_255[i]);
  }

  std::bitset<11> bitset_256;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[9],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_256);
  for(int i = 0; i < bitset_256.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_256[i]);
  }

  std::bitset<11> bitset_257;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[10],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_257);
  for(int i = 0; i < bitset_257.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_257[i]);
  }

  std::bitset<11> bitset_258;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[11],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_258);
  for(int i = 0; i < bitset_258.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_258[i]);
  }

  std::bitset<11> bitset_259;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[12],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_259);
  for(int i = 0; i < bitset_259.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_259[i]);
  }

  std::bitset<11> bitset_260;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[13],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_260);
  for(int i = 0; i < bitset_260.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_260[i]);
  }

  std::bitset<11> bitset_261;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[14],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_261);
  for(int i = 0; i < bitset_261.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_261[i]);
  }

  std::bitset<11> bitset_262;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[15],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_262);
  for(int i = 0; i < bitset_262.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_262[i]);
  }

  std::bitset<11> bitset_263;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[16],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_263);
  for(int i = 0; i < bitset_263.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_263[i]);
  }

  std::bitset<11> bitset_264;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[17],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_264);
  for(int i = 0; i < bitset_264.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_264[i]);
  }

  std::bitset<11> bitset_265;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[18],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_265);
  for(int i = 0; i < bitset_265.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_265[i]);
  }

  std::bitset<11> bitset_266;
  trim_vector(State::read(StateHistory::ADCS::gyro_history[19],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_266);
  for(int i = 0; i < bitset_266.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_266[i]);
  }

  std::bitset<11> bitset_267;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[0],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_267);
  for(int i = 0; i < bitset_267.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_267[i]);
  }

  std::bitset<11> bitset_268;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[1],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_268);
  for(int i = 0; i < bitset_268.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_268[i]);
  }

  std::bitset<11> bitset_269;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[2],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_269);
  for(int i = 0; i < bitset_269.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_269[i]);
  }

  std::bitset<11> bitset_270;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[3],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_270);
  for(int i = 0; i < bitset_270.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_270[i]);
  }

  std::bitset<11> bitset_271;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[4],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_271);
  for(int i = 0; i < bitset_271.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_271[i]);
  }

  std::bitset<11> bitset_272;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[5],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_272);
  for(int i = 0; i < bitset_272.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_272[i]);
  }

  std::bitset<11> bitset_273;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[6],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_273);
  for(int i = 0; i < bitset_273.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_273[i]);
  }

  std::bitset<11> bitset_274;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[7],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_274);
  for(int i = 0; i < bitset_274.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_274[i]);
  }

  std::bitset<11> bitset_275;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[8],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_275);
  for(int i = 0; i < bitset_275.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_275[i]);
  }

  std::bitset<11> bitset_276;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[9],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_276);
  for(int i = 0; i < bitset_276.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_276[i]);
  }

  std::bitset<11> bitset_277;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[10],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_277);
  for(int i = 0; i < bitset_277.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_277[i]);
  }

  std::bitset<11> bitset_278;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[11],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_278);
  for(int i = 0; i < bitset_278.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_278[i]);
  }

  std::bitset<11> bitset_279;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[12],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_279);
  for(int i = 0; i < bitset_279.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_279[i]);
  }

  std::bitset<11> bitset_280;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[13],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_280);
  for(int i = 0; i < bitset_280.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_280[i]);
  }

  std::bitset<11> bitset_281;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[14],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_281);
  for(int i = 0; i < bitset_281.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_281[i]);
  }

  std::bitset<11> bitset_282;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[15],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_282);
  for(int i = 0; i < bitset_282.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_282[i]);
  }

  std::bitset<11> bitset_283;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[16],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_283);
  for(int i = 0; i < bitset_283.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_283[i]);
  }

  std::bitset<11> bitset_284;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[17],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_284);
  for(int i = 0; i < bitset_284.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_284[i]);
  }

  std::bitset<11> bitset_285;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[18],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_285);
  for(int i = 0; i < bitset_285.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_285[i]);
  }

  std::bitset<11> bitset_286;
  trim_vector(State::read(StateHistory::ADCS::gyro_fast_history[19],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_286);
  for(int i = 0; i < bitset_286.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_286[i]);
  }

  std::bitset<11> bitset_287;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[0],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_287);
  for(int i = 0; i < bitset_287.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_287[i]);
  }

  std::bitset<11> bitset_288;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[1],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_288);
  for(int i = 0; i < bitset_288.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_288[i]);
  }

  std::bitset<11> bitset_289;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[2],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_289);
  for(int i = 0; i < bitset_289.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_289[i]);
  }

  std::bitset<11> bitset_290;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[3],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_290);
  for(int i = 0; i < bitset_290.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_290[i]);
  }

  std::bitset<11> bitset_291;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[4],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_291);
  for(int i = 0; i < bitset_291.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_291[i]);
  }

  std::bitset<11> bitset_292;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[5],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_292);
  for(int i = 0; i < bitset_292.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_292[i]);
  }

  std::bitset<11> bitset_293;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[6],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_293);
  for(int i = 0; i < bitset_293.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_293[i]);
  }

  std::bitset<11> bitset_294;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[7],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_294);
  for(int i = 0; i < bitset_294.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_294[i]);
  }

  std::bitset<11> bitset_295;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[8],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_295);
  for(int i = 0; i < bitset_295.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_295[i]);
  }

  std::bitset<11> bitset_296;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[9],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_296);
  for(int i = 0; i < bitset_296.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_296[i]);
  }

  std::bitset<11> bitset_297;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[10],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_297);
  for(int i = 0; i < bitset_297.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_297[i]);
  }

  std::bitset<11> bitset_298;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[11],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_298);
  for(int i = 0; i < bitset_298.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_298[i]);
  }

  std::bitset<11> bitset_299;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[12],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_299);
  for(int i = 0; i < bitset_299.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_299[i]);
  }

  std::bitset<11> bitset_300;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[13],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_300);
  for(int i = 0; i < bitset_300.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_300[i]);
  }

  std::bitset<11> bitset_301;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[14],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_301);
  for(int i = 0; i < bitset_301.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_301[i]);
  }

  std::bitset<11> bitset_302;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[15],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_302);
  for(int i = 0; i < bitset_302.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_302[i]);
  }

  std::bitset<11> bitset_303;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[16],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_303);
  for(int i = 0; i < bitset_303.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_303[i]);
  }

  std::bitset<11> bitset_304;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[17],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_304);
  for(int i = 0; i < bitset_304.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_304[i]);
  }

  std::bitset<11> bitset_305;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[18],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_305);
  for(int i = 0; i < bitset_305.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_305[i]);
  }

  std::bitset<11> bitset_306;
  trim_vector(State::read(StateHistory::ADCS::magnetometer_history[19],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_306);
  for(int i = 0; i < bitset_306.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_306[i]);
  }

  std::bitset<11> bitset_307;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[0],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_307);
  for(int i = 0; i < bitset_307.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_307[i]);
  }

  std::bitset<11> bitset_308;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[1],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_308);
  for(int i = 0; i < bitset_308.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_308[i]);
  }

  std::bitset<11> bitset_309;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[2],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_309);
  for(int i = 0; i < bitset_309.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_309[i]);
  }

  std::bitset<11> bitset_310;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[3],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_310);
  for(int i = 0; i < bitset_310.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_310[i]);
  }

  std::bitset<11> bitset_311;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[4],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_311);
  for(int i = 0; i < bitset_311.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_311[i]);
  }

  std::bitset<11> bitset_312;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[5],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_312);
  for(int i = 0; i < bitset_312.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_312[i]);
  }

  std::bitset<11> bitset_313;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[6],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_313);
  for(int i = 0; i < bitset_313.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_313[i]);
  }

  std::bitset<11> bitset_314;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[7],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_314);
  for(int i = 0; i < bitset_314.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_314[i]);
  }

  std::bitset<11> bitset_315;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[8],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_315);
  for(int i = 0; i < bitset_315.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_315[i]);
  }

  std::bitset<11> bitset_316;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[9],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_316);
  for(int i = 0; i < bitset_316.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_316[i]);
  }

  std::bitset<11> bitset_317;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[10],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_317);
  for(int i = 0; i < bitset_317.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_317[i]);
  }

  std::bitset<11> bitset_318;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[11],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_318);
  for(int i = 0; i < bitset_318.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_318[i]);
  }

  std::bitset<11> bitset_319;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[12],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_319);
  for(int i = 0; i < bitset_319.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_319[i]);
  }

  std::bitset<11> bitset_320;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[13],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_320);
  for(int i = 0; i < bitset_320.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_320[i]);
  }

  std::bitset<11> bitset_321;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[14],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_321);
  for(int i = 0; i < bitset_321.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_321[i]);
  }

  std::bitset<11> bitset_322;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[15],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_322);
  for(int i = 0; i < bitset_322.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_322[i]);
  }

  std::bitset<11> bitset_323;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[16],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_323);
  for(int i = 0; i < bitset_323.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_323[i]);
  }

  std::bitset<11> bitset_324;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[17],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_324);
  for(int i = 0; i < bitset_324.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_324[i]);
  }

  std::bitset<11> bitset_325;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[18],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_325);
  for(int i = 0; i < bitset_325.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_325[i]);
  }

  std::bitset<11> bitset_326;
  trim_vector(State::read(StateHistory::ADCS::rwa_ramp_cmd_history[19],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_326);
  for(int i = 0; i < bitset_326.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_326[i]);
  }

  std::bitset<11> bitset_327;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[0],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_327);
  for(int i = 0; i < bitset_327.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_327[i]);
  }

  std::bitset<11> bitset_328;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[1],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_328);
  for(int i = 0; i < bitset_328.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_328[i]);
  }

  std::bitset<11> bitset_329;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[2],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_329);
  for(int i = 0; i < bitset_329.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_329[i]);
  }

  std::bitset<11> bitset_330;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[3],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_330);
  for(int i = 0; i < bitset_330.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_330[i]);
  }

  std::bitset<11> bitset_331;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[4],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_331);
  for(int i = 0; i < bitset_331.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_331[i]);
  }

  std::bitset<11> bitset_332;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[5],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_332);
  for(int i = 0; i < bitset_332.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_332[i]);
  }

  std::bitset<11> bitset_333;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[6],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_333);
  for(int i = 0; i < bitset_333.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_333[i]);
  }

  std::bitset<11> bitset_334;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[7],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_334);
  for(int i = 0; i < bitset_334.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_334[i]);
  }

  std::bitset<11> bitset_335;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[8],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_335);
  for(int i = 0; i < bitset_335.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_335[i]);
  }

  std::bitset<11> bitset_336;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[9],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_336);
  for(int i = 0; i < bitset_336.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_336[i]);
  }

  std::bitset<11> bitset_337;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[10],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_337);
  for(int i = 0; i < bitset_337.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_337[i]);
  }

  std::bitset<11> bitset_338;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[11],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_338);
  for(int i = 0; i < bitset_338.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_338[i]);
  }

  std::bitset<11> bitset_339;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[12],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_339);
  for(int i = 0; i < bitset_339.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_339[i]);
  }

  std::bitset<11> bitset_340;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[13],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_340);
  for(int i = 0; i < bitset_340.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_340[i]);
  }

  std::bitset<11> bitset_341;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[14],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_341);
  for(int i = 0; i < bitset_341.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_341[i]);
  }

  std::bitset<11> bitset_342;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[15],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_342);
  for(int i = 0; i < bitset_342.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_342[i]);
  }

  std::bitset<11> bitset_343;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[16],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_343);
  for(int i = 0; i < bitset_343.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_343[i]);
  }

  std::bitset<11> bitset_344;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[17],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_344);
  for(int i = 0; i < bitset_344.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_344[i]);
  }

  std::bitset<11> bitset_345;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[18],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_345);
  for(int i = 0; i < bitset_345.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_345[i]);
  }

  std::bitset<11> bitset_346;
  trim_vector(State::read(StateHistory::ADCS::mtr_cmd_history[19],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_346);
  for(int i = 0; i < bitset_346.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_346[i]);
  }

  std::bitset<11> bitset_347;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[0],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_347);
  for(int i = 0; i < bitset_347.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_347[i]);
  }

  std::bitset<11> bitset_348;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[1],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_348);
  for(int i = 0; i < bitset_348.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_348[i]);
  }

  std::bitset<11> bitset_349;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[2],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_349);
  for(int i = 0; i < bitset_349.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_349[i]);
  }

  std::bitset<11> bitset_350;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[3],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_350);
  for(int i = 0; i < bitset_350.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_350[i]);
  }

  std::bitset<11> bitset_351;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[4],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_351);
  for(int i = 0; i < bitset_351.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_351[i]);
  }

  std::bitset<11> bitset_352;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[5],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_352);
  for(int i = 0; i < bitset_352.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_352[i]);
  }

  std::bitset<11> bitset_353;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[6],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_353);
  for(int i = 0; i < bitset_353.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_353[i]);
  }

  std::bitset<11> bitset_354;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[7],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_354);
  for(int i = 0; i < bitset_354.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_354[i]);
  }

  std::bitset<11> bitset_355;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[8],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_355);
  for(int i = 0; i < bitset_355.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_355[i]);
  }

  std::bitset<11> bitset_356;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[9],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_356);
  for(int i = 0; i < bitset_356.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_356[i]);
  }

  std::bitset<11> bitset_357;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[10],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_357);
  for(int i = 0; i < bitset_357.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_357[i]);
  }

  std::bitset<11> bitset_358;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[11],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_358);
  for(int i = 0; i < bitset_358.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_358[i]);
  }

  std::bitset<11> bitset_359;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[12],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_359);
  for(int i = 0; i < bitset_359.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_359[i]);
  }

  std::bitset<11> bitset_360;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[13],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_360);
  for(int i = 0; i < bitset_360.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_360[i]);
  }

  std::bitset<11> bitset_361;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[14],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_361);
  for(int i = 0; i < bitset_361.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_361[i]);
  }

  std::bitset<11> bitset_362;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[15],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_362);
  for(int i = 0; i < bitset_362.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_362[i]);
  }

  std::bitset<11> bitset_363;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[16],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_363);
  for(int i = 0; i < bitset_363.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_363[i]);
  }

  std::bitset<11> bitset_364;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[17],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_364);
  for(int i = 0; i < bitset_364.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_364[i]);
  }

  std::bitset<11> bitset_365;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[18],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_365);
  for(int i = 0; i < bitset_365.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_365[i]);
  }

  std::bitset<11> bitset_366;
  trim_vector(State::read(StateHistory::ADCS::ssa_vector_history[19],StateHistory::ADCS::story::ADCS_state_lock), 0, 0, &bitset_366);
  for(int i = 0; i < bitset_366.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_366[i]);
  }

  std::bitset<11> bitset_367;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[0],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_367);
  for(int i = 0; i < bitset_367.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_367[i]);
  }

  std::bitset<11> bitset_368;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[1],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_368);
  for(int i = 0; i < bitset_368.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_368[i]);
  }

  std::bitset<11> bitset_369;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[2],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_369);
  for(int i = 0; i < bitset_369.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_369[i]);
  }

  std::bitset<11> bitset_370;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[3],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_370);
  for(int i = 0; i < bitset_370.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_370[i]);
  }

  std::bitset<11> bitset_371;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[4],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_371);
  for(int i = 0; i < bitset_371.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_371[i]);
  }

  std::bitset<11> bitset_372;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[5],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_372);
  for(int i = 0; i < bitset_372.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_372[i]);
  }

  std::bitset<11> bitset_373;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[6],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_373);
  for(int i = 0; i < bitset_373.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_373[i]);
  }

  std::bitset<11> bitset_374;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[7],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_374);
  for(int i = 0; i < bitset_374.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_374[i]);
  }

  std::bitset<11> bitset_375;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[8],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_375);
  for(int i = 0; i < bitset_375.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_375[i]);
  }

  std::bitset<11> bitset_376;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[9],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_376);
  for(int i = 0; i < bitset_376.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_376[i]);
  }

  std::bitset<11> bitset_377;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[10],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_377);
  for(int i = 0; i < bitset_377.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_377[i]);
  }

  std::bitset<11> bitset_378;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[11],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_378);
  for(int i = 0; i < bitset_378.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_378[i]);
  }

  std::bitset<11> bitset_379;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[12],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_379);
  for(int i = 0; i < bitset_379.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_379[i]);
  }

  std::bitset<11> bitset_380;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[13],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_380);
  for(int i = 0; i < bitset_380.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_380[i]);
  }

  std::bitset<11> bitset_381;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[14],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_381);
  for(int i = 0; i < bitset_381.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_381[i]);
  }

  std::bitset<11> bitset_382;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[15],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_382);
  for(int i = 0; i < bitset_382.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_382[i]);
  }

  std::bitset<11> bitset_383;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[16],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_383);
  for(int i = 0; i < bitset_383.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_383[i]);
  }

  std::bitset<11> bitset_384;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[17],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_384);
  for(int i = 0; i < bitset_384.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_384[i]);
  }

  std::bitset<11> bitset_385;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[18],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_385);
  for(int i = 0; i < bitset_385.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_385[i]);
  }

  std::bitset<11> bitset_386;
  trim_float(State::read(StateHistory::Propulsion::tank_pressure_history[19],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_386);
  for(int i = 0; i < bitset_386.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_386[i]);
  }

  std::bitset<11> bitset_387;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[0],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_387);
  for(int i = 0; i < bitset_387.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_387[i]);
  }

  std::bitset<11> bitset_388;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[1],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_388);
  for(int i = 0; i < bitset_388.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_388[i]);
  }

  std::bitset<11> bitset_389;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[2],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_389);
  for(int i = 0; i < bitset_389.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_389[i]);
  }

  std::bitset<11> bitset_390;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[3],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_390);
  for(int i = 0; i < bitset_390.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_390[i]);
  }

  std::bitset<11> bitset_391;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[4],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_391);
  for(int i = 0; i < bitset_391.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_391[i]);
  }

  std::bitset<11> bitset_392;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[5],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_392);
  for(int i = 0; i < bitset_392.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_392[i]);
  }

  std::bitset<11> bitset_393;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[6],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_393);
  for(int i = 0; i < bitset_393.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_393[i]);
  }

  std::bitset<11> bitset_394;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[7],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_394);
  for(int i = 0; i < bitset_394.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_394[i]);
  }

  std::bitset<11> bitset_395;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[8],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_395);
  for(int i = 0; i < bitset_395.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_395[i]);
  }

  std::bitset<11> bitset_396;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[9],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_396);
  for(int i = 0; i < bitset_396.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_396[i]);
  }

  std::bitset<11> bitset_397;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[10],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_397);
  for(int i = 0; i < bitset_397.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_397[i]);
  }

  std::bitset<11> bitset_398;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[11],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_398);
  for(int i = 0; i < bitset_398.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_398[i]);
  }

  std::bitset<11> bitset_399;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[12],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_399);
  for(int i = 0; i < bitset_399.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_399[i]);
  }

  std::bitset<11> bitset_400;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[13],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_400);
  for(int i = 0; i < bitset_400.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_400[i]);
  }

  std::bitset<11> bitset_401;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[14],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_401);
  for(int i = 0; i < bitset_401.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_401[i]);
  }

  std::bitset<11> bitset_402;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[15],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_402);
  for(int i = 0; i < bitset_402.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_402[i]);
  }

  std::bitset<11> bitset_403;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[16],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_403);
  for(int i = 0; i < bitset_403.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_403[i]);
  }

  std::bitset<11> bitset_404;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[17],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_404);
  for(int i = 0; i < bitset_404.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_404[i]);
  }

  std::bitset<11> bitset_405;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[18],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_405);
  for(int i = 0; i < bitset_405.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_405[i]);
  }

  std::bitset<11> bitset_406;
  trim_float(State::read(StateHistory::Propulsion::inner_tank_temperature_history[19],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_406);
  for(int i = 0; i < bitset_406.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_406[i]);
  }

  std::bitset<11> bitset_407;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[0],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_407);
  for(int i = 0; i < bitset_407.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_407[i]);
  }

  std::bitset<11> bitset_408;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[1],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_408);
  for(int i = 0; i < bitset_408.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_408[i]);
  }

  std::bitset<11> bitset_409;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[2],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_409);
  for(int i = 0; i < bitset_409.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_409[i]);
  }

  std::bitset<11> bitset_410;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[3],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_410);
  for(int i = 0; i < bitset_410.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_410[i]);
  }

  std::bitset<11> bitset_411;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[4],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_411);
  for(int i = 0; i < bitset_411.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_411[i]);
  }

  std::bitset<11> bitset_412;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[5],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_412);
  for(int i = 0; i < bitset_412.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_412[i]);
  }

  std::bitset<11> bitset_413;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[6],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_413);
  for(int i = 0; i < bitset_413.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_413[i]);
  }

  std::bitset<11> bitset_414;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[7],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_414);
  for(int i = 0; i < bitset_414.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_414[i]);
  }

  std::bitset<11> bitset_415;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[8],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_415);
  for(int i = 0; i < bitset_415.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_415[i]);
  }

  std::bitset<11> bitset_416;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[9],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_416);
  for(int i = 0; i < bitset_416.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_416[i]);
  }

  std::bitset<11> bitset_417;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[10],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_417);
  for(int i = 0; i < bitset_417.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_417[i]);
  }

  std::bitset<11> bitset_418;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[11],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_418);
  for(int i = 0; i < bitset_418.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_418[i]);
  }

  std::bitset<11> bitset_419;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[12],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_419);
  for(int i = 0; i < bitset_419.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_419[i]);
  }

  std::bitset<11> bitset_420;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[13],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_420);
  for(int i = 0; i < bitset_420.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_420[i]);
  }

  std::bitset<11> bitset_421;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[14],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_421);
  for(int i = 0; i < bitset_421.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_421[i]);
  }

  std::bitset<11> bitset_422;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[15],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_422);
  for(int i = 0; i < bitset_422.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_422[i]);
  }

  std::bitset<11> bitset_423;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[16],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_423);
  for(int i = 0; i < bitset_423.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_423[i]);
  }

  std::bitset<11> bitset_424;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[17],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_424);
  for(int i = 0; i < bitset_424.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_424[i]);
  }

  std::bitset<11> bitset_425;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[18],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_425);
  for(int i = 0; i < bitset_425.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_425[i]);
  }

  std::bitset<11> bitset_426;
  trim_float(State::read(StateHistory::Propulsion::outer_tank_temperature_history[19],StateHistory::Propulsion::story::Propulsion_state_lock), 0, 0, &bitset_426);
  for(int i = 0; i < bitset_426.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_426[i]);
  }

  std::bitset<11> bitset_427;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[0],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_427);
  for(int i = 0; i < bitset_427.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_427[i]);
  }

  std::bitset<11> bitset_428;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[1],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_428);
  for(int i = 0; i < bitset_428.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_428[i]);
  }

  std::bitset<11> bitset_429;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[2],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_429);
  for(int i = 0; i < bitset_429.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_429[i]);
  }

  std::bitset<11> bitset_430;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[3],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_430);
  for(int i = 0; i < bitset_430.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_430[i]);
  }

  std::bitset<11> bitset_431;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[4],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_431);
  for(int i = 0; i < bitset_431.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_431[i]);
  }

  std::bitset<11> bitset_432;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[5],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_432);
  for(int i = 0; i < bitset_432.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_432[i]);
  }

  std::bitset<11> bitset_433;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[6],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_433);
  for(int i = 0; i < bitset_433.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_433[i]);
  }

  std::bitset<11> bitset_434;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[7],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_434);
  for(int i = 0; i < bitset_434.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_434[i]);
  }

  std::bitset<11> bitset_435;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[8],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_435);
  for(int i = 0; i < bitset_435.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_435[i]);
  }

  std::bitset<11> bitset_436;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[9],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_436);
  for(int i = 0; i < bitset_436.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_436[i]);
  }

  std::bitset<11> bitset_437;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[10],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_437);
  for(int i = 0; i < bitset_437.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_437[i]);
  }

  std::bitset<11> bitset_438;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[11],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_438);
  for(int i = 0; i < bitset_438.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_438[i]);
  }

  std::bitset<11> bitset_439;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[12],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_439);
  for(int i = 0; i < bitset_439.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_439[i]);
  }

  std::bitset<11> bitset_440;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[13],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_440);
  for(int i = 0; i < bitset_440.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_440[i]);
  }

  std::bitset<11> bitset_441;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[14],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_441);
  for(int i = 0; i < bitset_441.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_441[i]);
  }

  std::bitset<11> bitset_442;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[15],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_442);
  for(int i = 0; i < bitset_442.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_442[i]);
  }

  std::bitset<11> bitset_443;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[16],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_443);
  for(int i = 0; i < bitset_443.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_443[i]);
  }

  std::bitset<11> bitset_444;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[17],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_444);
  for(int i = 0; i < bitset_444.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_444[i]);
  }

  std::bitset<11> bitset_445;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[18],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_445);
  for(int i = 0; i < bitset_445.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_445[i]);
  }

  std::bitset<11> bitset_446;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_history[19],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_446);
  for(int i = 0; i < bitset_446.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_446[i]);
  }

  std::bitset<11> bitset_447;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[0],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_447);
  for(int i = 0; i < bitset_447.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_447[i]);
  }

  std::bitset<11> bitset_448;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[1],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_448);
  for(int i = 0; i < bitset_448.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_448[i]);
  }

  std::bitset<11> bitset_449;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[2],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_449);
  for(int i = 0; i < bitset_449.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_449[i]);
  }

  std::bitset<11> bitset_450;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[3],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_450);
  for(int i = 0; i < bitset_450.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_450[i]);
  }

  std::bitset<11> bitset_451;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[4],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_451);
  for(int i = 0; i < bitset_451.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_451[i]);
  }

  std::bitset<11> bitset_452;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[5],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_452);
  for(int i = 0; i < bitset_452.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_452[i]);
  }

  std::bitset<11> bitset_453;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[6],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_453);
  for(int i = 0; i < bitset_453.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_453[i]);
  }

  std::bitset<11> bitset_454;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[7],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_454);
  for(int i = 0; i < bitset_454.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_454[i]);
  }

  std::bitset<11> bitset_455;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[8],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_455);
  for(int i = 0; i < bitset_455.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_455[i]);
  }

  std::bitset<11> bitset_456;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[9],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_456);
  for(int i = 0; i < bitset_456.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_456[i]);
  }

  std::bitset<11> bitset_457;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[10],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_457);
  for(int i = 0; i < bitset_457.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_457[i]);
  }

  std::bitset<11> bitset_458;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[11],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_458);
  for(int i = 0; i < bitset_458.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_458[i]);
  }

  std::bitset<11> bitset_459;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[12],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_459);
  for(int i = 0; i < bitset_459.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_459[i]);
  }

  std::bitset<11> bitset_460;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[13],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_460);
  for(int i = 0; i < bitset_460.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_460[i]);
  }

  std::bitset<11> bitset_461;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[14],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_461);
  for(int i = 0; i < bitset_461.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_461[i]);
  }

  std::bitset<11> bitset_462;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[15],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_462);
  for(int i = 0; i < bitset_462.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_462[i]);
  }

  std::bitset<11> bitset_463;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[16],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_463);
  for(int i = 0; i < bitset_463.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_463[i]);
  }

  std::bitset<11> bitset_464;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[17],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_464);
  for(int i = 0; i < bitset_464.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_464[i]);
  }

  std::bitset<11> bitset_465;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[18],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_465);
  for(int i = 0; i < bitset_465.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_465[i]);
  }

  std::bitset<11> bitset_466;
  trim_vector(State::read(StateHistory::Piksi::recorded_velocity_history[19],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_466);
  for(int i = 0; i < bitset_466.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_466[i]);
  }

  std::bitset<11> bitset_467;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[0],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_467);
  for(int i = 0; i < bitset_467.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_467[i]);
  }

  std::bitset<11> bitset_468;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[1],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_468);
  for(int i = 0; i < bitset_468.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_468[i]);
  }

  std::bitset<11> bitset_469;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[2],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_469);
  for(int i = 0; i < bitset_469.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_469[i]);
  }

  std::bitset<11> bitset_470;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[3],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_470);
  for(int i = 0; i < bitset_470.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_470[i]);
  }

  std::bitset<11> bitset_471;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[4],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_471);
  for(int i = 0; i < bitset_471.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_471[i]);
  }

  std::bitset<11> bitset_472;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[5],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_472);
  for(int i = 0; i < bitset_472.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_472[i]);
  }

  std::bitset<11> bitset_473;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[6],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_473);
  for(int i = 0; i < bitset_473.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_473[i]);
  }

  std::bitset<11> bitset_474;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[7],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_474);
  for(int i = 0; i < bitset_474.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_474[i]);
  }

  std::bitset<11> bitset_475;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[8],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_475);
  for(int i = 0; i < bitset_475.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_475[i]);
  }

  std::bitset<11> bitset_476;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[9],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_476);
  for(int i = 0; i < bitset_476.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_476[i]);
  }

  std::bitset<11> bitset_477;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[10],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_477);
  for(int i = 0; i < bitset_477.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_477[i]);
  }

  std::bitset<11> bitset_478;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[11],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_478);
  for(int i = 0; i < bitset_478.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_478[i]);
  }

  std::bitset<11> bitset_479;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[12],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_479);
  for(int i = 0; i < bitset_479.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_479[i]);
  }

  std::bitset<11> bitset_480;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[13],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_480);
  for(int i = 0; i < bitset_480.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_480[i]);
  }

  std::bitset<11> bitset_481;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[14],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_481);
  for(int i = 0; i < bitset_481.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_481[i]);
  }

  std::bitset<11> bitset_482;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[15],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_482);
  for(int i = 0; i < bitset_482.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_482[i]);
  }

  std::bitset<11> bitset_483;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[16],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_483);
  for(int i = 0; i < bitset_483.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_483[i]);
  }

  std::bitset<11> bitset_484;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[17],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_484);
  for(int i = 0; i < bitset_484.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_484[i]);
  }

  std::bitset<11> bitset_485;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[18],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_485);
  for(int i = 0; i < bitset_485.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_485[i]);
  }

  std::bitset<11> bitset_486;
  trim_vector(State::read(StateHistory::Piksi::recorded_position_other_history[19],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_486);
  for(int i = 0; i < bitset_486.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_486[i]);
  }

  std::bitset<11> bitset_487;
  trim_vector(State::read(StateHistory::Piksi::position_history[0],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_487);
  for(int i = 0; i < bitset_487.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_487[i]);
  }

  std::bitset<11> bitset_488;
  trim_vector(State::read(StateHistory::Piksi::position_history[1],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_488);
  for(int i = 0; i < bitset_488.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_488[i]);
  }

  std::bitset<11> bitset_489;
  trim_vector(State::read(StateHistory::Piksi::position_history[2],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_489);
  for(int i = 0; i < bitset_489.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_489[i]);
  }

  std::bitset<11> bitset_490;
  trim_vector(State::read(StateHistory::Piksi::position_history[3],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_490);
  for(int i = 0; i < bitset_490.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_490[i]);
  }

  std::bitset<11> bitset_491;
  trim_vector(State::read(StateHistory::Piksi::position_history[4],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_491);
  for(int i = 0; i < bitset_491.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_491[i]);
  }

  std::bitset<11> bitset_492;
  trim_vector(State::read(StateHistory::Piksi::position_history[5],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_492);
  for(int i = 0; i < bitset_492.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_492[i]);
  }

  std::bitset<11> bitset_493;
  trim_vector(State::read(StateHistory::Piksi::position_history[6],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_493);
  for(int i = 0; i < bitset_493.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_493[i]);
  }

  std::bitset<11> bitset_494;
  trim_vector(State::read(StateHistory::Piksi::position_history[7],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_494);
  for(int i = 0; i < bitset_494.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_494[i]);
  }

  std::bitset<11> bitset_495;
  trim_vector(State::read(StateHistory::Piksi::position_history[8],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_495);
  for(int i = 0; i < bitset_495.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_495[i]);
  }

  std::bitset<11> bitset_496;
  trim_vector(State::read(StateHistory::Piksi::position_history[9],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_496);
  for(int i = 0; i < bitset_496.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_496[i]);
  }

  std::bitset<11> bitset_497;
  trim_vector(State::read(StateHistory::Piksi::position_history[10],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_497);
  for(int i = 0; i < bitset_497.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_497[i]);
  }

  std::bitset<11> bitset_498;
  trim_vector(State::read(StateHistory::Piksi::position_history[11],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_498);
  for(int i = 0; i < bitset_498.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_498[i]);
  }

  std::bitset<11> bitset_499;
  trim_vector(State::read(StateHistory::Piksi::position_history[12],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_499);
  for(int i = 0; i < bitset_499.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_499[i]);
  }

  std::bitset<11> bitset_500;
  trim_vector(State::read(StateHistory::Piksi::position_history[13],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_500);
  for(int i = 0; i < bitset_500.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_500[i]);
  }

  std::bitset<11> bitset_501;
  trim_vector(State::read(StateHistory::Piksi::position_history[14],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_501);
  for(int i = 0; i < bitset_501.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_501[i]);
  }

  std::bitset<11> bitset_502;
  trim_vector(State::read(StateHistory::Piksi::position_history[15],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_502);
  for(int i = 0; i < bitset_502.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_502[i]);
  }

  std::bitset<11> bitset_503;
  trim_vector(State::read(StateHistory::Piksi::position_history[16],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_503);
  for(int i = 0; i < bitset_503.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_503[i]);
  }

  std::bitset<11> bitset_504;
  trim_vector(State::read(StateHistory::Piksi::position_history[17],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_504);
  for(int i = 0; i < bitset_504.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_504[i]);
  }

  std::bitset<11> bitset_505;
  trim_vector(State::read(StateHistory::Piksi::position_history[18],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_505);
  for(int i = 0; i < bitset_505.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_505[i]);
  }

  std::bitset<11> bitset_506;
  trim_vector(State::read(StateHistory::Piksi::position_history[19],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_506);
  for(int i = 0; i < bitset_506.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_506[i]);
  }

  std::bitset<11> bitset_507;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[0],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_507);
  for(int i = 0; i < bitset_507.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_507[i]);
  }

  std::bitset<11> bitset_508;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[1],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_508);
  for(int i = 0; i < bitset_508.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_508[i]);
  }

  std::bitset<11> bitset_509;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[2],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_509);
  for(int i = 0; i < bitset_509.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_509[i]);
  }

  std::bitset<11> bitset_510;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[3],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_510);
  for(int i = 0; i < bitset_510.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_510[i]);
  }

  std::bitset<11> bitset_511;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[4],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_511);
  for(int i = 0; i < bitset_511.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_511[i]);
  }

  std::bitset<11> bitset_512;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[5],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_512);
  for(int i = 0; i < bitset_512.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_512[i]);
  }

  std::bitset<11> bitset_513;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[6],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_513);
  for(int i = 0; i < bitset_513.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_513[i]);
  }

  std::bitset<11> bitset_514;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[7],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_514);
  for(int i = 0; i < bitset_514.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_514[i]);
  }

  std::bitset<11> bitset_515;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[8],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_515);
  for(int i = 0; i < bitset_515.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_515[i]);
  }

  std::bitset<11> bitset_516;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[9],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_516);
  for(int i = 0; i < bitset_516.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_516[i]);
  }

  std::bitset<11> bitset_517;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[10],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_517);
  for(int i = 0; i < bitset_517.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_517[i]);
  }

  std::bitset<11> bitset_518;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[11],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_518);
  for(int i = 0; i < bitset_518.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_518[i]);
  }

  std::bitset<11> bitset_519;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[12],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_519);
  for(int i = 0; i < bitset_519.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_519[i]);
  }

  std::bitset<11> bitset_520;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[13],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_520);
  for(int i = 0; i < bitset_520.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_520[i]);
  }

  std::bitset<11> bitset_521;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[14],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_521);
  for(int i = 0; i < bitset_521.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_521[i]);
  }

  std::bitset<11> bitset_522;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[15],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_522);
  for(int i = 0; i < bitset_522.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_522[i]);
  }

  std::bitset<11> bitset_523;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[16],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_523);
  for(int i = 0; i < bitset_523.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_523[i]);
  }

  std::bitset<11> bitset_524;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[17],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_524);
  for(int i = 0; i < bitset_524.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_524[i]);
  }

  std::bitset<11> bitset_525;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[18],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_525);
  for(int i = 0; i < bitset_525.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_525[i]);
  }

  std::bitset<11> bitset_526;
  trim_vector(State::read(StateHistory::Piksi::velocity_history[19],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_526);
  for(int i = 0; i < bitset_526.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_526[i]);
  }

  std::bitset<11> bitset_527;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[0],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_527);
  for(int i = 0; i < bitset_527.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_527[i]);
  }

  std::bitset<11> bitset_528;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[1],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_528);
  for(int i = 0; i < bitset_528.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_528[i]);
  }

  std::bitset<11> bitset_529;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[2],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_529);
  for(int i = 0; i < bitset_529.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_529[i]);
  }

  std::bitset<11> bitset_530;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[3],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_530);
  for(int i = 0; i < bitset_530.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_530[i]);
  }

  std::bitset<11> bitset_531;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[4],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_531);
  for(int i = 0; i < bitset_531.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_531[i]);
  }

  std::bitset<11> bitset_532;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[5],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_532);
  for(int i = 0; i < bitset_532.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_532[i]);
  }

  std::bitset<11> bitset_533;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[6],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_533);
  for(int i = 0; i < bitset_533.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_533[i]);
  }

  std::bitset<11> bitset_534;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[7],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_534);
  for(int i = 0; i < bitset_534.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_534[i]);
  }

  std::bitset<11> bitset_535;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[8],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_535);
  for(int i = 0; i < bitset_535.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_535[i]);
  }

  std::bitset<11> bitset_536;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[9],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_536);
  for(int i = 0; i < bitset_536.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_536[i]);
  }

  std::bitset<11> bitset_537;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[10],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_537);
  for(int i = 0; i < bitset_537.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_537[i]);
  }

  std::bitset<11> bitset_538;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[11],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_538);
  for(int i = 0; i < bitset_538.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_538[i]);
  }

  std::bitset<11> bitset_539;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[12],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_539);
  for(int i = 0; i < bitset_539.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_539[i]);
  }

  std::bitset<11> bitset_540;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[13],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_540);
  for(int i = 0; i < bitset_540.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_540[i]);
  }

  std::bitset<11> bitset_541;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[14],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_541);
  for(int i = 0; i < bitset_541.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_541[i]);
  }

  std::bitset<11> bitset_542;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[15],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_542);
  for(int i = 0; i < bitset_542.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_542[i]);
  }

  std::bitset<11> bitset_543;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[16],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_543);
  for(int i = 0; i < bitset_543.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_543[i]);
  }

  std::bitset<11> bitset_544;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[17],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_544);
  for(int i = 0; i < bitset_544.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_544[i]);
  }

  std::bitset<11> bitset_545;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[18],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_545);
  for(int i = 0; i < bitset_545.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_545[i]);
  }

  std::bitset<11> bitset_546;
  trim_vector(State::read(StateHistory::Piksi::position_other_history[19],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_546);
  for(int i = 0; i < bitset_546.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_546[i]);
  }

  std::bitset<11> bitset_547;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[0],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_547);
  for(int i = 0; i < bitset_547.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_547[i]);
  }

  std::bitset<11> bitset_548;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[1],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_548);
  for(int i = 0; i < bitset_548.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_548[i]);
  }

  std::bitset<11> bitset_549;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[2],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_549);
  for(int i = 0; i < bitset_549.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_549[i]);
  }

  std::bitset<11> bitset_550;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[3],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_550);
  for(int i = 0; i < bitset_550.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_550[i]);
  }

  std::bitset<11> bitset_551;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[4],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_551);
  for(int i = 0; i < bitset_551.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_551[i]);
  }

  std::bitset<11> bitset_552;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[5],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_552);
  for(int i = 0; i < bitset_552.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_552[i]);
  }

  std::bitset<11> bitset_553;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[6],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_553);
  for(int i = 0; i < bitset_553.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_553[i]);
  }

  std::bitset<11> bitset_554;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[7],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_554);
  for(int i = 0; i < bitset_554.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_554[i]);
  }

  std::bitset<11> bitset_555;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[8],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_555);
  for(int i = 0; i < bitset_555.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_555[i]);
  }

  std::bitset<11> bitset_556;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[9],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_556);
  for(int i = 0; i < bitset_556.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_556[i]);
  }

  std::bitset<11> bitset_557;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[10],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_557);
  for(int i = 0; i < bitset_557.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_557[i]);
  }

  std::bitset<11> bitset_558;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[11],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_558);
  for(int i = 0; i < bitset_558.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_558[i]);
  }

  std::bitset<11> bitset_559;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[12],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_559);
  for(int i = 0; i < bitset_559.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_559[i]);
  }

  std::bitset<11> bitset_560;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[13],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_560);
  for(int i = 0; i < bitset_560.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_560[i]);
  }

  std::bitset<11> bitset_561;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[14],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_561);
  for(int i = 0; i < bitset_561.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_561[i]);
  }

  std::bitset<11> bitset_562;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[15],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_562);
  for(int i = 0; i < bitset_562.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_562[i]);
  }

  std::bitset<11> bitset_563;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[16],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_563);
  for(int i = 0; i < bitset_563.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_563[i]);
  }

  std::bitset<11> bitset_564;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[17],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_564);
  for(int i = 0; i < bitset_564.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_564[i]);
  }

  std::bitset<11> bitset_565;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[18],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_565);
  for(int i = 0; i < bitset_565.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_565[i]);
  }

  std::bitset<11> bitset_566;
  trim_vector(State::read(StateHistory::Piksi::velocity_other_history[19],StateHistory::Piksi::story::Piksi_state_lock), 0, 0, &bitset_566);
  for(int i = 0; i < bitset_566.size(); i++) {
    packet_0.set(packet_0_ptr++,bitset_566[i]);
  }
  unsigned int packet_1_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_1.set(packet_1_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(1);
  for(int i = 0; i < 8; i++) {
    packet_1.set(packet_1_ptr++, packet_num_repr[i]);
  }
  unsigned int packet_2_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_2.set(packet_2_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(2);
  for(int i = 0; i < 8; i++) {
    packet_2.set(packet_2_ptr++, packet_num_repr[i]);
  }
  unsigned int packet_3_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_3.set(packet_3_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(3);
  for(int i = 0; i < 8; i++) {
    packet_3.set(packet_3_ptr++, packet_num_repr[i]);
  }
  unsigned int packet_4_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_4.set(packet_4_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(4);
  for(int i = 0; i < 8; i++) {
    packet_4.set(packet_4_ptr++, packet_num_repr[i]);
  }
  unsigned int packet_5_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_5.set(packet_5_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(5);
  for(int i = 0; i < 8; i++) {
    packet_5.set(packet_5_ptr++, packet_num_repr[i]);
  }
  unsigned int packet_6_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_6.set(packet_6_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(6);
  for(int i = 0; i < 8; i++) {
    packet_6.set(packet_6_ptr++, packet_num_repr[i]);
  }
  unsigned int packet_7_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_7.set(packet_7_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(7);
  for(int i = 0; i < 8; i++) {
    packet_7.set(packet_7_ptr++, packet_num_repr[i]);
  }
  unsigned int packet_8_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_8.set(packet_8_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(8);
  for(int i = 0; i < 8; i++) {
    packet_8.set(packet_8_ptr++, packet_num_repr[i]);
  }
  unsigned int packet_9_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_9.set(packet_9_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(9);
  for(int i = 0; i < 8; i++) {
    packet_9.set(packet_9_ptr++, packet_num_repr[i]);
  }
  unsigned int packet_10_ptr;
  std::bitset<32> downlink_num_repr(downlink_no);
  for(int i = 0; i < 32; i++) {
    packet_10.set(packet_10_ptr++, downlink_num_repr[i]);
  }
  std::bitset<8> packet_num_repr(10);
  for(int i = 0; i < 8; i++) {
    packet_10.set(packet_10_ptr++, packet_num_repr[i]);
  }
}
