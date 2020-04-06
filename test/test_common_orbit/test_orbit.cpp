#include <unity.h>
#define PANGRAVORDER 40
#include <common/Orbit.h>
#include <cstdint>
#include <limits>
#include <lin/core.hpp>
#include <gnc_constants.hpp>
#include <common/GPSTime.hpp>


//UTILITY MACROS
char errormessage[1000];

#define TEST_ASSERT_LIN_3VECT_WITHIN(delta, expected, actual) do {\
            if (!(lin::norm(expected-actual)<=delta)){ \
                TEST_MESSAGE("TEST_ASSERT_LIN_VECT_WITHIN Failed:");\
                sprintf(errormessage, "    expected " #expected " is %.16e, %.16e, %.16e",expected(0),expected(1),expected(2));\
                TEST_MESSAGE(errormessage);\
                sprintf(errormessage, "    actual " #actual " is %.16e, %.16e, %.16e",actual(0),actual(1),actual(2));\
                TEST_MESSAGE(errormessage);\
                sprintf(errormessage, "    expected-actual: %.16e, %.16e, %.16e",expected(0)-actual(0),expected(1)-actual(1),expected(2)-actual(2));\
                TEST_MESSAGE(errormessage);\
                TEST_FAIL();\
            }\
           } while(0) 


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

    //test NAN invalid orbit
    lin::Vector3d r3 {0.0, std::numeric_limits<double>::quiet_NaN(), 0.0};
    lin::Vector3d v3 {941.0211143841228L, 85.66662333729801L, 7552.870253470936L};
    uint64_t t3= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y3(t3,r3,v3);
    TEST_ASSERT_FALSE(y3.valid());
}


void test_applydeltav() {
    //valid orbit
    lin::Vector3d r {-6522019.833240811L, 2067829.846415895L, 776905.9724453629L};
    lin::Vector3d v {941.0211143841228L, 85.66662333729801L, 7552.870253470936L};
    uint64_t t= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y(t,r,v);
    TEST_ASSERT_TRUE(y.valid());
    y.applydeltav({1,2,3});
    TEST_ASSERT_TRUE(y.vecef()(0)==942.0211143841228 );
    TEST_ASSERT_TRUE(y.vecef()(1)==87.66662333729801 );
    TEST_ASSERT_TRUE(y.vecef()(2)==7555.870253470936 );
    //TODO add more valid checks for applying dv that make the orbit invalid.
}

void test_calc_geograv() {
    // numbers from matlab
    constexpr int numtests= 10;
    lin::Vector3d rs[numtests] {
        {1.273445953962255E6L, -5.635810703121731E6L, 3.783748274228486E6L},
        {6.767496925457523E6L, -0.775632314386751E6L, 0.174789034158034E6L},
        {5.511954641818583E6L, 1.730369684612448E6L, 3.700402361779120E6L},
        {-3.429728807059977E6L, -4.393428319886837E6L, -3.898008933058192E6L},
        {1.424398032673680E6L, -0.058834158871470E6L, 6.800537806551877E6L},
        {3.754626327420062E6L, 2.339238703220571E6L, -5.222910396523982E6L},
        {1.116546540571841E6L, -5.441195535522701E6L, 4.060251407964340E6L},
        {5.891369179605223E6L, -0.774724953068517E6L, 3.379524064396650E6L},
        {4.980413524369719E6L, 3.215190762180997E6L, 3.492507069569023E6L},
        {4.340653872138119E6L, 4.827454579177474E6L, -2.333313402895764E6L}
    };
    lin::Vector3d g_trues[numtests] {
        {-1.539696971018925, 6.813869729277867, -4.587360098621410},
        {-8.538310385217216, 0.978581348990112, -0.221100492287455},
        {-6.799385753352515, -2.134609944055416, -4.577509267369736},
        {4.341005311205861, 5.560758133290192, 4.947837863556324},
        {-1.683669710025605, 0.069496883079301, -8.060879268587501},
        {-4.654853552706789, -2.900048127443640, 6.493434342829370},
        {-1.365026870642826, 6.651905135251407, -4.977551205270546},
        {-7.349181112037527, 0.966421474652404, -4.227639428049087},
        {-6.092479080716284, -3.933150994208532, -4.284247589652169},
        {-5.273243501305509, -5.864689616987217, 2.842381806608654}
    };
    lin::Vector3d gs[numtests]= {{NAN,NAN,NAN}};
    double ps[numtests]= {NAN};
    for (int i=0; i< numtests; i++){
        Orbit::calc_geograv(rs[i],gs[i],ps[i]);
        TEST_ASSERT_DOUBLE_WITHIN(1E-6, g_trues[i](0), gs[i](0));
        TEST_ASSERT_DOUBLE_WITHIN(1E-6, g_trues[i](1), gs[i](1));
        TEST_ASSERT_DOUBLE_WITHIN(1E-6, g_trues[i](2), gs[i](2));
        //finite diff check
        lin::Vector3d delta_rs[4]= {
            {10.0,0.0,0.0},
            {0.0,10.0,0.0},
            {0.0,0.0,10.0},
            {-10.0,0.4,-.043}
        };
        for (int j=0; j<4; j++){
            double p_at_delta= {NAN};
            lin::Vector3d junk;
            double predicted_deltap= lin::dot(delta_rs[j],gs[i]);
            Orbit::calc_geograv(rs[i]+delta_rs[j],junk,p_at_delta);
            double real_deltap= p_at_delta- ps[i];
            TEST_ASSERT_DOUBLE_WITHIN(1E-3, real_deltap, predicted_deltap);
        }
    }
}

