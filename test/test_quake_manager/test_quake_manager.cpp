#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../../src/FCCode/QuakeManager.h"
#include "../../src/FCCode/radio_mode_t.enum"

#include <unity.h>

// Check that state x matches the current state of the QuakeControlTask
#define assert_qct(x) {\
  TEST_ASSERT_EQUAL(x, static_cast<unsigned int>(tf.quake_manager->qct.get_current_state()));\
  }

// Check that radio state x matches the current radio state
#define assert_radio(x) {\
  TEST_ASSERT_EQUAL(x, tf.radio_mode_fp->get());\
}

// Check that x matches the current fn number
#define assert_fn_num(x) {\
  TEST_ASSERT_EQUAL(x, tf.quake_manager->qct.get_current_fn_number());\
}

// ---------------------------------------------------------------------------
// Test Setup
// ---------------------------------------------------------------------------

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
    std::shared_ptr<InternalStateField<char*>> radio_mo_packet_fp;
    std::shared_ptr<InternalStateField<char*>> radio_mt_packet_fp;
    std::shared_ptr<WritableStateField<int>> radio_err_fp;
    std::shared_ptr<WritableStateField<unsigned int>> snapshot_size_fp;

    std::unique_ptr<QuakeManager> quake_manager;
    // Create a TestFixture instance of QuakeManager with the following parameters
    TestFixture(unsigned int radio_mode, int qct_state) : registry() {
        // Create external field dependencies
        cycle_no_fp = registry.create_writable_field<unsigned int>("pan.cycle_no", 0, cycleNumber, 32);
        snapshot_size_fp = registry.create_writable_field<unsigned int>("downlink_producer.snap_size");
        radio_mo_packet_fp = registry.create_writable_field<char*>("downlink_producer.mo_ptr");
        radio_mt_packet_fp = registry.create_writable_field<char*>("downlink_producer.mt_ptr");
        radio_err_fp = registry.create_writable_field<int>("downlink_producer.radio_err_ptr");

        // Create Quake Manager instance
        quake_manager = std::make_unique<QuakeManager>(registry);
        radio_mode_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("radio.mode"));
        
        // Initialize external fields
        snapshot_size_fp->set(350);
        radio_mo_packet_fp->set(snap1);
        cycle_no_fp->set(static_cast<unsigned int>(4242)); 

        // Initialize internal fields
        if (qct_state != -1)
        {
          quake_manager->qct.currentState = qct_state;
          radio_mode_fp->set(radio_mode); 
        }
    }
  // Make a step in the world
  void step(unsigned int amt = 1) {
    unsigned int cycleNo = cycle_no_fp->get();
    cycle_no_fp->set(cycleNo + amt); 
    quake_manager->execute(); 
  }
};
// ---------------------------------------------------------------------------
// Unexpected error transitions
// ---------------------------------------------------------------------------
void test_wait_unexpected() 
{
  // setup
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::wait), IDLE);
  // If in WAIT and unexpected Flag is set
  tf.quake_manager->unexpectedFlag = true;
  // then we should transition to Config
  tf.step();
  assert_qct(CONFIG);
  assert_radio(radio_mode_t::config);
}

void test_config_unexpected() 
{
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::config), CONFIG);
  // If in CONFIG and unexpected Flag is set 
  tf.quake_manager->unexpectedFlag = true;
  // then we should clear the flag and start executing config
  tf.step();
  TEST_ASSERT_FALSE(tf.quake_manager->unexpectedFlag);
  assert_qct(CONFIG);
  assert_radio(radio_mode_t::config);
  assert_fn_num(1);
}
// Helper test Function
void test_rwt_unexpected(int qct_state, unsigned int radio_mode)
{
  TestFixture tf(radio_mode, qct_state);
  // If in SBDRB, SBDWB, or SBDIX and unexpected Flag is set
  tf.quake_manager->unexpectedFlag = true;
  // then we should transition to Wait
  tf.step();
  assert_qct(IDLE);
  assert_radio(radio_mode_t::wait);
  assert_fn_num(0);
}
void test_read_unexpected() 
{
  test_rwt_unexpected(SBDRB, static_cast<unsigned int>(radio_mode_t::read));
}
void test_write_unexpected() 
{
  test_rwt_unexpected(SBDWB, static_cast<unsigned int>(radio_mode_t::write));
}
void test_trans_unexpected() 
{
  test_rwt_unexpected(SBDIX, static_cast<unsigned int>(radio_mode_t::transceive));
}

// ---------------------------------------------------------------------------
// No more cycles transitions
// ---------------------------------------------------------------------------
void test_wait_no_more_cycles()
{
  // If we are in WAIT and there are no more cycles
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::wait), IDLE);
  tf.step(tf.quake_manager->max_wait_cycles);
  // Then we should execute transcieve on the next cycle
  assert_qct(SBDIX);
  assert_fn_num(0);
  assert_radio(static_cast<unsigned int>(radio_mode_t::read));
}

// Helper test Function
void test_rwc_unexpected(int qct_state, unsigned int radio_mode)
{
  // If in CONFIG, WRITE, READ, TRANS and we run out of cycles
  TestFixture tf(radio_mode, qct_state);
  tf.quake_manager->unexpectedFlag = false;
  tf.cycle_no_fp->set(tf.cycle_no_fp->get() + tf.quake_manager->max_config_cycles);
  tf.step();
  // Then we should transition to WAIT and set the error flag
  assert_qct(IDLE);
  assert_radio(radio_mode_t::wait);
  assert_fn_num(0);
  TEST_ASSERT_TRUE(tf.quake_manager->unexpectedFlag);
}

