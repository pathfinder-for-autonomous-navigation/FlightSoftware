#include "../StateFieldRegistryMock.hpp"
#include "../custom_assertions.hpp"

#include <fsw/FCCode/Drivers/Piksi.hpp>
#include <fsw/FCCode/PiksiControlTask.hpp>
#include <fsw/FCCode/piksi_mode_t.enum>

#include <lin/core.hpp>

#define assert_piksi_mode(x) {\
    TEST_ASSERT_EQUAL(x, static_cast<piksi_mode_t>(tf.currentState_fp->get()));\
}

class TestFixture {
    public:
        StateFieldRegistryMock registry;

        // pointers to statefields for easy access
        ReadableStateField<unsigned char>* currentState_fp;
        ReadableStateField<lin::Vector3d>* pos_fp;
        ReadableStateField<lin::Vector3d>* vel_fp;
        ReadableStateField<lin::Vector3d>* baseline_fp;
        ReadableStateField<gps_time_t>* time_fp;
        ReadableStateField<unsigned int>* fix_error_count_fp;

        std::unique_ptr<PiksiControlTask> piksi_task;

        Devices::Piksi piksi;
        
        #ifdef DESKTOP
                #define PIKSI_INITIALIZATION piksi("piksi")
        #else
                #define PIKSI_INITIALIZATION piksi("piksi", Serial4)
        #endif

        // Create a TestFixture instance of PiksiController with pointers to statefields
        TestFixture() : registry(), PIKSI_INITIALIZATION {
                piksi_task = std::make_unique<PiksiControlTask>(registry, 0, piksi);  

                // initialize pointers to statefields      
                currentState_fp = registry.find_readable_field_t<unsigned char>("piksi.state");
                pos_fp = registry.find_readable_field_t<lin::Vector3d>("piksi.pos");
                vel_fp = registry.find_readable_field_t<lin::Vector3d>("piksi.vel");
                baseline_fp = registry.find_readable_field_t<lin::Vector3d>("piksi.baseline_pos");
                time_fp = registry.find_readable_field_t<gps_time_t>("piksi.time");
                fix_error_count_fp = registry.find_readable_field_t<unsigned int>("piksi.fix_error_count");
        }

        #undef PIKSI_INITIALIZATION

        //method to make calling execute faster
        void execute(){
                piksi_task->execute();
        }

        //set of mocking methods
        void set_read_return(unsigned int read_out){
                piksi_task->piksi.set_read_return(read_out);
        }
        void set_gps_time(const unsigned int tow){
                piksi_task->piksi.set_gps_time(tow);
        }
        void set_pos_ecef(const unsigned int tow, const lin::Vector3d& position, const unsigned char nsats){
                const std::array<double, 3> arr = {position(0), position(1), position(2)};
                piksi_task->piksi.set_pos_ecef(tow, arr, nsats);
        }
        void set_vel_ecef(const unsigned int tow, const lin::Vector3d& velocity){
                const std::array<double, 3> arr = {velocity(0), velocity(1), velocity(2)};
                piksi_task->piksi.set_vel_ecef(tow, arr);
        }
        void set_baseline_ecef(const unsigned int tow, const lin::Vector3d& position){
                const std::array<double, 3> arr = {position(0), position(1), position(2)};
                piksi_task->piksi.set_baseline_ecef(tow, arr);
        }
        void set_baseline_flag(const unsigned char flag){
                piksi_task->piksi.set_baseline_flag(flag);
        }
        void set_microdelta(unsigned long udelta){
                piksi_task->piksi.set_microdelta(udelta);
        }
};

void test_task_initialization()
{
        TestFixture tf;
        assert_piksi_mode(piksi_mode_t::no_fix);
}

void test_read_errors(){
        TestFixture tf;

        //read out == 3 means a CRC error occurred
        tf.set_read_return(3);
        tf.execute();
        assert_piksi_mode(piksi_mode_t::crc_error);

        //read out == 4 means no bytes were in the buffer
        tf.set_read_return(4);
        tf.execute();
        assert_piksi_mode(piksi_mode_t::no_data_error);

        //read out == 5 means we were processing bytes for more than 900 microseconds
        tf.set_read_return(5);
        tf.execute();
        assert_piksi_mode(piksi_mode_t::time_limit_error);

        //not cataloged read_return value
        tf.set_read_return(7);
        tf.execute();
        assert_piksi_mode(piksi_mode_t::data_error);
    
}

