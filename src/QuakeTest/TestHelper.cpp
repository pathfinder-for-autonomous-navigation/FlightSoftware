#include "TestHelper.hpp"
#include <Arduino.h>

void assert(int bSuccess, const std::string &testName, int errorCode)
{
  if (bSuccess)
  {
    Serial.printf("Test %s passed\n", testName.c_str());
  }
  else
  {
    Serial.printf("Test %s failed with error code [%d]\n", testName.c_str(), errorCode);
  }
}