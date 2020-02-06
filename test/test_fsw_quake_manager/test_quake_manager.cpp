#define DEBUG
#include "../StateFieldRegistryMock.hpp"
#define QUAKE_DEBUG
#include <fsw/FCCode/QuakeManager.h>
#include <fsw/FCCode/radio_state_t.enum>

#include <unity.h>

// Check that state x matches the current state of the QuakeControlTask
#define assert_qct(x) {\
  TEST_ASSERT_EQUAL(x, tf.quake_manager->dbg_get_qct().get_current_state());\
  }

// Check that radio state x matches the current radio state
#define assert_radio_state(x) {\
  TEST_ASSERT_EQUAL(static_cast<unsigned int>(x), tf.quake_manager->radio_state_f.get());\
}

// Check that x matches the current fn number
#define assert_fn_num(x) {\
  TEST_ASSERT_EQUAL(x, tf.quake_manager->dbg_get_qct().get_current_fn_number());\
}

// ---------------------------------------------------------------------------
// Test Setup
// ---------------------------------------------------------------------------

unsigned int initCycles = 4242;
// Used in TestFixture constructor
char* snap1 = (char*)
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"\
          "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"\
          "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"\
          "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"\
          "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE";
class TestFixture {
  public:
    StateFieldRegistryMock registry;
    // Input state fields to quake manager
    std::shared_ptr<InternalStateField<char*>> radio_mo_packet_fp;
    std::shared_ptr<InternalStateField<size_t>> snapshot_size_fp;

    // Output state fields from quake manager
    WritableStateField<unsigned int>* max_wait_cycles_fp;
    WritableStateField<unsigned int>* max_transceive_cycles_fp;
    InternalStateField<char*>* radio_mt_packet_fp;
    InternalStateField<size_t>* radio_mt_len_fp;
    ReadableStateField<int>* radio_err_fp;
    InternalStateField<unsigned char>* radio_state_fp;
    InternalStateField<unsigned int>* last_checkin_cycle_fp;

    // Quake manager object
    std::unique_ptr<QuakeManager> quake_manager;
    
    // Create a TestFixture instance of QuakeManager with the following parameters
    TestFixture(unsigned int radio_state, int qct_state) : registry() {
        // Create external field dependencies
        snapshot_size_fp = registry.create_internal_field<size_t>("downlink.snap_size");
        radio_mo_packet_fp = registry.create_internal_field<char*>("downlink.ptr");
        // Initialize external fields
        snapshot_size_fp->set(static_cast<int>(350));
        radio_mo_packet_fp->set(snap1);
        TimedControlTaskBase::control_cycle_count = initCycles;

        // Create Quake Manager instance
        quake_manager = std::make_unique<QuakeManager>(registry, 0);
        max_wait_cycles_fp = registry.find_writable_field_t<unsigned int>("radio.max_wait");
        max_transceive_cycles_fp = registry.find_writable_field_t<unsigned int>("radio.max_transceive");
        radio_mt_packet_fp = registry.find_internal_field_t<char*>("uplink.ptr");
        radio_mt_len_fp = registry.find_internal_field_t<size_t>("uplink.len");
        radio_err_fp = registry.find_readable_field_t<int>("radio.err");
        radio_state_fp = registry.find_internal_field_t<unsigned char>("radio.state");
        last_checkin_cycle_fp = registry.find_internal_field_t<unsigned int>("radio.last_comms_ccno");

        // Initialize internal fields
        if (qct_state != -1) // If qct_state == -1, then expect use the default initialization
        {
          quake_manager->dbg_get_qct().dbg_set_state(qct_state);
          radio_state_fp->set(radio_state);
        }
    }
  // Make a step in the world
  void step(unsigned int amt = 1) {
    TimedControlTaskBase::control_cycle_count += amt; 
    quake_manager->execute(); 
  }
  void realSteps(unsigned int amt = 1)
  {
    for (unsigned int i = 0; i < amt; i++)
    {
      quake_manager->execute();
      TimedControlTaskBase::control_cycle_count++;
    }
  }
};
// ---------------------------------------------------------------------------
// Unexpected error transitions
// ---------------------------------------------------------------------------
void test_wait_unexpected() 
{
  // setup
  TestFixture tf(static_cast<unsigned int>(radio_state_t::wait), IDLE);
  // If in WAIT and unexpected Flag is set
  tf.quake_manager->dbg_get_unexpected_flag() = true;
  // then expect execute all the wait cycles
  tf.realSteps(tf.max_wait_cycles_fp->get());
  assert_radio_state(radio_state_t::wait);
  TEST_ASSERT_TRUE(tf.quake_manager->dbg_get_unexpected_flag());
  // When wait finishes waiting
  tf.step();
  // Clear the error flag and transition to config
  assert_radio_state(radio_state_t::config);
  TEST_ASSERT_FALSE(tf.quake_manager->dbg_get_unexpected_flag());
}

