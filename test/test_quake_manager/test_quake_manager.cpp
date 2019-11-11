#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../../src/FCCode/QuakeManager.h"
#include "../../src/FCCode/radio_mode_t.enum"

#include <unity.h>

#define assert_qct(x) {\
  tf.step(); \
  TEST_ASSERT_EQUAL(x, static_cast<unsigned int>(tf.quake_manager->qct.get_current_state()));\
  }

#define assert_radio(x) {\
  tf.step(); \
  TEST_ASSERT_EQUAL(x, tf.radio_mode_fp->get());\
}
unsigned int cycleNumber = 4294967295;
char* snap1 = 
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
          BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\
          CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\
          DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD\
          EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE";
char* snap2 = 
          "1111111111111111111111111111111111111111111111111111111111111111111111\
          2222222222222222222222222222222222222222222222222222222222222222222222\
          3333333333333333333333333333333333333333333333333333333333333333333333\
          4444444444444444444444444444444444444444444444444444444444444444444444\
          555555555555555555555555555555555555555555555555555555555555555555555";
class TestFixture {
  public:
    StateFieldRegistryMock registry;
    // Input state fields to quake manager
    std::shared_ptr<WritableStateField<unsigned int>> cycle_no_fp;

    // Output state fields from quake manager
    std::shared_ptr<WritableStateField<unsigned int>> radio_mode_fp;
    std::shared_ptr<WritableStateField<char*>> radio_mo_packet_fp;
    std::shared_ptr<WritableStateField<char*>> radio_mt_packet_fp;
    std::shared_ptr<WritableStateField<int>> radio_err_fp;
    std::shared_ptr<WritableStateField<unsigned int>> snapshot_size_fp;

    std::unique_ptr<QuakeManager> quake_manager;
    // Not sure if this is how it works
    TestFixture() : registry() {
        cycle_no_fp = registry.create_writable_field<unsigned int>("pan.cycle_no", 0, cycleNumber, 32);
        radio_mode_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("radio.mode"));
        snapshot_size_fp = registry.create_writable_field<unsigned int>("downlink_producer.snap_size");
        radio_mo_packet_fp = registry.create_writable_field<char*>("downlink_producer.mo_ptr");
        radio_mt_packet_fp = registry.create_writable_field<char*>("downlink_producer.mt_ptr");
        radio_err_fp = registry.create_writable_field<int>("downlink_producer.radio_err_ptr");
        quake_manager = std::make_unique<QuakeManager>(registry);

        snapshot_size_fp->set(350);
        radio_mo_packet_fp->set(snap1);
        cycle_no_fp->set(static_cast<unsigned int>(4242)); 
        
    }
  // Make a step in the world
  void step(unsigned int amt = 1) {
    unsigned int cycleNo = cycle_no_fp->get();
    cycle_no_fp->set(cycleNo + amt); 
    quake_manager->execute(); 
  }
};

void test_valid_initialization() {
    TestFixture tf;
    // We should start in config
    assert_radio(radio_mode_t::config);
    tf.step();
    tf.step();
    tf.step();
    assert_radio(radio_mode_t::config);
    tf.step();
    // We should be in sbdwb since we want to immediately load a message
    assert_radio(radio_mode_t::write);
}

void test_dispatch_config()
{
  TestFixture tf;
  // Set the state to config
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::config));
  tf.quake_manager->qct.currentState = CONFIG;
  // Step a bunch of cycles
  tf.step(tf.quake_manager->max_config_cycles);
  // Make sure that we're in wait
  assert_radio(radio_mode_t::wait);
  assert_qct(IDLE);
}

void test_dispatch_wait()
{
  TestFixture tf;
  // Set the state to wait
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::wait));
  tf.quake_manager->qct.currentState = IDLE;
  // Make a step
  tf.step();
  // We should still be in wait
  assert_radio(radio_mode_t::wait);
  // Consume all the wait cycles
  tf.step(tf.quake_manager->max_wait_cycles-2);
  assert_radio(radio_mode_t::wait);
  tf.step();
  // We should be in transceive
  assert_qct(SBDIX);
  assert_radio(radio_mode_t::transceive);
}

