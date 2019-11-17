#include "PiksiControlTask.hpp"
#include "piksi_mode_t.enum"

using namespace Devices;

PiksiControlTask::PiksiControlTask(StateFieldRegistry &registry, unsigned int offset, Devices::Piksi &_piksi) : TimedControlTask<void>(registry, offset),
                                                                   piksi(_piksi),
                                                                   pos_sr(0, 100000, 100),
                                                                   pos_f("piksi.pos", pos_sr),
                                                                   vel_sr(0, 100000, 100),
                                                                   vel_f("piksi.vel", vel_sr),
                                                                   baseline_pos_sr(0, 100000, 100),
                                                                   baseline_pos_f("piksi.baseline_pos", baseline_pos_sr),
                                                                   currentState_sr(0, 4, 2),
                                                                   currentState_f("piksi.state", currentState_sr),
                                                                   time_sr(),
                                                                   time_f("piksi.time", time_sr)
{
    add_readable_field(pos_f);
    add_readable_field(vel_f);
    add_readable_field(baseline_pos_f);
    add_readable_field(currentState_f);
    add_readable_field(time_f);

    currentState_f.set(static_cast<int>(piksi_mode_t::NO_FIX));
}

void PiksiControlTask::execute()
{  
    int read_out = piksi.read_all();

    //4 means no bytes
    //3 means CRC error on serial
    //5 means timing error exceed
    if(read_out == 3|| read_out == 4|| read_out == 5)
        since_good_cycles += 1;
    else 
        since_good_cycles = 0;
        
    //if we haven't had a good reading in ~120 seconds the piksi is probably dead
    //eventually replace with HAVT logic
    if(since_good_cycles > 1000){
        currentState_f.set(static_cast<int>(piksi_mode_t::DEAD));
        //prevent roll over
        since_good_cycles = 1001;
        return;
    }

    if(read_out == 5){
        currentState_f.set(static_cast<int>(piksi_mode_t::TIME_LIMIT_ERROR));
        return;
    }

    else if(read_out == 4){
        currentState_f.set(static_cast<int>(piksi_mode_t::NO_DATA_ERROR));
        return;
    }

    else if(read_out == 3){
        currentState_f.set(static_cast<int>(piksi_mode_t::CRC_ERROR));
        return;
    }

    else if(read_out == 2){
        currentState_f.set(static_cast<int>(piksi_mode_t::NO_FIX));
        return;
    }

    else if(read_out == 1 || read_out == 0){
        //get time from driver, then dump into a gps_time_t
        piksi.get_gps_time(&msg_time);
        time = gps_time_t(msg_time);

        piksi.get_pos_ecef(&pos_tow, &pos);
        piksi.get_vel_ecef(&vel_tow, &vel);

        if(read_out == 1)
            piksi.get_baseline_ecef(&baseline_tow, &baseline_pos);

        bool check_time;
        if(read_out == 1)
            check_time = time.tow == pos_tow && time.tow == vel_tow && time.tow == baseline_tow;
        else
            check_time = time.tow == pos_tow && time.tow == vel_tow;

        if(!check_time){
            //error caused by times not matching up
            //indicitave of getting only part of the next scream

            currentState_f.set(static_cast<int>(piksi_mode_t::SYNC_ERROR));
            return;
        }

        int nsats = piksi.get_pos_ecef_nsats();
        if(nsats < 4){
            currentState_f.set(static_cast<int>(piksi_mode_t::NSAT_ERROR));
            return;
        }

        if(read_out == 0) {
            currentState_f.set(static_cast<int>(piksi_mode_t::SPP));
        }
        if(read_out == 1){
            int baseline_flag = piksi.get_baseline_ecef_flags();
            if(baseline_flag == 1){
                currentState_f.set(static_cast<int>(piksi_mode_t::FIXED_RTK));
            }
            else if(baseline_flag == 0){
                currentState_f.set(static_cast<int>(piksi_mode_t::FLOAT_RTK));
            }
            else{
                //baseline flag unexpected value
                currentState_f.set(static_cast<int>(piksi_mode_t::DATA_ERROR));
                return;
            }
        }

        //set values in data statefields
        time_f.set(time);
        pos_f.set(pos);
        vel_f.set(vel);
        if(read_out == 1){
            baseline_pos_f.set(baseline_pos);
        }

    }

    //if read_out is unexpected value which it shouldn't do lol
    else{
        currentState_f.set(static_cast<int>(piksi_mode_t::DATA_ERROR));
        return;
    }

}
