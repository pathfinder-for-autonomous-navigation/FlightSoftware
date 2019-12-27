#ifndef FIELD_CREATOR_TASK_HPP_
#define FIELD_CREATOR_TASK_HPP_

#include "ControlTask.hpp"

// This class does the unpleasant task of creating state fields that
// controllers expect to see but for which we haven't defined any
// behavior yet.
//
// As flight software develops, this list will grow longer and shorter, but
// eventually become zero.
class FieldCreatorTask : public ControlTask<void> {
    public:
      WritableStateField<unsigned char> adcs_state_f;
      WritableStateField<f_quat_t> adcs_cmd_attitude_f;
      ReadableStateField<float> adcs_ang_rate_f;
      WritableStateField<float> adcs_min_stable_ang_rate_f;

      WritableStateField<bool> docking_config_cmd_f;

      ReadableStateField<unsigned char> prop_mode_f;
      ReadableStateField<d_vector_t> propagated_baseline_pos_f;

      FieldCreatorTask(StateFieldRegistry& r) : 
        ControlTask<void>(r),
        adcs_state_f("adcs.state", Serializer<unsigned char>(10)),
        adcs_cmd_attitude_f("adcs.cmd_attitude", Serializer<f_quat_t>()),
        adcs_ang_rate_f("adcs.ang_rate", Serializer<float>(0, 10, 4)),
        adcs_min_stable_ang_rate_f("adcs.min_stable_ang_rate", Serializer<float>(0, 10, 4)),
        docking_config_cmd_f("docksys.config_cmd", Serializer<bool>()),
        prop_mode_f("prop.mode", Serializer<unsigned char>(1)),
        propagated_baseline_pos_f("orbit.baseline_pos", Serializer<d_vector_t>(0, 100000, 100))
      {
          // Create the fields!

          // For MissionManager
          add_writable_field(adcs_state_f);
          add_writable_field(adcs_cmd_attitude_f);
          add_readable_field(adcs_ang_rate_f);
          add_writable_field(adcs_min_stable_ang_rate_f);

          // For Docking Controller
          add_writable_field(docking_config_cmd_f);

          // For propulsion controller
          add_readable_field(prop_mode_f);

          // For orbit estimator
          add_readable_field(propagated_baseline_pos_f);
      }

      void execute() {
          // Do nada
      }

      ~FieldCreatorTask() {}
};

#endif
