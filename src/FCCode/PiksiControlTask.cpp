#include "PiksiControlTask.hpp"

using namespace Devices;

PiksiControlTask::PiksiControlTask(StateFieldRegistry &registry) : ControlTask<void>(registry),
                                                                   piksi("piksi"),
                                                                   pos_sr(0, 100000, 10),
                                                                   pos_f("piksi.pos", pos_sr),
                                                                   vel_sr(0, 100000, 10),
                                                                   vel_f("piksi.vel", vel_sr),
                                                                   baseline_pos_sr(0, 100000, 10),
                                                                   baseline_pos_f("piksi.baseline.pos", baseline_pos_sr),
                                                                   currentState_sr(0, 4, 2),
                                                                   currentState_f("piksi.state", currentState_sr)
{
  add_readable_field(pos_f);
  add_readable_field(vel_f);
  add_readable_field(baseline_pos_f);
  add_readable_field(currentState_f);
}

// void PiksiControlTask::set_downlink_msg(const char *_szMsg, size_t _len)
// {
//   szMsg = _szMsg;
//   len = _len;
// }

int PiksiControlTask::get_current_state() const
{
  return currentState;
}

int PiksiControlTask::get_fix() const
{
  return fix;
}

void PiksiControlTask::execute()
{
  // TODO: allow any state to ignore state and call CONFIG, remember to reset fnSeqNum
  
  //if successfully read data
  int read_out = piksi.read_all();

  if(read_out == 3){
    currentState = NO_DATA;
    return;
  }
  else if(read_out == 2){
    currentState = ERROR;
    return;
  }

  else if(read_out == 1 || read_out == 0){
    piksi.get_gps_time(&time);
    piksi.get_pos_ecef(&baseline_tow, &pos);
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
      currentState = ERROR;
      return;
    }

    int nsats = piksi.get_pos_ecef_nsats();
    if(nsats < 4){
      currentState = ERROR;
      return;
    }

    if(read_out == 1){
      int baseline_flag = piksi.get_baseline_ecef_flags();
      if(baseline_flag == 1)
        currentState = FIXED_RTK;
      else if(baseline_flag == 0)
        currentState = FLOAT_RTK;
      else
        currentState = ERROR;
    }
    else
      currentState = SPP;

  }
  //if read_out is unexpected value which it shouldn't lol
  else
    currentState = ERROR;
  // int res_buffer_read = piksi.read_buffer();
  // if (res_buffer_read == 1){
  //   currentState = BAD_BUFFER;
  // }
  // else if(res_buffer_read == 2){
  //   currentState = MSG_LEN_WRONG;
  // }
  // //res_buffer = 0;
  // else{
  //   get_values();
  //   //int ver_good = verify_good();

  //   bool time_valid = pos_tow == time.tow && vel_tow == time.tow 
  //   && baseline_tow == time.tow && time.tow > tow_past;
    
  //   bool nsats_valid = true;

  //   if(time_valid && nsats_valid){
  //     pos_f.set(pos);
  //     vel_f.set(vel);
  //     baseline_pos_f.set(baseline_pos);
  //     currentState = SUCCESS;
  //   }
  //   else if(time_valid && !nsats_valid){
  //     // TODO SET TIME STATE FIELD
  //     currentState = BAD_NSATS;
  //   }
  //   else
  //   {
  //     // time is not good and nsats not good, so nothing is good, set nothing
  //     currentState = BAD_DATA;
  //   }

    
  // }

  // currentState_f.set(currentState);

  //result = piksi.read_buffer();
  //return res1 + res2;

  // switch (currentState)
  // {
  // case SBDWB:
  //   result = dispatch_sbdwb();
  //   break;
  // case SBDRB:
  //   result = dispatch_sbdrb();
  //   break;
  // case SBDIX:
  //   result = dispatch_sbdix();
  //   break;
  // case CONFIG:
  //   result = dispatch_config();
  //   break;
  // case IS_FUNCTIONAL:
  //   result = dispatch_is_functional();
  //   break;
  // case IDLE:
  //   // TODO: check if statefield registry request has been set
  //   // if so, set currentState and rerun execute
  //   break;
  // default:
  //   // TODO: set error message
  //   currentState = IDLE;
  // }
  // // Reset currentState to idle if fnSeqNum == 0 since that means we executed the last function
  // if (fnSeqNum == 0)
  //   currentState = IDLE;
  // return result;
}

