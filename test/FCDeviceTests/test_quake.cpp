#include <HardwareSerial.h>
#include <QLocate/QLocate.hpp>
#include <unity.h>
#include <unity_test/unity_fixture.h>

using namespace Devices;
QLocate quake(&Serial3, 20);
bool quake_setup;

TEST_GROUP(QuakeTests);

TEST_SETUP(QuakeTests) {
    if (!quake_setup) {
        Serial4.begin(9600);
        quake.setup();
        quake_setup = true;
    }
    if(!quake.is_functional()) TEST_IGNORE_MESSAGE("Quake is not connected/not functional");
}

TEST_TEAR_DOWN(QuakeTests) {}

TEST_GROUP_RUNNER(QuakeTests) {
    
}