void test_dispatch_write()
{
  TestFixture tf;
  // Set the state to write
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::write));
  tf.quake_manager->qct.currentState = SBDWB;
  tf.quake_manager->mo_idx = 0;
  // Should be pointing to the first piece of the first snapshot
  TEST_ASSERT_EQUAL(0, tf.quake_manager->mo_idx);
  tf.step(); // sbdwb 0
  // Should be pointing to the second piece of the first snapshot
  TEST_ASSERT_EQUAL(1, tf.quake_manager->mo_idx);
  // Should have the first piece of snap1 loaded
  TEST_ASSERT_EQUAL(
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 
    tf.quake_manager->qct.szMsg); 
  TEST_ASSERT_EQUAL(70, tf.quake_manager->qct.len); // should be length 70
  tf.step(); // sbdwb 1
  tf.step(); // sbdwb 2
  assert_qct(SBDIX); // should be back to SBDIX now
  TEST_ASSERT_EQUAL(1, tf.quake_manager->mo_idx); // should still be pointing to second piece of snap1
  tf.step(); // sbdix 0
  tf.quake_manager->qct.quake.sbdix_r[0] = 1; // pretend that we have comms
  tf.quake_manager->qct.quake.sbdix_r[4] = 0; // pretend that sbdix has no message
  tf.step(); // sbdix 1
  // Write next snapshot, but we should still be pointing to the snap1
  tf.radio_mo_packet_fp->set(snap2); 
  TEST_ASSERT_EQUAL(snap1, tf.quake_manager->mo_buffer_copy); // should still be snap1
  assert_qct(SBDWB); // we should be back to SBDWB since SBDIX had no message
  assert_radio(radio_mode_t::write);
  TEST_ASSERT_EQUAL(1, tf.quake_manager->mo_idx); // should still be pointing to second piece of snap1
  tf.step(); // sbdwb 0
  TEST_ASSERT_EQUAL(
    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB", 
    tf.quake_manager->qct.szMsg); 
  TEST_ASSERT_EQUAL(2, tf.quake_manager->mo_idx); // should be pointing to third piece of snap1
  tf.step(); // sbdwb 1
  tf.step(); // sbdwb 2
  assert_radio(radio_mode_t::transceive); // back to SBDIX
  tf.step(); // sbdix 0
  tf.step(); // sbdix 1
  assert_qct(SBDWB); // back to SBDWB
  TEST_ASSERT_EQUAL(2, tf.quake_manager->mo_idx); // should be pointing to third piece of snap1
  // fast forward, pretend this is the fifth piece of snap1
  tf.quake_manager->mo_idx = 4;
  TEST_ASSERT_EQUAL(snap1, tf.quake_manager->mo_buffer_copy); // should be snap1
  tf.step(); // sbdwb 1
  TEST_ASSERT_EQUAL(
    "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE", 
    tf.quake_manager->qct.szMsg); 
  tf.step(); // sbdwb 2
  TEST_ASSERT_EQUAL(0, tf.quake_manager->mo_idx); // should be reset to 0
  TEST_ASSERT_EQUAL(snap1, tf.quake_manager->mo_buffer_copy); // should still be snap1
  assert_qct(SBDIX);
  tf.step(); // sbdix 0
  tf.step(); // sbdix 1
  TEST_ASSERT_EQUAL(0, tf.quake_manager->mo_idx); // should still be 0
  TEST_ASSERT_EQUAL(snap2, tf.quake_manager->mo_buffer_copy); // SBDIX should have loaded snap2
  tf.step(); // sbdwb 0
  TEST_ASSERT_EQUAL(
    "1111111111111111111111111111111111111111111111111111111111111111111111", 
    tf.quake_manager->qct.szMsg); 
  TEST_ASSERT_EQUAL(0, tf.quake_manager->mo_idx); // should still be 0 now
  tf.step(); // sbdwb 1
  tf.step(); // sbdwb 2
  assert_radio(radio_mode_t::transceive); // back to SBDIX
  TEST_ASSERT_EQUAL(1, tf.quake_manager->mo_idx); // should be 1 now

}

void test_dispatch_transceive()
{
  TestFixture tf;
  // Set the state to transceive
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::transceive));
  tf.quake_manager->qct.currentState = SBDIX;
  tf.step(); // sbdix 0
  // pretend that we've had no comms for a logn time
  tf.quake_manager->last_checkin_cycle = tf.cycle_no_fp->get() - tf.quake_manager->max_transceive_cycles - 1;
  tf.quake_manager->qct.quake.sbdix_r[0] = 32; // pretend that we have no comms
  tf.step(); // sbdix 1
  tf.step(); // sbdix 0
  assert_radio(radio_mode_t::transceive);
  tf.step(); // sbdix 1
  // still no comms... 
  
  // We should now be in wait 
  assert_radio(radio_mode_t::wait);
  assert_qct(IDLE);

  // Wait a lot of cycles
  tf.step(tf.quake_manager->max_wait_cycles);
  // We should be back in transceive after we've expired wait
  assert_radio(radio_mode_t::transceive);
  assert_qct(SBDIX);

  tf.quake_manager->qct.quake.sbdix_r[0] = 1; // pretend that we have comms
  tf.quake_manager->qct.quake.sbdix_r[4] = 70; // pretend that we have a message 
  tf.step(); // sbdix 0
  tf.step(); // sbdix 1
  assert_radio(radio_mode_t::read); // should be reading
  assert_qct(SBDRB);
}

void test_dispatch_read()
{
  TestFixture tf;
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::read));
  tf.quake_manager->qct.currentState = SBDRB;
  tf.step(); // sbdrb 0
  tf.step(); //sbdrb 1
  assert_radio(radio_mode_t::write); // should transition to SBDWB
  assert_qct(SBDWB);
}

void test_dispatch_manual()
{
  TestFixture tf;
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::manual));
}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_dispatch_config);
    RUN_TEST(test_dispatch_wait);
    RUN_TEST(test_dispatch_write);
    RUN_TEST(test_dispatch_transceive);
    RUN_TEST(test_dispatch_read);
    RUN_TEST(test_dispatch_manual);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_mission_manager();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_mission_manager();
}

void loop() {}
#endif