void test_specificenergy() {
    //valid orbit from grace
    lin::Vector3d r1 {-6522019.833240811L, 2067829.846415895L, 776905.9724453629L};
    lin::Vector3d v1 {941.0211143841228L, 85.66662333729801L, 7552.870253470936L};
    uint64_t t1= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y1(t1,r1,v1);
    TEST_ASSERT_TRUE(y1.valid());
    double e1= y1.specificenergy({0.0,0.0,gnc::constant::earth_rate_ecef_z});

    //623246500 D E -6388456.55330517 2062929.296577276 1525892.564091281 0.0009730537727755604 0.0006308360706885164 0.0006414402851363097 1726.923087560988 -185.5049475128178 7411.544615026139 1.906279023699492e-06 1.419033598283502e-06 2.088561789107221e-06  00000000
    //grace orbit 100 seconds later
    lin::Vector3d r2 {-6388456.55330517L, 2062929.296577276L, 1525892.564091281L};
    lin::Vector3d v2 {1726.923087560988L, -185.5049475128178L, 7411.544615026139L};
    uint64_t t2= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y2(t2,r2,v2);
    TEST_ASSERT_TRUE(y2.valid());
    double e2= y2.specificenergy({0.0,0.0,gnc::constant::earth_rate_ecef_z});

    TEST_ASSERT_DOUBLE_WITHIN(10, e2, e1);

    //Test invalid orbit returns NAN
    Orbit x;
    TEST_ASSERT_FALSE(x.valid());
    TEST_ASSERT_TRUE(isnan(x.specificenergy({0.0,0.0,gnc::constant::earth_rate_ecef_z})));
}

void test_shortupdate() {
    //valid orbit from grace
    lin::Vector3d r1 {-6522019.833240811L, 2067829.846415895L, 776905.9724453629L};
    lin::Vector3d v1 {941.0211143841228L, 85.66662333729801L, 7552.870253470936L};
    uint64_t t1= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y1(t1,r1,v1);
    TEST_ASSERT_TRUE(y1.valid());

    //623246500 D E -6388456.55330517 2062929.296577276 1525892.564091281 0.0009730537727755604 0.0006308360706885164 0.0006414402851363097 1726.923087560988 -185.5049475128178 7411.544615026139 1.906279023699492e-06 1.419033598283502e-06 2.088561789107221e-06  00000000
    //grace orbit 100 seconds later
    lin::Vector3d r2 {-6388456.55330517L, 2062929.296577276L, 1525892.564091281L};
    lin::Vector3d v2 {1726.923087560988L, -185.5049475128178L, 7411.544615026139L};
    uint64_t t2= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y2(t2,r2,v2);
    TEST_ASSERT_TRUE(y2.valid());
    
    double junk;
    for(int i=0; i<1000;i++){
        y1.shortupdate(100'000'000,{0.0,0.0,gnc::constant::earth_rate_ecef_z},junk);
    }
    lin::Vector3d vfinal = y1.vecef();
    //lin::Vector3d rfinal = y1.recef();
    TEST_ASSERT_LIN_3VECT_WITHIN(1E-5, vfinal, v2);

    
}

int test_orbit() {
    UNITY_BEGIN();
    RUN_TEST(test_basic_constructors);
    RUN_TEST(test_applydeltav);
    RUN_TEST(test_calc_geograv);
    RUN_TEST(test_specificenergy);
    RUN_TEST(test_shortupdate);
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