void test_config_unexpected() 
{
  TestFixture tf(static_cast<unsigned int>(radio_state_t::config), CONFIG);
  // If in CONFIG and unexpected Flag is set 
  tf.quake_manager->dbg_get_unexpected_flag() = true;
  // then expect wait
  tf.step();
  assert_qct(IDLE);
  assert_radio_state(radio_state_t::wait);
  assert_fn_num(0);
}
// Helper test Function
void test_rwt_unexpected(int qct_state, unsigned int radio_state)
{
  TestFixture tf(radio_state, qct_state);
  // If in SBDRB, SBDWB, or SBDIX and unexpected Flag is set
  tf.quake_manager->dbg_get_unexpected_flag() = true;
  // then expect transition to Wait
  tf.step();
  assert_qct(IDLE);
  assert_radio_state(radio_state_t::wait);
  assert_fn_num(0);
}
void test_read_unexpected() 
{
  test_rwt_unexpected(SBDRB, static_cast<unsigned int>(radio_state_t::read));
}
void test_write_unexpected() 
{
  test_rwt_unexpected(SBDWB, static_cast<unsigned int>(radio_state_t::write));
}
void test_trans_unexpected() 
{
  test_rwt_unexpected(SBDIX, static_cast<unsigned int>(radio_state_t::transceive));
}

// ---------------------------------------------------------------------------
// No more cycles transitions
// ---------------------------------------------------------------------------
void test_wait_no_more_cycles()
{
  // If in WAIT and there are no more cycles
  TestFixture tf(static_cast<unsigned int>(radio_state_t::wait), IDLE);
  tf.step(tf.max_wait_cycles_fp->get());
  // then expect execute write on the next cycle
  TEST_ASSERT_EQUAL(initCycles + tf.max_wait_cycles_fp->get(), TimedControlTaskBase::control_cycle_count);
  tf.step();
  assert_qct(SBDWB);
  assert_fn_num(0);
  assert_radio_state(static_cast<unsigned int>(radio_state_t::write));
}

// Helper test Function
void test_rwc_no_more_cycles(int qct_state, unsigned int radio_state)
{
  // If in CONFIG, WRITE, READ, TRANS and run out of cycles
  TestFixture tf(radio_state, qct_state);
  tf.quake_manager->dbg_get_unexpected_flag() = false;
  TimedControlTaskBase::control_cycle_count += tf.max_transceive_cycles_fp->get();
  TEST_ASSERT_EQUAL(initCycles + tf.max_transceive_cycles_fp->get(), TimedControlTaskBase::control_cycle_count);
  tf.realSteps();
  tf.realSteps();
  // then expect transition to WAIT and SBDWB, CONFIG, SBDRB should set the error flag
  assert_qct(IDLE);
  assert_radio_state(radio_state_t::wait);
  assert_fn_num(0);
  if (static_cast<unsigned int>(radio_state) != static_cast<unsigned int>(radio_state_t::transceive))
  {
    TEST_ASSERT_TRUE(tf.quake_manager->dbg_get_unexpected_flag());
  }
}

