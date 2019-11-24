#include "QuakeControlTask.h"

using namespace Devices;

void QuakeControlTask::set_downlink_msg(const char *_szMsg, size_t _len)
{
  MO_msg_p = _szMsg;
  MO_msg_len = _len;
}

int QuakeControlTask::get_current_state() const
{
  return currentState;
}

size_t QuakeControlTask::get_current_fn_number() const
{
  return fnSeqNum;
}

bool QuakeControlTask::request_state(int requested_state)
{
  if (requested_state == CONFIG || requested_state == IDLE)
    currentState = requested_state;
  else if (currentState == IDLE)
    currentState = requested_state;
  else
    return false;
  fnSeqNum = 0;
  return true;
}

int QuakeControlTask::execute()
{
  // TODO: allow any state to ignore state and call CONFIG, remember to reset fnSeqNum
  int result = false;
  switch (currentState)
  {
  case SBDWB:
    result = dispatch_sbdwb();
    break;
  case SBDRB:
    result = dispatch_sbdrb();
    break;
  case SBDIX:
    result = dispatch_sbdix();
    break;
  case CONFIG:
    result = dispatch_config();
    break;
  case IS_FUNCTIONAL:
    result = dispatch_is_functional();
    break;
  case IDLE:
    break;
  default:
    fnSeqNum = 0;
  }
  // Reset currentState to idle if fnSeqNum == 0 since that means we executed the last function
  if (fnSeqNum == 0)
    currentState = IDLE;
  return result;
}

int QuakeControlTask::dispatch_sbdwb()
{
  int errCode = -1;
  switch (fnSeqNum)
  {
  case 0:
    errCode = quake.query_sbdwb_1(MO_msg_len);
    break;
  case 1:
    errCode = quake.query_sbdwb_2(MO_msg_p, MO_msg_len);
    break;
  case 2:
    errCode = quake.get_sbdwb();
    break;
  default:
    return WRONG_FN_ORDER; // don't know why fnSeqNum would be wrong
  }
  if (errCode == OK)
    fnSeqNum = (fnSeqNum + 1) % 3;
  return errCode;
}

int QuakeControlTask::dispatch_sbdrb()
{
  int errCode = -1;
  switch (fnSeqNum)
  {
  case 0:
    errCode = quake.query_sbdrb_1();
    break;
  case 1:
    errCode = quake.get_sbdrb();
    break;
  default:
    return WRONG_FN_ORDER;
  }
  if (errCode == OK)
    fnSeqNum = (fnSeqNum + 1) % 2;
  return errCode;
}

int QuakeControlTask::dispatch_sbdix()
{
  int errCode = -1;
  switch (fnSeqNum)
  {
  case 0:
    errCode = quake.query_sbdix_1();
    break;
  case 1:
    errCode = quake.get_sbdix();
    break;
  default:
    return WRONG_FN_ORDER;
  }
  if (errCode == OK)
    fnSeqNum = (fnSeqNum + 1) % 2;
  return errCode;
}

int QuakeControlTask::dispatch_config()
{
  int errCode = -1;
  switch (fnSeqNum)
  {
  case 0:
    errCode = quake.query_config_1();
    break;
  case 1:
    errCode = quake.query_config_2();
    break;
  case 2:
    errCode = quake.query_config_3();
    break;
  case 3:
    errCode = quake.get_config();
    break;
  default:
    return WRONG_FN_ORDER;
  }
  if (errCode == OK)
    fnSeqNum = (fnSeqNum + 1) % 4;
  return errCode;
}

int QuakeControlTask::dispatch_is_functional()
{
  int errCode = -1;
  switch (fnSeqNum)
  {
  case 0:
    errCode = quake.query_is_functional_1();
    break;
  case 1:
    errCode = quake.get_is_functional();
    break;
  default:
    return WRONG_FN_ORDER;
  }
  if (errCode == OK)
    fnSeqNum = (fnSeqNum + 1) % 2;
  return errCode;
}