void test_config_no_more_cycles()
{
  test_rwc_unexpected(CONFIG, static_cast<unsigned int>(radio_mode_t::config));
}
void test_read_no_more_cycles()
{
  test_rwc_unexpected(SBDRB, static_cast<unsigned int>(radio_mode_t::read));
}
void test_write_no_more_cycles()
{
  test_rwc_unexpected(SBDWB, static_cast<unsigned int>(radio_mode_t::write));
}
void test_trans_no_more_cycles()
{
  test_rwc_unexpected(SBDIX, static_cast<unsigned int>(radio_mode_t::transceive));
}

// ---------------------------------------------------------------------------
// On success transitions
// ---------------------------------------------------------------------------
void test_config_ok()
{
  // If we are in CONFIG and we complete config
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::config), CONFIG);
  // Step a bunch of cycles
  tf.step(); // 0
  tf.step(); // 1
  tf.step(); // 2
  tf.step(); // 3
  // Then we should execute WRITE on the next cycle
  assert_radio(radio_mode_t::config);
  assert_qct(IDLE); // transition occurs on IDLE
  tf.step();
  assert_radio(radio_mode_t::write);
  assert_qct(SBDWB);
  assert_fn_num(0);
}

void test_read_ok()
{
  // If we are in READ and we complete read
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::read), SBDRB);
  tf.step(); // 0
  tf.step(); // 1
  // Then we should execute WRITE on the next cycle
  assert_radio(radio_mode_t::read);
  assert_qct(IDLE); // transition occurs on IDLE
  tf.step();
  assert_radio(radio_mode_t::write);
  assert_qct(SBDWB);
  assert_fn_num(0);
}

void test_write_ok()
{
  // If we are in WRITE and we complete write
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::write), SBDWB);
  tf.step(); // 0
  tf.step(); // 1
  tf.step(); // 2
  // Then we should execute TRANS on the next cycle
  assert_radio(radio_mode_t::write);
  assert_qct(IDLE); // transition occurs on IDLE
  tf.step();
  assert_radio(radio_mode_t::transceive);
  assert_qct(SBDIX);
  assert_fn_num(0);
}


// Transcieve no network
void test_transceive_ok_no_network()
{
  // If we are in TRANS and we complete trans but there is no network
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::read), SBDRB);
  tf.step(); // 0
  tf.step(); // 1
  // Then we should try again
  assert_radio(radio_mode_t::transceive);
  assert_qct(IDLE); // transition occurs on IDLE
  tf.step();
  assert_radio(radio_mode_t::transceive);
  assert_qct(SBDIX);
  assert_fn_num(0);
}

void test_transceive_ok_no_network_timedout()
{
  // If we are in TRANS and we complete trans but there is no network and we out of cycles
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::read), SBDRB);
  tf.cycle_no_fp->set(tf.cycle_no_fp->get() + tf.quake_manager->max_transceive_cycles - 2);
  tf.step(); // 0
  tf.step(); // 1
  // Then we should transition to WAIT
  assert_radio(radio_mode_t::transceive);
  assert_qct(IDLE); // transition occurs on IDLE
  tf.step();
  assert_radio(radio_mode_t::wait);
  assert_qct(IDLE);
  assert_fn_num(0);
}

// Transcieve received MT msg
void test_transceive_ok_with_mt()
{

}
// Transcieve no MT msg
void test_transceive_ok_with_mt()
{

}
// ---------------------------------------------------------------------------
// Error Handling and Recovery
// ---------------------------------------------------------------------------
void test_wrong_state()
{

}

void test_wrong_fn_num()
{

}

// ---------------------------------------------------------------------------
// Helper functions
// ---------------------------------------------------------------------------

void test_transition_radio_state()
{

}

void test_update_mo_new_snap()
{

}

void test_update_mo_same_snap()
{

}

void test_valid_initialization() {
    // If QuakeManager has just been created
    TestFixture tf(static_cast<unsigned int>(radio_mode_t::wait), -1);
    // then we should be in config state and unexpectedFlag should be false
    assert_radio(radio_mode_t::config);
    assert_qct(CONFIG);
    assert_fn_num(0);
    TEST_ASSERT_FALSE(tf.quake_manager->unexpectedFlag);
}

void test_dispatch_config()
{
  // Set the state to config
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::config), CONFIG);
  // Step a bunch of cycles
  tf.step(tf.quake_manager->max_config_cycles);
  // Make sure that we're in wait
  assert_radio(radio_mode_t::wait);
  assert_qct(IDLE);
}

void test_dispatch_wait()
{
  // Set the state to wait
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::wait), IDLE);
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
  // Set the state to write
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::write), SBDWB);
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
  // Set the state to transceive
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::transceive), SBDIX);
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
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::read), SBDRB);
  tf.step(); // sbdrb 0
  tf.step(); //sbdrb 1
  assert_radio(radio_mode_t::write); // should transition to SBDWB
  assert_qct(SBDWB);
}

void test_dispatch_manual()
{
  TestFixture tf(static_cast<unsigned int>(radio_mode_t::manual), IDLE);
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