void test_config_no_more_cycles()
{
  test_rwc_no_more_cycles(CONFIG, static_cast<unsigned int>(radio_state_t::config));
}
void test_read_no_more_cycles()
{
  test_rwc_no_more_cycles(SBDRB, static_cast<unsigned int>(radio_state_t::read));
}
void test_write_no_more_cycles()
{
  test_rwc_no_more_cycles(SBDWB, static_cast<unsigned int>(radio_state_t::write));
}
void test_trans_no_more_cycles()
{
  test_rwc_no_more_cycles(SBDIX, static_cast<unsigned int>(radio_state_t::transceive));
}

// ---------------------------------------------------------------------------
// On success transitions
// ---------------------------------------------------------------------------
void test_config_ok()
{
  // If in CONFIG and complete config
  TestFixture tf(static_cast<unsigned int>(radio_state_t::config), CONFIG);
  // Step a bunch of cycles
  tf.step(); // 0
  tf.step(); // 1
  tf.step(); // 2
  assert_fn_num(3); // sanity check
  tf.step(); // 3
  // then expect execute WRITE on the next cycle
  assert_fn_num(0);
  assert_radio_state(radio_state_t::write);
  assert_qct(SBDWB);
}

void test_read_ok()
{
  // If in READ and complete read
  TestFixture tf(static_cast<unsigned int>(radio_state_t::read), SBDRB);
  tf.step(); // 0
  assert_radio_state(radio_state_t::read); // sanity
  tf.step(); // 1
  // then expect execute WRITE on the next cycle
  assert_radio_state(radio_state_t::write);
  assert_qct(SBDWB);
  assert_fn_num(0);
}

void test_write_ok()
{
  // If in WRITE and complete write
  TestFixture tf(static_cast<unsigned int>(radio_state_t::write), SBDWB);
  tf.step(); // 0
  tf.step(); // 1
  assert_radio_state(radio_state_t::write); // sanity
  tf.step(); // 2
  // then expect execute TRANS on the next cycle
  assert_radio_state(radio_state_t::transceive);
  assert_qct(SBDIX);
  assert_fn_num(0);
}


// Transcieve no network
void test_transceive_ok_no_network()
{
  // If in TRANS and complete trans but there is no network
  TestFixture tf(static_cast<unsigned int>(radio_state_t::transceive), SBDIX);
  tf.step(); // 0
  tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[0] = 32;
  tf.step(); // 1
  // then expect try again
  assert_radio_state(radio_state_t::transceive);
  assert_qct(SBDIX);
  assert_fn_num(0);
}

void test_transceive_ok_no_network_timed_out()
{
  // If in TRANS and complete trans but there is no network and out of cycles
  TestFixture tf(static_cast<unsigned int>(radio_state_t::transceive), SBDIX);
  TimedControlTaskBase::control_cycle_count += tf.max_transceive_cycles_fp->get() - 1;
  tf.step(); // 0
  tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[0] = 32;
  tf.step(); // 1
  // then expect transition to WAIT
  assert_radio_state(radio_state_t::wait);
  assert_qct(IDLE);
  assert_fn_num(0);
}

// Transcieve received MT msg
void test_transceive_ok_with_mt()
{
  // If in TRANS and complete trans and have msg
  TestFixture tf(static_cast<unsigned int>(radio_state_t::transceive), SBDIX);
  tf.step(); // 0
  tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[0] = 1;
  tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[2] = 1;
  tf.step(); // 1
  // then expect transition to READ
  assert_radio_state(radio_state_t::read);
  assert_qct(SBDRB);
  assert_fn_num(0);
}

// Transcieve no MT msg
void test_transceive_ok_no_mt()
{
  // If in TRANS and complete trans and have msg
  TestFixture tf(static_cast<unsigned int>(radio_state_t::transceive), SBDIX);
  tf.step(); // 0
  tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[0] = 2;
  tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[2] = 0;
  tf.step(); // 1
  // then expect transition to WRITE to load the next msg
  assert_radio_state(radio_state_t::write);
  assert_qct(SBDWB);
  assert_fn_num(0);
}

