#include <unity.h>
#include "../custom_assertions.hpp"
#include <stdio.h>
#include <fsw/FCCode/orb/Orbit.h>
#include <cstdint>
#include <limits>
#include <lin.hpp>
#include <gnc_constants.hpp>
#include <common/GPSTime.hpp>


//UTILITY MACROS

//0 or 1 that can't be calculated at compile time.
#ifdef DESKTOP
#define notcompiletime (rand()%2)
#else
#include <Arduino.h>
#define notcompiletime (millis()%2)
#endif





/**
 * Recursive Lin Tensor determinant function.
 * */
template<typename T, int MR>
T det(const lin::Matrix<T, 0, 0, MR, MR>& x){
    int n= x.rows();
    int m= x.cols();
    if (m!=n){
        assert(0);
    }
    assert(n>0);
    if (n==1){
        return x(0,0);
    }
    if (n==2){
        return x(0,0)*x(1,1)-x(1,0)*x(0,1);
    }
    T result=0;

    for (int i=0; i<n; i++){
        //x without row 0 or col i
        lin::Matrix<T,0,0,MR,MR> submat(n-1,n-1);
        if (i==0){
            submat= lin::ref<0, 0, MR, MR> (x, 1, i+1, n-1, n-1-i);
        }else if (i==n-1){
            submat= lin::ref<0, 0, MR, MR> (x, 1, 0, n-1, i);
        }else {
            lin::ref<0, 0, MR, MR> (submat, 0, 0, n-1, i) = lin::ref<0, 0, MR, MR> (x, 1, 0, n-1, i);
            lin::ref<0, 0, MR, MR> (submat, 0, i, n-1, n-1-i) = lin::ref<0, 0, MR, MR> (x, 1, i+1, n-1, n-1-i);
        }
        result+= (i%2?-1:1)*x(0,i)*det<T,MR>(submat);
    }
    return result;
}

//grace orbit initial
const orb::Orbit gracestart(uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK,{-6522019.833240811L, 2067829.846415895L, 776905.9724453629L},{941.0211143841228L, 85.66662333729801L, 7552.870253470936L});

