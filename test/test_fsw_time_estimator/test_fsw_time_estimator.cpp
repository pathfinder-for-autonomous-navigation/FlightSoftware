#include "../StateFieldRegistryMock.hpp"



int test_control_task()
{
        UNITY_BEGIN();
        RUN_TEST(test_task_initialization);
        return UNITY_END();
}

#ifdef DESKTOP
int main()
{
        return test_control_task();
}
#else
#include <Arduino.h>
void setup()
{
        delay(2000);
        Serial.begin(9600);
        test_control_task();
}

void loop() {}
#endif