int PiksiControlTask::get_values()
{
  tow_past = time.tow;
  piksi.get_gps_time(&time);

  pos_past = pos_tow;
  vel_past = vel_tow;
  baseline_past = baseline_tow;

  piksi.get_pos_ecef(&pos_tow, &pos);

  //pos_f.set(static_cast<d_vector_t>(pos_tow));
  //pos_f.set(pos);

  piksi.get_vel_ecef(&vel_tow, &vel);
  piksi.get_baseline_ecef(&baseline_tow, &baseline_pos);

  iar = piksi.get_iar();

  //bool ret = verify_time() & verify_baseline() & verify_pos() & verify_vel() & verify_iar();
  return 0;
}

// int PiksiControlTask::dispatch_sbdwb()
// {
//   int errCode = -1;
//   switch (fnSeqNum)
//   {
//   case 0:
//     errCode = quake.query_sbdwb_1(len);
//     break;
//   case 1:
//     errCode = quake.query_sbdwb_2(szMsg, len);
//     break;
//   case 2:
//     errCode = quake.get_sbdwb();
//     break;
//   default:
//     return WRONG_FN_ORDER; // don't know why fnSeqNum would be wrong
//   }
//   if (errCode == OK)
//     fnSeqNum = (fnSeqNum + 1) % 3;
//   return errCode;
// }

// int PiksiControlTask::dispatch_sbdrb()
// {
//   int errCode = -1;
//   switch (fnSeqNum)
//   {
//   case 0:
//     errCode = quake.query_sbdrb_1();
//     break;
//   case 1:
//     errCode = quake.get_sbdrb();
//     break;
//   default:
//     return WRONG_FN_ORDER;
//   }
//   if (errCode == OK)
//     fnSeqNum = (fnSeqNum + 1) % 2;
//   return errCode;
// }

// int PiksiControlTask::dispatch_sbdix()
// {
//   int errCode = -1;
//   switch (fnSeqNum)
//   {
//   case 0:
//     errCode = quake.query_sbdix_1();
//     break;
//   case 1:
//     errCode = quake.get_sbdix();
//     break;
//   default:
//     return WRONG_FN_ORDER;
//   }
//   if (errCode == OK)
//     fnSeqNum = (fnSeqNum + 1) % 2;
//   return errCode;
// }

// int PiksiControlTask::dispatch_config()
// {
//   int errCode = -1;
//   switch (fnSeqNum)
//   {
//   case 0:
//     errCode = quake.query_config_1();
//     break;
//   case 1:
//     errCode = quake.query_config_2();
//     break;
//   case 2:
//     errCode = quake.query_config_3();
//     break;
//   case 3:
//     errCode = quake.get_config();
//     break;
//   default:
//     return WRONG_FN_ORDER;
//   }
//   if (errCode == OK)
//     fnSeqNum = (fnSeqNum + 1) % 4;
//   return errCode;
// }

// int PiksiControlTask::dispatch_is_functional()
// {
//   int errCode = -1;
//   switch (fnSeqNum)
//   {
//   case 0:
//     errCode = quake.get_is_functional();
//     break;
//   case 1:
//     errCode = quake.get_is_functional();
//     break;
//   default:
//     return WRONG_FN_ORDER;
//   }
//   if (errCode == OK)
//     fnSeqNum = (fnSeqNum + 1) % 2;
//   return errCode;
// }