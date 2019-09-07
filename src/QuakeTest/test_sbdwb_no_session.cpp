
#include "TestHelper.hpp"

// test device
static Devices::QLocate q("anything", &Serial3, 35, 10);
// testCases is a vector of pairs <testString, expectedStatus>
std::vector<std::pair<std::string, int>> testCases{
    std::make_pair("noResponseTest", -1), // expect error code -1
    std::make_pair("test2", -1),          // expect 0
};

// pointer to an iterator for testCases
// std::vector<std::pair<std::string, int>>::iterator pTestIter = testCases.begin();

void setup()
{
  q.setup();
  for (std::pair<std::string, int> test : testCases)
  {
    // Retrieve the error code
    const std::string &st = test.first;
    int len = st.length();
    int errorCode = q.sbdwb((test.first).c_str(), len);
    assert(errorCode == test.second, test.first, errorCode);
  }
  while (1)
    ;
}

void loop()
{
  // sbdix_response
  // uintptr_t sbdix_r[6];

  // Serial.printf("%d\n", q.is_functional());
}