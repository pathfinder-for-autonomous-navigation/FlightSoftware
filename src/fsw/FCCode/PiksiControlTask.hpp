#pragma once

#include "Drivers/Piksi.hpp"
#include <string>
#include <common/GPSTime.hpp>

#include "TimedControlTask.hpp"

#include "piksi_mode_t.enum"
#include "Fault.hpp"
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

    /**
     * @brief In the case of a bad read from the piksi, set relevant outputs to NaN.
     * 
     */
    void nan_return();

    /**
     * @brief This is called from MCL to find the radio state after QM has be constructed.
     * 
     */
    void init();

    // StateField for position, velocity, and baseline
    ReadableStateField<d_vector_t> pos_f;
    ReadableStateField<d_vector_t> vel_f;
    ReadableStateField<d_vector_t> baseline_pos_f;

    // Serializer and StateField for currentState and
    // number of cycles since a good reading
    ReadableStateField<unsigned int> current_state_f;
    ReadableStateField<unsigned int> fix_error_count_f;

    //Serializer and StateField for time
    Serializer<gps_time_t> time_sr;
    ReadableStateField<gps_time_t> time_f;

    // System time of last good Piksi reading
    InternalStateField<sys_time_t> last_fix_time_f;

    Serializer<bool> bool_sr;

    // Pointer to the radio state
    const WritableStateField<unsigned char>* radio_state_fp;
    // Ground commandablefield that toggles whether or not 
    // piksi is muted if radio is broadcasting
    WritableStateField<bool> data_mute_f;

    // fault is signaled whenever piksi has an error, including no_data
    Fault piksi_fault;
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
