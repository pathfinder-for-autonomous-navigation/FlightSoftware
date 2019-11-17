#pragma once

#include <ControlTask.hpp>
#include <Piksi.hpp>
#include <string>
#include <GPSTime.hpp>

#include "piksi_mode_t.enum"

class PiksiControlTask : public ControlTask<void>
{
public:
#ifndef DESKTOP
    PiksiControlTask(StateFieldRegistry &registry) : ControlTask<int>(registry),
                                                    piksi("Piksi", Serial4) {}

#else
    PiksiControlTask(StateFieldRegistry &registry);

#endif
    Devices::Piksi piksi;
    /** 
    * execute is overriden from ControlTask 
    * Calling execute() when the state is IDLE generates no effects. 
    */
    void execute();

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
    Serializer<int> currentState_sr;
    ReadableStateField<int> currentState_f;

    //Serializer and StateField for time
    Serializer<gps_time_t> time_sr;
    ReadableStateField<gps_time_t> time_f;

protected:
    //Internal Data Containers
    std::array<double, 3> pos;
    std::array<double, 3> vel;
    std::array<double, 3> baseline_pos;

    msg_gps_time_t msg_time;
    gps_time_t time;
    gps_time_t time_old;

    unsigned int tow_past = 0;
    unsigned int iar;

    unsigned int pos_tow;

    unsigned int vel_tow;

    unsigned int baseline_tow;

private:

    piksi_mode_t currentState; // the state of the Piksi

    // the number of cycles since a successful / meaningful read operation
    unsigned int since_good_cycles = 0;

};