// ---------------------------------------------------------------------------
// Error Handling and Recovery
// ---------------------------------------------------------------------------
void test_wrong_state()
{
  // If in a mismatched state
  TestFixture tf(static_cast<unsigned int>(radio_state_t::transceive), CONFIG);
  TEST_ASSERT_FALSE(tf.quake_manager->dbg_get_unexpected_flag());
  tf.step();
  // then expect the error flag should be set and transition to WAIT
  assert_radio_state(radio_state_t::wait);
  assert_qct(IDLE);
  TEST_ASSERT_TRUE(tf.quake_manager->dbg_get_unexpected_flag());
}

// ---------------------------------------------------------------------------
// Helper function tests
// ---------------------------------------------------------------------------

// Tests that wait only transitions when it uses up all of its cycles
void test_oldcycles_do_not_change()
{
  TestFixture tf(static_cast<unsigned int>(radio_state_t::wait), IDLE);
  // Take a step
  tf.realSteps();
  TEST_ASSERT_EQUAL(initCycles, tf.quake_manager->dbg_get_last_checkin().get());
  // Fake step until we are almost out of cycles
  tf.realSteps(tf.max_wait_cycles_fp->get() - 1);
  // Make sure state is still in wait
  TEST_ASSERT_EQUAL(initCycles, tf.quake_manager->dbg_get_last_checkin().get());
  // Take the last step
  tf.step();
  // Expect to no longer be in wait
  TEST_ASSERT_EQUAL(TimedControlTaskBase::control_cycle_count, tf.quake_manager->dbg_get_last_checkin().get());
}

void test_transition_radio_state()
{
  TestFixture tf(static_cast<unsigned int>(radio_state_t::wait), IDLE);
  // If I call transition radio state
  tf.step();
  TEST_ASSERT_EQUAL(initCycles, tf.quake_manager->dbg_get_last_checkin().get());
  tf.quake_manager->dbg_transition_radio_state(radio_state_t::config);
  // then expect my cycles should be updated
  assert_radio_state(static_cast<unsigned int>(radio_state_t::config));
  TEST_ASSERT_EQUAL(initCycles+1, tf.quake_manager->dbg_get_last_checkin().get());
}

void test_no_more_cycles()
{
  TestFixture tf(static_cast<unsigned int>(radio_state_t::config), CONFIG);
  // If in config and there are no more cycles
  TimedControlTaskBase::control_cycle_count = initCycles + tf.quake_manager->max_config_cycles;
  tf.step();
  // then expect reset counter and transition to wait
  TEST_ASSERT_EQUAL(tf.quake_manager->dbg_get_last_checkin().get(), 
    initCycles + tf.quake_manager->max_config_cycles + 1);
  assert_radio_state(static_cast<unsigned int>(radio_state_t::wait));
}

// ---------------------------------------------------------------------------
// Snapshot buffers and loading MO buffer
// ---------------------------------------------------------------------------

char* snap2 = (char*)
          "1111111111111111111111111111111111111111111111111111111111111111111111"\
          "2222222222222222222222222222222222222222222222222222222222222222222222"\
          "3333333333333333333333333333333333333333333333333333333333333333333333"\
          "4444444444444444444444444444444444444444444444444444444444444444444444"\
          "5555555555555555555555555555555555555555555555555555555555555555555555";

void check_buf_bytes(const char *buf1, const char *buf2, size_t size)
{
  for (size_t i = 0; i < size; i++)
  {
    if (buf1[i] != buf2[i])
    {
        TEST_ASSERT_EQUAL(buf1[i], buf2[i]);
    }
  }
}

