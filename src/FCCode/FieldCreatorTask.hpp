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
      ReadableStateField<d_vector_t> pos_f;
      ReadableStateField<d_vector_t> pos_baseline_f;

      ReadableStateField<unsigned char> prop_mode_f;

      FieldCreatorTask(StateFieldRegistry& r) : 
        ControlTask<void>(r),
        pos_f("orbit.pos", Serializer<d_vector_t>(0, 100000, 100)),
        pos_baseline_f("orbit.baseline_pos", Serializer<d_vector_t>(0, 100000, 100)),
        prop_mode_f("prop.mode", Serializer<unsigned char>(1))
      {
          // Create the fields!

          // For orbit estimator
          add_readable_field(pos_f);
          add_readable_field(pos_baseline_f);

          // For propulsion controller
          add_readable_field(prop_mode_f);
      }

      void execute() {
          // Do nada
      }

      ~FieldCreatorTask() {}
};

#endif
