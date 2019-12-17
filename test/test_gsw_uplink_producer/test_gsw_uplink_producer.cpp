#include "UplinkProducerMock.h"
#include <unity.h>
#include <fstream>

// Flow data for test
struct TestFixture {
    // Flight Software and Ground Software control classes
    StateFieldRegistryMock reg;
    std::unique_ptr<UplinkProducerMock> producer;

};

void test_task_initialization() {
    TestFixture tf;
    char mydata[4];
    UplinkPacket(mydata, 4);
}

void test_task_execute() {
  TestFixture tf;
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}
