//#include <StateFieldRegistryMock.hpp>
#include "../StateFieldRegistryMock.hpp"

#include "../../src/FCCode/PiksiControlTask.hpp"
#include <unity.h>
#include <Piksi.hpp>

#include <chrono>
#include <ctime>

#include "../../src/FCCode/piksi_mode_t.enum"
//#include <piksi_mode_t.enum>

#define assert_piksi_mode(x) {\
  TEST_ASSERT_EQUAL(x, static_cast<piksi_mode_t>(tf.currentState_fp->get()));\
}

using namespace Devices;

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    // Input state fields to quake manager
    // std::shared_ptr<ReadableStateField<unsigned int>> cycle_no_fp;

    // std::shared_ptr<InternalStateField<char*>> radio_mo_packet_fp;
    // std::shared_ptr<InternalStateField<char*>> radio_mt_packet_fp;
    // std::shared_ptr<InternalStateField<int>> radio_err_fp;
    // std::shared_ptr<InternalStateField<unsigned int>> snapshot_size_fp;
    // Quake has no output state fields since it is created after downlink producer

    ReadableStateField<int>* currentState_fp;

    std::unique_ptr<PiksiControlTask> piksi_task;

    // Create a TestFixture instance of QuakeManager with the following parameters
    TestFixture() : registry() {
        // Create external field dependencies
        // cycle_no_fp = registry.create_readable_field<unsigned int>("pan.cycle_no");
        // snapshot_size_fp = registry.create_internal_field<unsigned int>("downlink_producer.snap_size");
        // radio_mo_packet_fp = registry.create_internal_field<char*>("downlink_producer.mo_ptr");
        // radio_mt_packet_fp = registry.create_internal_field<char*>("downlink_producer.mt_ptr");
        // radio_err_fp = registry.create_internal_field<int>("downlink_producer.radio_err_ptr");

        // Initialize external fields
        // snapshot_size_fp->set(static_cast<int>(350));
        // radio_mo_packet_fp->set(snap1);
        // cycle_no_fp->set(static_cast<unsigned int>(initCycles));

        // Create Quake Manager instance
        piksi_task = std::make_unique<PiksiControlTask>(registry);
        //piksi_task->piksi.set_read_return(read_out);
        
        currentState_fp = registry.find_readable_field_t<int>("piksi.state");
        
    }
    void set_read_return(unsigned int read_out){
        piksi_task->piksi.set_read_return(read_out);
    }
    void execute(){
        piksi_task->execute();
    }

};

void test_task_initialization()
{
    TestFixture tf;
    // StateFieldRegistry registry;
    // PiksiControlTask task(registry);
}
// void test_execute(){
//     TestFixture tf;
//     tf.set_read_return(2);
//     tf.execute();
//     TEST_ASSERT_EQUAL(piksi_mode_t::NO_FIX, static_cast<piksi_mode_t>(tf.currentState_fp->get()));
// }
void test_read_errors(){
    TestFixture tf;
    //returns a two, no relevant packets coming over line
    //task.piksi.set_read_return(2);
    // tf.set_read_return(2);
    // tf.execute();
    // TEST_ASSERT_EQUAL(piksi_mode_t::NO_FIX, static_cast<piksi_mode_t>(tf.currentState_fp->get()));
    // //assert_piksi_mode(piksi_mode_t::NO_FIX);
    
    //crc error return
    tf.set_read_return(3);
    tf.execute();
    //TEST_ASSERT_EQUAL(piksi_mode_t::CRC_ERROR, task.get_current_state());
    assert_piksi_mode(piksi_mode_t::CRC_ERROR);

    tf.set_read_return(4);
    tf.execute();
    assert_piksi_mode(piksi_mode_t::NO_DATA_ERROR);

    //assert we hit a time limit in read and early termination
    tf.set_read_return(5);
    tf.execute();
    assert_piksi_mode(piksi_mode_t::TIME_LIMIT_ERROR);

    //not cataloged read_return value
    tf.set_read_return(6);
    tf.execute();
    assert_piksi_mode(piksi_mode_t::DATA_ERROR);
    
}
void test_task_execute()
{
    // StateFieldRegistry registry;
    // PiksiControlTask task(registry);

    // //TEST_ASSERT_EQUAL()
    // std::array<double, 3> pos = {1000.0, 2000.0, 3000.0};
    // std::array<double, 3> vel = {4000.0, 5000.0, 6000.0};
    // std::array<double, 3> baseline = {7000.0, 8000.0, 9000.0};

    // //tests to make sure to error out if packets not synced to same tow
    // unsigned int tow = 100;
    // unsigned int bad_tow = 200;
    // task.piksi.set_read_return(1);
    // task.piksi.set_gps_time(bad_tow);
    // task.piksi.set_pos_ecef(tow, pos, 4);
    // task.piksi.set_vel_ecef(tow, vel);
    // task.piksi.set_baseline_ecef(tow, baseline);
    // task.piksi.set_baseline_flag(1);
    // task.execute();
    // //should error out because of time inconsistency
    // TEST_ASSERT_EQUAL(SYNC_ERROR, task.get_current_state());

    // //insufficient nsats
    // tow = 200;
    // task.piksi.set_read_return(1);
    // task.piksi.set_gps_time(tow);
    // task.piksi.set_pos_ecef(tow, pos, 3);
    // task.piksi.set_vel_ecef(tow, vel);
    // task.piksi.set_baseline_ecef(tow, baseline);
    // task.piksi.set_baseline_flag(1);
    // task.execute();
    // //times agree, but insufficient nsat
    // TEST_ASSERT_EQUAL(NSAT_ERROR, task.get_current_state());
    
    // //fixed RTK
    // tow = 200;
    // task.piksi.set_read_return(1);
    // task.piksi.set_gps_time(tow);
    // task.piksi.set_pos_ecef(tow, pos, 4);
    // task.piksi.set_vel_ecef(tow, vel);
    // task.piksi.set_baseline_ecef(tow, baseline);
    // task.piksi.set_baseline_flag(1);
    // task.execute();
    // //times should now agree, and be in baseline
    // TEST_ASSERT_EQUAL(FIXED_RTK, task.get_current_state());

    // //float RTK
    // tow = 200;
    // task.piksi.set_read_return(1);
    // task.piksi.set_gps_time(tow);
    // task.piksi.set_pos_ecef(tow, pos, 4);
    // task.piksi.set_vel_ecef(tow, vel);
    // task.piksi.set_baseline_ecef(tow, baseline);
    // task.piksi.set_baseline_flag(0);
    // task.execute();
    // //float rtk test
    // TEST_ASSERT_EQUAL(FLOAT_RTK, task.get_current_state());

    // //SPP check
    // tow = 200;
    // task.piksi.set_read_return(0);
    // task.piksi.set_gps_time(tow);
    // task.piksi.set_pos_ecef(tow, pos, 4);
    // task.piksi.set_vel_ecef(tow, vel);
    // task.execute();
    // //float rtk test
    // TEST_ASSERT_EQUAL(SPP, task.get_current_state());


    
}

int test_control_task()
{
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_read_errors);
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