void test_write_load_message()
{
  // If we've executed the first request to write
  TestFixture tf(static_cast<unsigned int>(radio_state_t::write), SBDWB);
  TEST_ASSERT_EQUAL_STRING(snap1, tf.radio_mo_packet_fp->get());
  TEST_ASSERT_EQUAL(0, tf.quake_manager->dbg_get_mo_idx());
  TEST_ASSERT_EQUAL(0, tf.quake_manager->dbg_get_qct().get_current_fn_number());
  tf.step();
  // then expect the first seciton of the snapshot should be loaded
  TEST_ASSERT_EQUAL(70, tf.quake_manager->dbg_get_qct().dbg_get_MO_len());
  check_buf_bytes(
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 
    tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len()); 
  tf.step();
}

// Test that when MO is set to a new snapshot, SBDWB will still finish sending
// the old snapshot
void test_update_mo_same_snap()
{
  // If SBDWB has written the first piece of the snapshot
  TestFixture tf(static_cast<unsigned int>(radio_state_t::write), SBDWB);
  TEST_ASSERT_EQUAL_STRING(snap1, tf.radio_mo_packet_fp->get());
  tf.realSteps(); // sbdwb 0
  TEST_ASSERT_EQUAL_STRING(snap1, tf.quake_manager->dbg_get_qct().dbg_get_MO_msg());
  tf.radio_mo_packet_fp->set(snap2);  
  tf.realSteps(); // 1
  tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[2] = 0; // have comms but no msg
  tf.realSteps(); // 2
  tf.radio_mo_packet_fp->set(snap2); 
  // Execute SBDIX
  TEST_ASSERT_EQUAL_STRING(snap1, tf.quake_manager->dbg_get_qct().dbg_get_MO_msg());
  tf.realSteps(); // sbdix 0
  tf.radio_mo_packet_fp->set(snap2); 
  tf.realSteps(); // sbdix 1
  tf.radio_mo_packet_fp->set(snap2); 
  TEST_ASSERT_EQUAL_STRING(snap1, tf.quake_manager->dbg_get_qct().dbg_get_MO_msg());
  // Return to SBDWB
  tf.realSteps(); // sbdwb 0
  // then expect the second piece of the snapshot should be loaded on the next write
  check_buf_bytes(
    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB", 
    tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len()); 
}

void test_update_mo_reset_idx()
{
  // If writing the last piece of the snapshot
  TestFixture tf(static_cast<unsigned int>(radio_state_t::write), SBDWB);
  tf.realSteps(3); // sbdwb 0, 1, 2
  tf.radio_mo_packet_fp->set(snap2);
  // Execute SBDIX
  TEST_ASSERT_EQUAL(SBDIX, tf.quake_manager->dbg_get_qct().get_current_state());
  tf.quake_manager->dbg_get_mo_idx() = 4; // pretend on the last snapshot
  tf.realSteps(2); // sbdix 0, 1
  TEST_ASSERT_EQUAL_STRING(snap1, tf.quake_manager->dbg_get_qct().dbg_get_MO_msg());

  // then expect the last piece should be loaded and mo_idx reset to 0
  TEST_ASSERT_EQUAL(SBDWB, tf.quake_manager->dbg_get_qct().get_current_state());
  TEST_ASSERT_EQUAL(0, tf.quake_manager->dbg_get_qct().get_current_fn_number());
  tf.step(); // 0
  TEST_ASSERT_EQUAL(1, tf.quake_manager->dbg_get_qct().get_current_fn_number());
  check_buf_bytes(
    "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE", 
    tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len()); 
  tf.realSteps(2); // 1, 2
  tf.radio_mo_packet_fp->set(snap2);
  // then expect mo_idx should be reset to 0 but mo_buffer should still be the same
  TEST_ASSERT_EQUAL(0, tf.quake_manager->dbg_get_mo_idx());
  TEST_ASSERT_EQUAL_STRING(snap1, tf.quake_manager->dbg_get_mo_buffer_copy());
}

