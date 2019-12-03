#ifndef FIELD_CREATOR_TASK_HPP_
#define FIELD_CREATOR_TASK_HPP_

#include <ControlTask.hpp>

// This class does the unpleasant task of creating state fields that
// controllers expect to see but for which we haven't defined any
// behavior yet.
//
// As flight software develops, this list will grow longer and shorter, but
// eventually become zero.
class FieldCreatorTask : public ControlTask<void> {
    public:
      WritableStateField<unsigned int> adcs_mode_f;
      WritableStateField<f_quat_t> adcs_cmd_attitude_f;
      ReadableStateField<float> adcs_ang_rate_f;
      WritableStateField<float> adcs_min_stable_ang_rate_f;

      InternalStateField<gps_time_t> pan_epoch_f;

      WritableStateField<bool> docking_config_cmd_f;

      InternalStateField<unsigned int> snapshot_size_f;
      InternalStateField<char*> radio_mo_packet_f;
      InternalStateField<char*> radio_mt_packet_f;
      ReadableStateField<signed int> radio_err_f;
      InternalStateField<bool> radio_mt_ready_f;

      FieldCreatorTask(StateFieldRegistry& r) : 
        ControlTask<void>(r),
        adcs_mode_f("adcs.mode", Serializer<unsigned int>(10)),
        adcs_cmd_attitude_f("adcs.cmd_attitude", Serializer<f_quat_t>()),
        adcs_ang_rate_f("adcs.ang_rate", Serializer<float>(0, 10, 4)),
        adcs_min_stable_ang_rate_f("adcs.min_stable_ang_rate", Serializer<float>(0, 10, 4)),
        pan_epoch_f("pan.epoch"),
        docking_config_cmd_f("docksys.config_cmd", Serializer<bool>()),
        snapshot_size_f("downlink_producer.snap_size"),
        radio_mo_packet_f("downlink_producer.mo_ptr"),
        radio_mt_packet_f("uplink_consumer.mt_ptr"),
        radio_err_f("downlink_producer.radio_err_ptr", Serializer<signed int>(-90, 10)),
        radio_mt_ready_f("uplink_consumer.mt_ready")
      {
          // Create the fields!

          // For MissionManager
          add_writable_field(adcs_mode_f);
          add_writable_field(adcs_cmd_attitude_f);
          add_readable_field(adcs_ang_rate_f);
          add_writable_field(adcs_min_stable_ang_rate_f);

          // For DockingController
          add_writable_field(docking_config_cmd_f);

          // For AttitudeEstimator
          add_internal_field(pan_epoch_f);
          pan_epoch_f.set(gps_time_t());

          // For QuakeManager
          add_internal_field(snapshot_size_f);
          add_internal_field(radio_mo_packet_f);
          add_internal_field(radio_mt_packet_f);
          add_readable_field(radio_err_f);
          add_internal_field(radio_mt_ready_f);
          snapshot_size_f.set(350);
          radio_mo_packet_f.set(new char[350]);
          memset(radio_mo_packet_f.get(), 0, 350);
      }

      void execute() {
          // Do nada
      }

      ~FieldCreatorTask() {
        char* buf = radio_mo_packet_f.get();
        delete[] buf;
      }
};

#endif