//normal errors that come up, but don't mean that the piksi has failed
void test_normal_errors(){
        TestFixture tf;

        lin::Vector3d pos = {1000.0, 2000.0, 3000.0};
        lin::Vector3d vel = {4000.0, 5000.0, 6000.0};
        lin::Vector3d baseline = {7000.0, 8000.0, 9000.0};

        //tests to make sure to error out if packets not synced to same tow
        unsigned int tow = 100;
        unsigned int bad_tow = 200;
        tf.set_read_return(1);
        tf.set_gps_time(bad_tow);
        tf.set_pos_ecef(tow, pos, 4);
        tf.set_vel_ecef(tow, vel);
        tf.set_baseline_ecef(tow, baseline);
        tf.set_baseline_flag(1);
        tf.set_microdelta(0);
        tf.execute();
        //should error out because of time inconsistency
        assert_piksi_mode(piksi_mode_t::sync_error);

        //insufficient nsats
        tow = 200;
        tf.set_read_return(1);
        tf.set_gps_time(tow);
        tf.set_pos_ecef(tow, pos, 3);
        tf.set_vel_ecef(tow, vel);
        tf.set_baseline_ecef(tow, baseline);
        tf.set_baseline_flag(1);
        tf.set_microdelta(0);
        tf.execute();
        //times agree, but insufficient nsat
        assert_piksi_mode(piksi_mode_t::nsat_error);

        //unexpected baseline flag, should only be 0 or 1
        tow = 200;
        tf.set_read_return(1);
        tf.set_gps_time(tow);
        tf.set_pos_ecef(tow, pos, 4);
        tf.set_vel_ecef(tow, vel);
        tf.set_baseline_ecef(tow, baseline);
        tf.set_baseline_flag(2);
        tf.set_microdelta(0);
        tf.execute();
        //times agree, but insufficient nsat
        assert_piksi_mode(piksi_mode_t::data_error);
}

//test executions that should yield some sort of fix/lock
void test_task_execute()
{
        TestFixture tf;

        lin::Vector3d pos = {1000.0, 2000.0, 3000.0};
        lin::Vector3d vel = {4000.0, 5000.0, 6000.0};
        lin::Vector3d baseline = {7000.0, 8000.0, 9000.0};

        tf.set_read_return(2);
        tf.execute();
        assert_piksi_mode(piksi_mode_t::no_fix);

        //fixed RTK
        unsigned int tow = 200;
        tf.set_read_return(1);
        tf.set_gps_time(tow);
        tf.set_pos_ecef(tow, pos, 4);
        tf.set_vel_ecef(tow, vel);
        tf.set_baseline_ecef(tow, baseline);
        tf.set_baseline_flag(1);
        tf.set_microdelta(0);
        tf.execute();
        //times should now agree, and be in baseline
        assert_piksi_mode(piksi_mode_t::fixed_rtk);
        TEST_ASSERT_TRUE(gps_time_t(0,200,0) == tf.time_fp->get());
        TEST_ASSERT_FLOAT_WITHIN(0.1,lin::fro(pos),lin::fro(tf.pos_fp->get()));
        TEST_ASSERT_FLOAT_WITHIN(0.1,lin::fro(vel),lin::fro(tf.vel_fp->get()));
        TEST_ASSERT_FLOAT_WITHIN(0.1,lin::fro(baseline),lin::fro(tf.baseline_fp->get()));

        //float RTK
        tow = 300;
        pos = {1200.0, 2200.0, 3200.0};
        vel = {4200.0, 5200.0, 6200.0};
        baseline = {7200.0, 8200.0, 9200.0};

        tf.set_read_return(1);
        tf.set_gps_time(tow);
        tf.set_pos_ecef(tow, pos, 4);
        tf.set_vel_ecef(tow, vel);
        tf.set_baseline_ecef(tow, baseline);
        tf.set_baseline_flag(0);
        tf.set_microdelta(0);
        tf.execute();
        //float rtk test
        assert_piksi_mode(piksi_mode_t::float_rtk);
        TEST_ASSERT_TRUE(gps_time_t(0,300,0) == tf.time_fp->get());
        TEST_ASSERT_FLOAT_WITHIN(0.1,lin::fro(pos),lin::fro(tf.pos_fp->get()));
        TEST_ASSERT_FLOAT_WITHIN(0.1,lin::fro(vel),lin::fro(tf.vel_fp->get()));
        TEST_ASSERT_FLOAT_WITHIN(0.1,lin::fro(baseline),lin::fro(tf.baseline_fp->get()));

        //SPP check
        tow = 500;
        pos = {1230.0, 2230.0, 3230.0};
        vel = {4230.0, 5230.0, 6230.0};

        tf.set_read_return(0);
        tf.set_gps_time(tow);
        tf.set_pos_ecef(tow, pos, 4);
        tf.set_vel_ecef(tow, vel);
        tf.execute();
        //check in SPP
        assert_piksi_mode(piksi_mode_t::spp);
        TEST_ASSERT_TRUE(gps_time_t(0,500,0) == tf.time_fp->get());
        TEST_ASSERT_FLOAT_WITHIN(0.1,lin::fro(pos),lin::fro(tf.pos_fp->get()));
        TEST_ASSERT_FLOAT_WITHIN(0.1,lin::fro(vel),lin::fro(tf.vel_fp->get()));
}

int test_control_task()
{
        UNITY_BEGIN();
        RUN_TEST(test_task_initialization);
        RUN_TEST(test_read_errors);
        RUN_TEST(test_normal_errors);
        RUN_TEST(test_task_execute);
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