void test_update_mo_load_new_snap()
{
  // If have written the last piece of the first snapshot
  TestFixture tf(static_cast<unsigned int>(radio_state_t::write), SBDWB);
  // Read Snap 1 (which is already loaded in the constructor of the test fixture)
  tf.realSteps(3);
  tf.radio_mo_packet_fp->set(snap2);
  tf.realSteps(2);
  for (int i = 0; i < 4; i++)
  {
    // Read Snap 1 Part i+1
    tf.realSteps(3);
    tf.realSteps(2);
  }

  tf.step(); // sbdwb 0
  // then expect snap2 should be copied to mo_buffer and snap2 part 1 loaded
  TEST_ASSERT_EQUAL_STRING(snap2, tf.quake_manager->dbg_get_qct().dbg_get_MO_msg());
  check_buf_bytes(
    "1111111111111111111111111111111111111111111111111111111111111111111111", 
    tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len()); 
  tf.step(); 
}

void test_valid_initialization() 
{
    // If QuakeManager has just been created
    TestFixture tf(static_cast<unsigned int>(radio_state_t::wait), -1);

    // Expect the configured state machine waits to be correct
    TEST_ASSERT_EQUAL(1, tf.max_wait_cycles_fp->get());
    TEST_ASSERT_EQUAL(500, tf.max_transceive_cycles_fp->get());

    // then expect be in config state and unexpected_flag should be false
    assert_radio_state(radio_state_t::config);
    assert_qct(CONFIG);
    assert_fn_num(0);
    TEST_ASSERT_EQUAL(0, tf.quake_manager->dbg_get_mo_idx());
    TEST_ASSERT_FALSE(tf.quake_manager->dbg_get_unexpected_flag());
}

void test_mt_ready_set_after_sbdrb_success()
{
    TestFixture tf(static_cast<unsigned int>(radio_state_t::read), SBDRB);
    assert_radio_state(radio_state_t::read);
    TEST_ASSERT_EQUAL(0, tf.radio_mt_len_fp->get());
    tf.realSteps(1);
    assert_qct(SBDRB); 
    tf.realSteps(1);
    assert_qct(SBDWB); // Should transition to SBDWB after finishing SBDRB
    TEST_ASSERT_EQUAL(tf.quake_manager->dbg_get_qct().get_MT_length(), tf.radio_mt_len_fp->get());
}

int test_quake_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_wait_unexpected); 
    RUN_TEST(test_config_unexpected); 
    RUN_TEST(test_read_unexpected); 
    RUN_TEST(test_write_unexpected); 
    RUN_TEST(test_trans_unexpected); 
    RUN_TEST(test_wait_no_more_cycles);
    RUN_TEST(test_config_no_more_cycles);
    RUN_TEST(test_read_no_more_cycles);
    RUN_TEST(test_write_no_more_cycles);
    RUN_TEST(test_trans_no_more_cycles);
    RUN_TEST(test_config_ok);
    RUN_TEST(test_read_ok);
    RUN_TEST(test_write_ok);
    RUN_TEST(test_transceive_ok_no_network);
    RUN_TEST(test_transceive_ok_no_network_timed_out);
    RUN_TEST(test_transceive_ok_with_mt);
    RUN_TEST(test_transceive_ok_no_mt);
    RUN_TEST(test_wrong_state);
    RUN_TEST(test_oldcycles_do_not_change);
    RUN_TEST(test_transition_radio_state);
    RUN_TEST(test_no_more_cycles);
    RUN_TEST(test_write_load_message);
    RUN_TEST(test_update_mo_same_snap);
    RUN_TEST(test_update_mo_reset_idx);
    RUN_TEST(test_update_mo_load_new_snap);
    RUN_TEST(test_valid_initialization); 
    RUN_TEST(test_mt_ready_set_after_sbdrb_success);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_quake_manager();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_quake_manager();
}

void loop() {}
#endif
/*
parse to test
cat test/test_quake_manager//test_quake_manager.cpp | grep "void test_" | sed 's/^void \(.*\)$/\1/' | sed 's/()/);/g'| sed -e 's/^/RUN_TEST(/'
*/