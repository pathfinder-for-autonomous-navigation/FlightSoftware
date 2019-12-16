#pragma once

#include <Piksi.hpp>
#include <string>
#include <GPSTime.hpp>

#include "TimedControlTask.hpp"

#include "piksi_mode_t.enum"

class PiksiControlTask : public TimedControlTask<void>
{
public:
    static constexpr unsigned int DEAD_CYCLE_COUNT = 1000;

    PiksiControlTask(StateFieldRegistry &registry, unsigned int offset, Devices::Piksi &_piksi);
    
    Devices::Piksi& piksi;
    /** 
    * execute is overriden from TimedControlTask 
    */
    void execute() override;

    //Serializer and StateField for position
    Serializer<d_vector_t> pos_sr;
    ReadableStateField<d_vector_t> pos_f;

    //Serializer and StateField for velocity
    Serializer<d_vector_t> vel_sr;
    ReadableStateField<d_vector_t> vel_f;

    //Serializer and StateField for baseline
    Serializer<d_vector_t> baseline_pos_sr;
    ReadableStateField<d_vector_t> baseline_pos_f;

    //Serializer and StateField for currentState
    Serializer<unsigned int> current_state_sr;
    ReadableStateField<unsigned int> current_state_f;

    //Serializer and StateField for time
    Serializer<gps_time_t> time_sr;
    ReadableStateField<gps_time_t> time_f;

    // Micros # since last good Piksi reading, and
    // the current propagated time from that last good
    // reading
    sys_time_t last_good_reading_time;
    ReadableStateField<unsigned int> us_since_last_reading_f;
    InternalStateField<gps_time_t> propagated_time_f;

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

private:
    // the number of cycles since a successful / meaningful read operation
    unsigned int since_good_cycles = 0;

};