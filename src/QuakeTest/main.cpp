
#include <Arduino.h>
#include "TestHelper.hpp"

// pointer to an iterator for testCases
// std::vector<std::pair<std::string, int>>::iterator pTestIter = testCases.begin();

void setup()
{

  // test device
  Devices::QLocate q("anything", &Serial3, 35, 10);

  // testCases is a vector of pairs <testString, expectedStatus>
  std::vector<std::pair<std::string, int>> testCases{
      std::make_pair("test2", 0), // expect 0

  };

  Serial.begin(9600);
  while (!Serial.available())
    ;
  Serial.println("Entering setup");
  q.setup();
  Serial.println("setup done");
  // Start sbdix session
  q.run_sbdix();
  Serial.println("runsbdix done");
  for (std::pair<std::string, int> test : testCases)
  {
    // Retrieve the error code
    int errorCode = q.sbdwb((test.first).c_str(), (test.first).length());
    assert(errorCode == test.second, test.first, errorCode);
  }
  q.end_sbdix();
  Serial.println("end sbdix done");
  const int *pResp = q.get_sbdix_response();
  Serial.println("get sbdix response done");
  sbdix_r_t *pRes = (sbdix_r_t *)(pResp);
  assert(pResp[0] == 0, "MOStatus = 0", pResp[0]);
  while (1)
    ;
}

void loop()
{
  // sbdix_response
  // uintptr_t sbdix_r[6];

  // Serial.println("%d\n", q.is_functional());
}