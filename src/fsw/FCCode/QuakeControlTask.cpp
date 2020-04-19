#include "QuakeControlTask.h"

using namespace Devices;

void QuakeControlTask::set_downlink_msg(char *_szMsg, size_t _len)
{
  MO_msg_p = _szMsg;
  MO_msg_len = _len;
}

size_t QuakeControlTask::get_fn_num() const
{
  return fnSeqNum;
}

int QuakeControlTask::execute(radio_state_t state)
{
  // TODO: allow any state to ignore state and call CONFIG, remember to reset fnSeqNum
  int result = false;
  switch (state)
  {
  case radio_state_t::write:
    result = dispatch_sbdwb();
    break;
  case radio_state_t::read:
    result = dispatch_sbdrb();
    break;
  case radio_state_t::transceive:
    result = dispatch_sbdix();
    break;
  case radio_state_t::config:
    result = dispatch_config();
    break;
  default:
    fnSeqNum = 0;
  }
  return result;
}

int QuakeControlTask::dispatch_sbdwb()
{
  int errCode;
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
  int errCode;
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
  int errCode;
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
  int errCode;
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
