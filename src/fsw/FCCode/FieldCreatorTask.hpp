#ifndef FIELD_CREATOR_TASK_HPP_
#define FIELD_CREATOR_TASK_HPP_

#include "ControlTask.hpp"
#include <common/Fault.hpp>

#include <adcs/havt_devices.hpp> // needed for ADCSCommander fill-in
#include <gnc/constants.hpp>

#include <common/GPSTime.hpp>

// This class does the unpleasant task of creating state fields that
// controllers expect to see but for which we haven't defined any
// behavior yet.
//
// As flight software develops, this list will grow longer and shorter, but
// eventually become zero.
class FieldCreatorTask : public ControlTask<void> {
  public:
    ReadableStateField<unsigned int> bootcount_f;

    FieldCreatorTask(StateFieldRegistry& r)
        : ControlTask<void>(r),
          bootcount_f("pan.bootcount",Serializer<unsigned int>(0xfffffff), 1000)
    {
        add_readable_field(bootcount_f);
    }

    ~FieldCreatorTask() = default;

    void execute()
    { }
};

#endif
