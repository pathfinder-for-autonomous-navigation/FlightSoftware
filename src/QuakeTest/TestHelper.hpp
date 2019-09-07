
#include "QLocate.hpp"
#include "QuakeMessage.hpp"

#include <vector>
#include <string>

// Test print helper
// bSuccess: true for pass, false for failed
// testName: name of the test
// errorCode: status code
void assert(int bSuccess, const std::string &testName, int errorCode);

//Sbdix response structure
typedef struct sbdix_r
{
  int MO_status;
  int MOMSN;
  int MT_status;
  int MTMSN;
  int MT_length;
  int MT_queued;
} sbdix_r_t;
