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
                                                                   currentState_sr(0, 2, 2),
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
  if(piksi.read_buffer() == 0){
    fix = piksi.get_pos_ecef_flags();
    //if any kind of RTk
    if(fix == 1 || fix == 2){
      //read baseline data
      piksi.get_baseline_ecef(&baseline_pos);
    }
    else if(fix == 0){
      piksi.get_pos_ecef(&pos);
      piksi.get_gps_time(&time);
      piksi.get_vel_ecef(&vel);
    }
    else
      fix = 3;

  }

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