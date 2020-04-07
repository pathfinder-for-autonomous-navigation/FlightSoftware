#include <unity.h>
#include <stdio.h>
#include <common/Orbit.h>
#include <cstdint>
#include <limits>
#include <lin.hpp>
#include <gnc_constants.hpp>
#include <common/GPSTime.hpp>


//UTILITY MACROS
/**
 * Lin Tensor Printing function
 * */
template<typename T>
void printtensor(T x){
    int n= x.rows();
    int m= x.cols();
    for (int i=0; i<n; i++){
        char linemessage[256];
        char *point= linemessage;
        for (int j=0; j<m; j++){
	            point+= sprintf(point,"%.10e, ",x(i,j));
	    }
        TEST_MESSAGE(linemessage);
    }
}

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

#define TEST_ASSERT_LIN_3VECT_WITHIN(delta, expected, actual) do {\
            if (!(lin::norm(expected-actual)<=delta)){ \
                char errormessage[256];\
                TEST_MESSAGE("TEST_ASSERT_LIN_VECT_WITHIN Failed:");\
                sprintf(errormessage, "    expected " #expected " is %.10e, %.10e, %.10e",expected(0),expected(1),expected(2));\
                TEST_MESSAGE(errormessage);\
                sprintf(errormessage, "    actual " #actual " is %.10e, %.10e, %.10e",actual(0),actual(1),actual(2));\
                TEST_MESSAGE(errormessage);\
                sprintf(errormessage, "    expected-actual: %.10e, %.10e, %.10e",expected(0)-actual(0),expected(1)-actual(1),expected(2)-actual(2));\
                TEST_MESSAGE(errormessage);\
                sprintf(errormessage, "    norm(expected-actual): %.10e",lin::norm(expected-actual));\
                TEST_MESSAGE(errormessage);\
                sprintf(errormessage, "    max error: %.10e",delta);\
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
        TEST_ASSERT_LIN_3VECT_WITHIN(1E-6, (g_trues[i]), (gs[i]));
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
            TEST_ASSERT_FLOAT_WITHIN(1E-3, 0.0, (real_deltap-predicted_deltap));
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

    TEST_ASSERT_FLOAT_WITHIN(10.0, 0.0,(e2-e1));

    //Test invalid orbit returns NAN
    Orbit x;
    TEST_ASSERT_FALSE(x.valid());
    TEST_ASSERT_FLOAT_IS_NAN(x.specificenergy({0.0,0.0,gnc::constant::earth_rate_ecef_z}));
}

void test_shortupdate() {
    lin::Vector3d earth_rate_ecef= {0.000000707063506E-4,-0.000001060595259E-4,0.729211585530000E-4};
    //valid orbit from grace
    lin::Vector3d r1 {-6522019.833240811L, 2067829.846415895L, 776905.9724453629L};
    lin::Vector3d v1 {941.0211143841228L, 85.66662333729801L, 7552.870253470936L};
    uint64_t t1= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y1(t1,r1,v1);
    Orbit ystart= y1;
    TEST_ASSERT_TRUE(y1.valid());

    //623246500 D E -6388456.55330517 2062929.296577276 1525892.564091281 0.0009730537727755604 0.0006308360706885164 0.0006414402851363097 1726.923087560988 -185.5049475128178 7411.544615026139 1.906279023699492e-06 1.419033598283502e-06 2.088561789107221e-06  00000000
    //grace orbit 100 seconds later
    lin::Vector3d r2 {-6388456.55330517L, 2062929.296577276L, 1525892.564091281L};
    lin::Vector3d v2 {1726.923087560988L, -185.5049475128178L, 7411.544615026139L};
    uint64_t t2= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y2(t2,r2,v2);
    TEST_ASSERT_TRUE(y2.valid());
    
    // 0.1 dt 100 sec test vs grace data
    double junk;
    for(int i=0; i<1000;i++){
        y1.shortupdate(100'000'000,earth_rate_ecef,junk);
    }
    lin::Vector3d vfinal = y1.vecef();
    lin::Vector3d rfinal = y1.recef();
    //lin::Vector3d rfinal = y1.recef();
    TEST_ASSERT_TRUE(y1.nsgpstime()==t1+100'000'000'000LL);
    TEST_ASSERT_LIN_3VECT_WITHIN(2E-4, vfinal, v2);
    TEST_ASSERT_LIN_3VECT_WITHIN(1E-2, rfinal, r2);
    
    // 0.2 dt 100 sec test vs grace data
    y1= ystart;
    for(int i=0; i<500;i++){
        y1.shortupdate(200'000'000,earth_rate_ecef,junk);
    }
    vfinal = y1.vecef();
    rfinal = y1.recef();
    TEST_ASSERT_TRUE(y1.valid());
    TEST_ASSERT_TRUE(y1.nsgpstime()==t1+100'000'000'000LL);
    TEST_ASSERT_LIN_3VECT_WITHIN(2E-4, vfinal, v2);
    TEST_ASSERT_LIN_3VECT_WITHIN(1E-2, rfinal, r2);

    //623260100 D E 1579190.147083268 -6066459.613667888 2785708.976728437 0.0005172311946209459 0.0008808523576941407 0.0006434386278678982 480.8261476296949 -3083.977113497177 -6969.470748202005 1.390617103867044e-06 1.919262957467571e-06 1.936534489542342e-06  00000000
    //grace orbit 623260100-623246400 = 13700 seconds later.
    lin::Vector3d r3 {1579190.147083268L, -6066459.613667888L, 2785708.976728437L};
    lin::Vector3d v3 {480.8261476296949L, -3083.977113497177L, -6969.470748202005L};
    uint64_t t3= uint64_t(gnc::constant::init_gps_week_number)*NANOSECONDS_IN_WEEK;
    Orbit y3(t3,r3,v3);
    TEST_ASSERT_TRUE(y3.valid());
    // 0.2 dt 13700 sec test vs grace data
    y1= ystart;
    for(int i=0; i<(13700*5);i++){
        y1.shortupdate(200'000'000,earth_rate_ecef,junk);
    }
    vfinal = y1.vecef();
    rfinal = y1.recef();
    TEST_ASSERT_TRUE(y1.valid());
    TEST_ASSERT_TRUE(y1.nsgpstime()==t1+13700'000'000'000LL);
    TEST_ASSERT_LIN_3VECT_WITHIN(1E-1, vfinal, v3);
    TEST_ASSERT_LIN_3VECT_WITHIN(20.0, rfinal, r3);

    // test reversibility of propagator
    for(int i=0; i<(13700*5);i++){
        y1.shortupdate(-200'000'000,earth_rate_ecef,junk);
    }
    vfinal = y1.vecef();
    rfinal = y1.recef();
    TEST_ASSERT_TRUE(y1.valid());
    TEST_ASSERT_TRUE(y1.nsgpstime()==t1);
    TEST_ASSERT_LIN_3VECT_WITHIN(1.0E-5, vfinal, v1);
    TEST_ASSERT_LIN_3VECT_WITHIN(1.0E-2, rfinal, r1);

    // test jacobians with finite difference
    lin::internal::RandomsGenerator const rand(0);
    y1= ystart;
    lin::Matrix<double, 6, 6> jac;
    y1.shortupdate(200'000'000,earth_rate_ecef,junk,jac);
    lin::Matrix<double,0,0,6,6> jacref= lin::ref<0, 0, 6, 6> (jac, 0, 0, 6, 6);
    double detjac=det<double,6>(jacref);
    //test jacobian has determinant 1
    TEST_ASSERT_FLOAT_WITHIN(1.0E-15,(1.0- detjac),0);
    TEST_ASSERT_TRUE(y1.valid());
    //TEST_MESSAGE("Jacobian");
    //printtensor(jac);
    for(int i=0; i<5;i++){
        lin::Vectord<6> initialdelta= lin::rands<lin::Vectord<6>>(6, 1, rand);
        initialdelta= initialdelta-lin::consts<lin::Vectord<6>>(0.5,6,1);
        initialdelta= initialdelta*0.2;
        lin::Vector3d deltar= lin::ref<3, 1>(initialdelta, 0, 0);
        lin::Vector3d deltav= lin::ref<3, 1>(initialdelta, 3, 0);
        Orbit y_diff(ystart.nsgpstime(),ystart.recef()+deltar,ystart.vecef()+deltav);
        y_diff.shortupdate(200'000'000,earth_rate_ecef,junk);
        lin::Vector3d finaldr= y_diff.recef()-y1.recef();
        lin::Vector3d finaldv= y_diff.vecef()-y1.vecef();
        lin::Vectord<6> finaldelta;
        lin::ref<3, 1>(finaldelta, 0, 0)= finaldr;
        lin::ref<3, 1>(finaldelta, 3, 0)= finaldv;
        lin::Vectord<6> expecteddelta= jac*initialdelta;
        //TEST_MESSAGE("Initial delta");
        //printtensor(initialdelta);
        //TEST_MESSAGE("Final delta");
        //printtensor(finaldelta);
        TEST_ASSERT_FLOAT_WITHIN(1E-9, 0.0, lin::norm(expecteddelta-finaldelta));
    }

    y1= ystart;
    y1.shortupdate(-200'000'000,earth_rate_ecef,junk,jac);
    TEST_ASSERT_TRUE(y1.valid());
    jacref= lin::ref<0, 0, 6, 6> (jac, 0, 0, 6, 6);
    detjac=det<double,6>(jacref);
    //test jacobian has determinant 1
    TEST_ASSERT_FLOAT_WITHIN(1.0E-15,(1.0- detjac),0);
    //TEST_MESSAGE("Jacobian");
    //printtensor(jac);
    for(int i=0; i<5;i++){
        lin::Vectord<6> initialdelta= lin::rands<lin::Vectord<6>>(6, 1, rand);
        initialdelta= initialdelta-lin::consts<lin::Vectord<6>>(0.5,6,1);
        initialdelta= initialdelta*0.2;
        lin::Vector3d deltar= lin::ref<3, 1>(initialdelta, 0, 0);
        lin::Vector3d deltav= lin::ref<3, 1>(initialdelta, 3, 0);
        Orbit y_diff(ystart.nsgpstime(),ystart.recef()+deltar,ystart.vecef()+deltav);
        y_diff.shortupdate(-200'000'000,earth_rate_ecef,junk);
        lin::Vector3d finaldr= y_diff.recef()-y1.recef();
        lin::Vector3d finaldv= y_diff.vecef()-y1.vecef();
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

    //test jacobian update doesn't change update
    y1= ystart;
    double espjacup;
    y1.shortupdate(200'000'000,earth_rate_ecef,espjacup,jac);
    Orbit yjacup= y1;
    y1= ystart;
    double espnojacup;
    y1.shortupdate(200'000'000,earth_rate_ecef,espnojacup);
    Orbit ynojacup= y1;
    TEST_ASSERT_EQUAL_MEMORY (&espjacup, &espnojacup, sizeof(double));
    TEST_ASSERT_EQUAL_MEMORY (&yjacup, &ynojacup, sizeof(Orbit));

    //Test specificenergy forward update
    y1= ystart;
    double energy_start= y1.specificenergy(earth_rate_ecef);
    double energy_mid;
    y1.shortupdate(200'000'000,earth_rate_ecef,energy_mid);
    double energy_end= y1.specificenergy(earth_rate_ecef);
    TEST_ASSERT_FLOAT_WITHIN(1E-2,0.0,(energy_start-energy_mid));
    TEST_ASSERT_FLOAT_WITHIN(1E-2,0.0,(energy_end-energy_mid));
    //backward update
    energy_start= y1.specificenergy(earth_rate_ecef);
    y1.shortupdate(-200'000'000,earth_rate_ecef,energy_mid);
    energy_end= y1.specificenergy(earth_rate_ecef);
    TEST_ASSERT_FLOAT_WITHIN(1E-2,0.0,(energy_start-energy_mid));
    TEST_ASSERT_FLOAT_WITHIN(1E-2,0.0,(energy_end-energy_mid));

    //test invalid orbit update gives NAN outputs
    Orbit invalidorbit;
    TEST_ASSERT_FALSE(invalidorbit.valid());
    double invalidenergy=0.0;
    invalidorbit.shortupdate(200'000'000,earth_rate_ecef,invalidenergy);
    TEST_ASSERT_FALSE(invalidorbit.valid());
    TEST_ASSERT_FLOAT_IS_NAN(invalidenergy);
    invalidorbit.shortupdate(200'000'000,earth_rate_ecef,invalidenergy,jac);
    TEST_ASSERT_FALSE(invalidorbit.valid());
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
