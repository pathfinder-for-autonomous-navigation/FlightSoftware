#pragma once

#include "Drivers/Piksi.hpp"
#include <string>
#include <common/GPSTime.hpp>

#include "TimedControlTask.hpp"

#include "piksi_mode_t.enum"

class PiksiControlTask : public TimedControlTask<void>
{
public:
    static constexpr uint32_t DEAD_CYCLE_COUNT = 1000;

    PiksiControlTask(StateFieldRegistry &registry, uint32_t offset, Devices::Piksi &_piksi);
    
    Devices::Piksi& piksi;
    /** 
    * execute is overriden from TimedControlTask 
    */
    void execute() override;

    // StateField for position, velocity, and baseline
    ReadableStateField<d_vector_t> pos_f;
    ReadableStateField<d_vector_t> vel_f;
    ReadableStateField<d_vector_t> baseline_pos_f;

    // Serializer and StateField for currentState and
    // number of cycles since a good reading
    ReadableStateField<uint32_t> current_state_f;
    ReadableStateField<uint32_t> fix_error_count_f;

    //Serializer and StateField for time
    Serializer<gps_time_t> time_sr;
    ReadableStateField<gps_time_t> time_f;

    // System time of last good Piksi reading
    InternalStateField<sys_time_t> last_fix_time_f;

protected:
    //Internal Data Containers
    std::array<double, 3> pos;
    std::array<double, 3> vel;
    std::array<double, 3> baseline_pos;

    msg_gps_time_t msg_time {0,0,0,0};
    gps_time_t time;

    uint32_t iar = 0;

    uint32_t pos_tow = 0;

    uint32_t vel_tow = 0;

    uint32_t baseline_tow = 0;
};
