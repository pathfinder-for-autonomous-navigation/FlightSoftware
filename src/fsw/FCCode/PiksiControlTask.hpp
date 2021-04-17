#pragma once

#include "Drivers/Piksi.hpp"
#include <string>
#include <common/GPSTime.hpp>
#include <common/constant_tracker.hpp>

#include "TimedControlTask.hpp"

#include "piksi_mode_t.enum"

class PiksiControlTask : public TimedControlTask<void>
{
public:
    TRACKED_CONSTANT_SC(unsigned int, DEAD_CYCLE_COUNT, 1000);
    TRACKED_CONSTANT_SC(unsigned int, PIKSI_MD_THRESHOLD, 100000);

    PiksiControlTask(StateFieldRegistry &registry, Devices::Piksi &_piksi);
    
    Devices::Piksi& piksi;
    /** 
    * execute is overriden from TimedControlTask 
    */
    void execute() override;

    // StateField for position, velocity, and baseline
    ReadableStateField<lin::Vector3d> pos_f;
    ReadableStateField<lin::Vector3d> vel_f;
    ReadableStateField<lin::Vector3d> baseline_pos_f;

    // Serializer and StateField for currentState and
    // number of cycles since a good reading
    ReadableStateField<unsigned char> current_state_f;
    ReadableStateField<unsigned int> fix_error_count_f;

    //Serializer and StateField for time
    Serializer<gps_time_t> time_sr;
    ReadableStateField<gps_time_t> time_f;

    //Serializer and StateField for
    // [difference between time of piksi data and current time]
    // in microseconds
    Serializer<unsigned int> microdelta_sr;
    ReadableStateField<unsigned int> microdelta_f;

    // Control cycle of last good Piksi reading
    InternalStateField<unsigned int> last_rtkfix_ccno_f;

protected:
    //Internal Data Containers
    std::array<double, 3> pos;
    std::array<double, 3> vel;
    std::array<double, 3> baseline_pos;

    msg_gps_time_t msg_time;
    gps_time_t time;

    unsigned int iar;

    unsigned int pos_tow;

    unsigned int vel_tow;

    unsigned int baseline_tow;
};
