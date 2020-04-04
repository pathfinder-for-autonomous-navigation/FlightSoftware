#include <unity.h>
#include <common/Orbit.h>
#include <cstdint>
#include <limits>
#include <lin/core.hpp>
#include <gnc_constants.hpp>
#include <common/GPSTime.hpp>

// grace data
// time,xpos,ypos,zpos,xposerr,yposerr,zposerr,xvel,yvel,zvel,xvelerr,yvelerr,zvelerr
// 623246400 D E -6522019.833240811 2067829.846415895 776905.9724453629 0.0009578283838282736 0.0006236271904723294 0.0006257082914522712 941.0211143841228 85.66662333729801 7552.870253470936 1.921071421308773e-06 1.457922754459223e-06 2.147748285029321e-06

void test_basic_constructors() {
    Orbit x;
    TEST_ASSERT_FALSE(x.valid());

    //test valid orbit
    lin::Vector3d r {-6522019.833240811L, 2067829.846415895L, 776905.9724453629L};
    lin::Vector3d v {941.0211143841228L, 85.66662333729801L, 7552.870253470936L};
    uint64_t t= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y(t,r,v);
    TEST_ASSERT_TRUE(y.valid());
    x=y;
    TEST_ASSERT_TRUE(x.valid());

    //test invalid orbit
    lin::Vector3d r2 {0.0L, 0.0L, 0.0L};
    lin::Vector3d v2 {941.0211143841228L, 85.66662333729801L, 7552.870253470936L};
    uint64_t t2= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y2(t2,r2,v2);
    TEST_ASSERT_FALSE(y2.valid());
}

int test_orbit() {
    UNITY_BEGIN();
    RUN_TEST(test_basic_constructors);
    return UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char *argv[]) {
    return test_orbit();
}
#else
#include <Arduino.h>
void setup() {
    delay(10000);
    Serial.begin(9600);
    test_orbit();
}

void loop() {}
#endif
