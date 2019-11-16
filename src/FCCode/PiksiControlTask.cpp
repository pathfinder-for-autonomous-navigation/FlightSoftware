#include "PiksiControlTask.hpp"

using namespace Devices;

PiksiControlTask::PiksiControlTask(StateFieldRegistry &registry) : ControlTask<void>(registry),
                                                                   piksi("piksi"),
                                                                   pos_sr(0, 100000, 100),
                                                                   pos_f("piksi.pos", pos_sr),
                                                                   vel_sr(0, 100000, 100),
                                                                   vel_f("piksi.vel", vel_sr),
                                                                   baseline_pos_sr(0, 100000, 100),
                                                                   baseline_pos_f("piksi.baseline.pos", baseline_pos_sr),
                                                                   currentState_sr(0, 4, 2),
                                                                   currentState_f("piksi.state", currentState_sr)
{
  add_readable_field(pos_f);
  add_readable_field(vel_f);
  add_readable_field(baseline_pos_f);
  add_readable_field(currentState_f);
}

int PiksiControlTask::get_current_state() const
{
  return currentState;
}

void PiksiControlTask::execute()
{  
  int read_out = piksi.read_all();

  if(read_out == 4 || read_out == 2 || read_out == 5)
    since_good_cycles += 1;
  else 
    since_good_cycles = 0;
  
  //if we haven't had a heartbeat or good reading in ~120 seconds we probably dead
  if(since_good_cycles > 1000){
    currentState = DEAD;
    currentState_f.set(DEAD);
    //prevent rollover lmao
    since_good_cycles = 1001;
    return;
  }

  if(read_out == 5){
    currentState = TIME_LIMIT;
    currentState_f.set(TIME_LIMIT);
    return;
  }

  if(read_out == 4){
    currentState = NO_DATA;
    currentState_f.set(NO_DATA);
    return;
  }
  else if(read_out == 2 || read_out == 3){
    currentState = NO_FIX;
    currentState_f.set(NO_FIX);
    return;

  }

  else if(read_out == 1 || read_out == 0){
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
      //indicitave of packet being split
      currentState = SYNC_ERROR;
      currentState_f.set(SYNC_ERROR);
      return;
    }

    int nsats = piksi.get_pos_ecef_nsats();
    if(nsats < 4){
      currentState = NSAT_ERROR;
      currentState_f.set(NSAT_ERROR);
      return;
    }

    if(read_out == 0) {
      currentState = SPP;
      currentState_f.set(SPP);
    }
    if(read_out == 1){
      int baseline_flag = piksi.get_baseline_ecef_flags();
      if(baseline_flag == 1){
        currentState = FIXED_RTK;
        currentState_f.set(FIXED_RTK);
      }
      else if(baseline_flag == 0){
        currentState = FLOAT_RTK;
        currentState_f.set(FLOAT_RTK);
      }
      else{
        currentState = DATA_ERROR;
        currentState_f.set(DATA_ERROR);
        return;
      }
    }

    //set values in statefields

    //LOL ASK TANISHQ ABOUT WHAT KINDA TIME HE WANTS
    //time_f.set(time);
    pos_f.set(pos);
    vel_f.set(vel);
    if(read_out == 1){
      baseline_pos_f.set(baseline_pos);
    }

  }
  //if read_out is unexpected value which it shouldn't lol
  else
    currentState = DATA_ERROR;

}
