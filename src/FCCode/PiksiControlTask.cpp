#include "PiksiControlTask.hpp"

using namespace Devices;

PiksiControlTask::PiksiControlTask(StateFieldRegistry &registry) : ControlTask<int>(registry), 
  piksi("piksi"){}

// void PiksiControlTask::set_downlink_msg(const char *_szMsg, size_t _len)
// {
//   szMsg = _szMsg;
//   len = _len;
// }

// int PiksiControlTask::get_current_state() const
// {
//   return currentState;
// }

// size_t PiksiControlTask::get_current_fn_number() const
// {
//   return fnSeqNum;
// }

// bool PiksiControlTask::request_state(int requested_state)
// {
//   if (requested_state == CONFIG)
//     currentState = CONFIG;
//   else if (currentState == IDLE)
//     currentState = requested_state;
//   else
//     return false;
//   fnSeqNum = 0;
//   return true;
// }

int PiksiControlTask::execute()
{
  // TODO: allow any state to ignore state and call CONFIG, remember to reset fnSeqNum
  int result = 1;
  result = exec_read_buffer();
  //result = piksi.read_buffer();
  return result;

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
int PiksiControlTask::exec_read_buffer(){
  //return 0;
  return piksi.read_buffer();
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