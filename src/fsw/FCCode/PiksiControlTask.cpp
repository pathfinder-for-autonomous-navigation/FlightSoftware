#include "PiksiControlTask.hpp"

#include <gnc/constants.hpp>

#include <lin/core.hpp>
#include <lin/views.hpp>

PiksiControlTask::PiksiControlTask(StateFieldRegistry &registry, Devices::Piksi &_piksi) 
    : TimedControlTask<void>(registry, "piksi"),
    piksi(_piksi),
    pos_f("piksi.pos", Serializer<lin::Vector3d>(6771000,6921000,28)),
    vel_f("piksi.vel", Serializer<lin::Vector3d>(7570,7685,19)),
    baseline_pos_f("piksi.baseline_pos", Serializer<lin::Vector3d>(0,2000,22)),
    current_state_f("piksi.state", Serializer<unsigned char>(14)),
    fix_error_count_f("piksi.fix_error_count", Serializer<unsigned int>(1001)),
    time_f("piksi.time", Serializer<gps_time_t>()),
    microdelta_f("piksi.microdelta", Serializer<unsigned int>())
    {
        add_readable_field(pos_f);
        add_readable_field(vel_f);
        add_readable_field(baseline_pos_f);
        add_readable_field(current_state_f);
        add_readable_field(fix_error_count_f);
        add_readable_field(time_f);
        add_readable_field(microdelta_f);

        //register callbacks and begin the serial port
        piksi.setup();

        // Set initial values
        constexpr double nan = gnc::constant::nan;
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::no_fix));
        pos_f.set({nan, nan, nan});
        vel_f.set({nan, nan, nan});
        baseline_pos_f.set({nan, nan, nan});
    }

void PiksiControlTask::execute()
{
    int read_out = piksi.read_all();

    unsigned int microdelta = piksi.get_microdelta();
    microdelta_f.set(microdelta);

    //Throw CRC error if microdelta is not in expected range and no other error is thrown
    if (microdelta > PIKSI_MD_THRESHOLD && read_out < 2) read_out = 3; 

    if(read_out == 5){
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::time_limit_error));
        return;
    }

    else if(read_out == 4){
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::no_data_error));
        return;
    }

    else if(read_out == 3){
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::crc_error));
        return;
    }

    else if(read_out == 2){
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::no_fix));
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
            //indicative of getting only part of the next scream

            current_state_f.set(static_cast<unsigned int>(piksi_mode_t::sync_error));
            return;
        }

        int nsats = piksi.get_pos_ecef_nsats();
        if(nsats < 4){
            current_state_f.set(static_cast<unsigned int>(piksi_mode_t::nsat_error));
            return;
        }

        if(read_out == 0) {
            current_state_f.set(static_cast<unsigned int>(piksi_mode_t::spp));
        }
        if(read_out == 1){
            int baseline_flag = piksi.get_baseline_ecef_flags();
            if(baseline_flag == 1){
                current_state_f.set(static_cast<unsigned int>(piksi_mode_t::fixed_rtk));
            }
            else if(baseline_flag == 0){
                current_state_f.set(static_cast<unsigned int>(piksi_mode_t::float_rtk));
            }
            else{
                //baseline flag unexpected value
                current_state_f.set(static_cast<unsigned int>(piksi_mode_t::data_error));
                return;
            }
        }

        //set values in data statefields
        time_f.set(time);
        pos_f.set(lin::view<lin::Vector3d>(pos.data()));
        vel_f.set(lin::view<lin::Vector3d>(vel.data()));
        if(read_out == 1){
            baseline_pos_f.set(lin::view<lin::Vector3d>(baseline_pos.data()));
        }
    }

    //if read_out is unexpected value which it shouldn't do lol
    else{
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::data_error));
        return;
    }
}