//grace orbit 100 seconds later
const orb::Orbit grace100s(uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK+100'000'000'000ULL,{-6388456.55330517L, 2062929.296577276L, 1525892.564091281L},{1726.923087560988L, -185.5049475128178L, 7411.544615026139L});

//grace orbit 13700 seconds later
const orb::Orbit grace13700s(uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK+13700'000'000'000ULL,{1579190.147083268L, -6066459.613667888L, 2785708.976728437L},{480.8261476296949L, -3083.977113497177L, -6969.470748202005L});

//earth rate in ecef (rad/s)
lin::Vector3d earth_rate_ecef= {0.000000707063506E-4,-0.000001060595259E-4,0.729211585530000E-4};

void test_basic_constructors() {
    orb::Orbit x;
    TEST_ASSERT_FALSE(x.valid());

    //test valid orbit
    TEST_ASSERT_TRUE(gracestart.valid());
    x=gracestart;
    TEST_ASSERT_TRUE(x.valid());

    //test invalid orbit
    lin::Vector3d r2 {0.0L, 0.0L, 0.0L};
    lin::Vector3d v2 {941.0211143841228L, 85.66662333729801L, 7552.870253470936L};
    uint64_t t2= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    orb::Orbit y2(t2,r2,v2);
    TEST_ASSERT_FALSE(y2.valid());

    //test NAN invalid orbit
    lin::Vector3d r3 {0.0, std::numeric_limits<double>::quiet_NaN(), 0.0};
    lin::Vector3d v3 {941.0211143841228L, 85.66662333729801L, 7552.870253470936L};
    uint64_t t3= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    orb::Orbit y3(t3,r3,v3);
    TEST_ASSERT_FALSE(y3.valid());
}


void test_applydeltav() {
    //valid orbit
    orb::Orbit y= gracestart;
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
    lin::Vector3d gs[numtests]= {lin::nans<lin::Vector3d>()};
    double ps[numtests]= {std::numeric_limits<double>::quiet_NaN()};
    for (int i=0; i< numtests; i++){
        orb::Orbit::calc_geograv(rs[i],gs[i],ps[i]);
        PAN_TEST_ASSERT_LIN_3VECT_WITHIN(1E-6, (g_trues[i]), (gs[i]));
        //finite diff check
        lin::Vector3d delta_rs[4]= {
            {10.0,0.0,0.0},
            {0.0,10.0,0.0},
            {0.0,0.0,10.0},
            {-10.0,0.4,-.043}
        };
        for (int j=0; j<4; j++){
            double p_at_delta= {std::numeric_limits<double>::quiet_NaN()};
            lin::Vector3d junk;
            double predicted_deltap= lin::dot(delta_rs[j],gs[i]);
            orb::Orbit::calc_geograv(rs[i]+delta_rs[j],junk,p_at_delta);
            double real_deltap= p_at_delta- ps[i];
            TEST_ASSERT_FLOAT_WITHIN(1E-3, 0.0, (real_deltap-predicted_deltap));
        }
    }
}

void test_specificenergy() {
    orb::Orbit y1=gracestart;
    TEST_ASSERT_TRUE(y1.valid());
    double e1= y1.specificenergy({0.0,0.0,gnc::constant::earth_rate_ecef_z});
    orb::Orbit y2=grace100s;
    TEST_ASSERT_TRUE(y2.valid());
    double e2= y2.specificenergy({0.0,0.0,gnc::constant::earth_rate_ecef_z});

    TEST_ASSERT_FLOAT_WITHIN(10.0, 0.0,(e2-e1));

    //Test invalid orbit returns NAN
    orb::Orbit x;
    TEST_ASSERT_FALSE(x.valid());
    TEST_ASSERT_FLOAT_IS_NAN(x.specificenergy({0.0,0.0,gnc::constant::earth_rate_ecef_z}));
}

/**
 * 0.1s and 0.2s time step 100s update test vs grace data
 */
void test_shortupdate_a(){
    double junk;
    //time steps to test
    int timesteps[2] = {100'000'000,200'000'000};
    for(int j=0; j<2; j++){
        orb::Orbit y=gracestart;
        for(int i=0; i<(100'000'000'000LL/timesteps[j]);i++){
            y.shortupdate(timesteps[j],earth_rate_ecef,junk);
        }
        TEST_ASSERT_TRUE(y.valid());
        TEST_ASSERT_TRUE(y.nsgpstime()==gracestart.nsgpstime()+100'000'000'000LL);
        PAN_TEST_ASSERT_LIN_3VECT_WITHIN(2E-4, y.vecef(), grace100s.vecef());
        PAN_TEST_ASSERT_LIN_3VECT_WITHIN(1E-2, y.recef(), grace100s.recef());
    }
}

/**
 * 0.2s time step 13700s update test vs grace data
 * This also does the reverse update with -0.2s time step to check reverability
 */
void test_shortupdate_b(){
    orb::Orbit y= gracestart;
    //force not compile time
    y.applydeltav({0.0,(notcompiletime*1E-10),0.0});
    TEST_ASSERT_TRUE(y.valid());
    double junk;
    for(int i=0; i<(13700*5);i++){
        y.shortupdate(200'000'000,earth_rate_ecef,junk);
    }
    TEST_ASSERT_TRUE(y.valid());
    PAN_TEST_ASSERT_LIN_3VECT_WITHIN(1E-1, y.vecef(), grace13700s.vecef());
    PAN_TEST_ASSERT_LIN_3VECT_WITHIN(20.0, y.recef(), grace13700s.recef());

    // test reversibility of propagator
    for(int i=0; i<(13700*5);i++){
        y.shortupdate(-200'000'000,earth_rate_ecef,junk);
    }
    TEST_ASSERT_TRUE(y.valid());
    TEST_ASSERT_TRUE(y.nsgpstime()==gracestart.nsgpstime());
    PAN_TEST_ASSERT_LIN_3VECT_WITHIN(1.0E-5, y.vecef(), gracestart.vecef());
    PAN_TEST_ASSERT_LIN_3VECT_WITHIN(1.0E-2, y.recef(), gracestart.recef());
}

/**
 * Test jacobians with finite difference and test jacobian has determinant 1
 */
void test_shortupdate_c(){
    lin::internal::RandomsGenerator const rand(0);
    int timesteps[4] = {100'000'000,200'000'000,-100'000'000,-200'000'000};
    for(int j=0; j<4; j++){
        orb::Orbit y= gracestart;
        double junk;
        lin::Matrix<double, 6, 6> jac;
        y.shortupdate(timesteps[j],earth_rate_ecef,junk,jac);
        TEST_ASSERT_TRUE(y.valid());
        lin::Matrix<double,0,0,6,6> jacref= lin::ref<0, 0, 6, 6> (jac, 0, 0, 6, 6);
        double detjac=det<double,6>(jacref);
        //test jacobian has determinant 1
        TEST_ASSERT_FLOAT_WITHIN(1.0E-15,(1.0- detjac),0);
        //TEST_MESSAGE("Jacobian");
        //printtensor(jac);
        for(int i=0; i<10;i++){
            lin::Vectord<6> initialdelta= lin::rands<lin::Vectord<6>>(6, 1, rand);
            initialdelta= initialdelta-lin::consts<lin::Vectord<6>>(0.5,6,1);
            initialdelta= initialdelta*0.2;
            //initialdelta is now a uniform random vector +-0.1
            lin::Vector3d deltar= lin::ref<3, 1>(initialdelta, 0, 0);
            lin::Vector3d deltav= lin::ref<3, 1>(initialdelta, 3, 0);
            orb::Orbit y_diff(gracestart.nsgpstime(),gracestart.recef()+deltar,gracestart.vecef()+deltav);
            TEST_ASSERT_TRUE(y_diff.valid());
            y_diff.shortupdate(timesteps[j],earth_rate_ecef,junk);
            TEST_ASSERT_TRUE(y_diff.valid());
            lin::Vector3d finaldr= y_diff.recef()-y.recef();
            lin::Vector3d finaldv= y_diff.vecef()-y.vecef();
            lin::Vectord<6> finaldelta;
            lin::ref<3, 1>(finaldelta, 0, 0)= finaldr;
            lin::ref<3, 1>(finaldelta, 3, 0)= finaldv;
            lin::Vectord<6> expecteddelta= jac*initialdelta;
            //TEST_MESSAGE("Initial delta");
            //printtensor(initialdelta);
            //TEST_MESSAGE("Final delta");
            //printtensor(finaldelta);
            TEST_ASSERT_FLOAT_WITHIN(5E-9, 0.0, lin::norm(expecteddelta-finaldelta));
        }
    }
}

/**
 * test zero time step doesn't effect orbit.
 */
void test_shortupdate_d() {
    orb::Orbit y= gracestart;
    double estart= gracestart.specificenergy(earth_rate_ecef);
    //do the update with zero time step
    double e;
    y.shortupdate(0,earth_rate_ecef,e);
    //test memory equal
    TEST_ASSERT_EQUAL_MEMORY (&e, &estart, sizeof(double));
    TEST_ASSERT_EQUAL_MEMORY (&y, &gracestart, sizeof(orb::Orbit));
}

/**
 * test jacobian output doesn't have any side effects.
 */
void test_shortupdate_e() {
    orb::Orbit y= gracestart;
    lin::Matrix<double, 6, 6> jac;
    //do the update with jacobian output
    double espjacup;
    y.shortupdate(200'000'000,earth_rate_ecef,espjacup,jac);
    orb::Orbit yjacup= y;
    //do the update without jacobian output
    y= gracestart;
    double espnojacup;
    y.shortupdate(200'000'000,earth_rate_ecef,espnojacup);
    orb::Orbit ynojacup= y;
    //test memory equal
    TEST_ASSERT_EQUAL_MEMORY (&espjacup, &espnojacup, sizeof(double));
    TEST_ASSERT_EQUAL_MEMORY (&yjacup, &ynojacup, sizeof(orb::Orbit));
}

/**
 * Test specificenergy output in shortupdate
 */
void test_shortupdate_f() {
    orb::Orbit y= gracestart;
    double energy_start= y.specificenergy(earth_rate_ecef);
    double energy_mid;
    y.shortupdate(200'000'000,earth_rate_ecef,energy_mid);
    double energy_end= y.specificenergy(earth_rate_ecef);
    TEST_ASSERT_FLOAT_WITHIN(1E-2,0.0,(energy_start-energy_mid));
    TEST_ASSERT_FLOAT_WITHIN(1E-2,0.0,(energy_end-energy_mid));
    //backward update
    energy_start= y.specificenergy(earth_rate_ecef);
    y.shortupdate(-200'000'000,earth_rate_ecef,energy_mid);
    energy_end= y.specificenergy(earth_rate_ecef);
    TEST_ASSERT_FLOAT_WITHIN(1E-2,0.0,(energy_start-energy_mid));
    TEST_ASSERT_FLOAT_WITHIN(1E-2,0.0,(energy_end-energy_mid));
}

/**
 * Test invalid orbit update gives NAN outputs
 */
void test_shortupdate_g() {
    //test invalid orbit update gives NAN outputs
    orb::Orbit invalidorbit;
    TEST_ASSERT_FALSE(invalidorbit.valid());
    double invalidenergy= 0.0;
    invalidorbit.shortupdate(200'000'000,earth_rate_ecef,invalidenergy);
    TEST_ASSERT_FALSE(invalidorbit.valid());
    TEST_ASSERT_FLOAT_IS_NAN(invalidenergy);
    invalidenergy= 0.0;
    lin::Matrix<double, 6, 6> jac= lin::zeros<lin::Matrix<double, 6, 6>>();
    invalidorbit.shortupdate(200'000'000,earth_rate_ecef,invalidenergy,jac);
    TEST_ASSERT_FALSE(invalidorbit.valid());
    TEST_ASSERT_FLOAT_IS_NAN(invalidenergy);
    for (int i=0; i<6; i++){
        for (int j=0; j<6; j++){
	        TEST_ASSERT_FLOAT_IS_NAN(jac(i,j));
	    }
    }
}

int test_orbit() {
    UNITY_BEGIN();
    RUN_TEST(test_basic_constructors);
    RUN_TEST(test_applydeltav);
    RUN_TEST(test_calc_geograv);
    RUN_TEST(test_specificenergy);
    RUN_TEST(test_shortupdate_a);
    RUN_TEST(test_shortupdate_b);
    RUN_TEST(test_shortupdate_c);
    RUN_TEST(test_shortupdate_d);
    RUN_TEST(test_shortupdate_e);
    RUN_TEST(test_shortupdate_f);
    RUN_TEST(test_shortupdate_g);
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
