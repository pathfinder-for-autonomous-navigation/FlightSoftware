#define DEBUG
#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/QuakeManager.h>
#include <fsw/FCCode/radio_state_t.enum>

#include <unity.h>

// Check that radio state x matches the current radio state
#define assert_radio_state(x) {\
  TEST_ASSERT_EQUAL(static_cast<unsigned int>(x), tf.quake_manager->radio_state_f.get());\
}

// Check that x matches the current fn number
#define assert_fn_num(x) {\
  TEST_ASSERT_EQUAL(x, tf.quake_manager->dbg_get_qct().get_fn_num());\
}

// ---------------------------------------------------------------------------
// Test Setup
// ---------------------------------------------------------------------------

unsigned int initCycles = 4242;
// Used in TestFixture constructor
const char* snap1 =
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
    TestFixture(unsigned int radio_state) : registry() {
        // Create external field dependencies
        snapshot_size_fp = registry.create_internal_field<size_t>("downlink.snap_size");
        radio_mo_packet_fp = registry.create_internal_field<char*>("downlink.ptr");
        // Initialize external fields
        snapshot_size_fp->set(static_cast<int>(350));
        radio_mo_packet_fp->set((char*)snap1);
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

        radio_state_fp->set(radio_state);

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

    int execUntilChange(unsigned int max_amt = 1000)
    {
        unsigned char current = radio_state_fp->get();
        for (size_t i = 0; i < max_amt; ++i)
        {
            quake_manager->execute();
            TimedControlTaskBase::control_cycle_count++;
            if (current != radio_state_fp->get())
                return i;
        }
        return -1;
    }
};
// ---------------------------------------------------------------------------
// Unexpected error transitions
// ---------------------------------------------------------------------------
void test_wait_unexpected()
{
    // setup
    TestFixture tf(static_cast<unsigned int>(radio_state_t::wait));
    TEST_ASSERT_EQUAL(initCycles, tf.quake_manager->dbg_get_cycle_of_entry());
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

// ---------------------------------------------------------------------------
// No more cycles transitions
// ---------------------------------------------------------------------------
void test_wait_no_more_cycles()
{
    // If in WAIT and there are no more cycles
    TestFixture tf(static_cast<unsigned int>(radio_state_t::wait));
    tf.realSteps(tf.max_wait_cycles_fp->get());
    // then expect execute write on the next cycle
    TEST_ASSERT_EQUAL(initCycles + tf.max_wait_cycles_fp->get(), TimedControlTaskBase::control_cycle_count);
    tf.step();
    assert_fn_num(0);
    assert_radio_state(static_cast<unsigned int>(radio_state_t::write));
}

// Helper test Function
void test_rwc_no_more_cycles(unsigned int radio_state, unsigned int max_cycles)
{
    // If in CONFIG, WRITE, READ and run out of cycles
    TestFixture tf(radio_state);
    TimedControlTaskBase::control_cycle_count += max_cycles;
    TEST_ASSERT_EQUAL(initCycles + max_cycles, TimedControlTaskBase::control_cycle_count);
    tf.realSteps();
    // then expect transition to WAIT with the error flag set
    assert_radio_state(radio_state_t::wait);
    TEST_ASSERT_TRUE(tf.quake_manager->dbg_get_unexpected_flag());
    tf.realSteps(tf.quake_manager->max_wait_cycles_f.get());
    // then wait should transition to config after it clears the error flag
    assert_radio_state(radio_state_t::config);
    assert_fn_num(0);
    TEST_ASSERT_FALSE(tf.quake_manager->dbg_get_unexpected_flag());

}

void test_config_no_more_cycles()
{
    test_rwc_no_more_cycles(static_cast<unsigned int>(radio_state_t::config),5);
}
void test_read_no_more_cycles()
{
    test_rwc_no_more_cycles(static_cast<unsigned int>(radio_state_t::read), 5);
}
void test_write_no_more_cycles()
{
    test_rwc_no_more_cycles(static_cast<unsigned int>(radio_state_t::write), 5);
}
void test_trans_no_more_cycles()
{
    test_rwc_no_more_cycles(static_cast<unsigned int>(radio_state_t::transceive), 500);
}

// ---------------------------------------------------------------------------
// On success transitions
// ---------------------------------------------------------------------------
void test_config_ok()
{
    // If in CONFIG and complete config
    TestFixture tf(static_cast<unsigned int>(radio_state_t::config));
    // Step a bunch of cycles
    tf.step(); // 0
    tf.step(); // 1
    tf.step(); // 2
    assert_fn_num(3); // sanity check
    tf.step(); // 3
    // then expect execute WRITE on the next cycle
    assert_fn_num(0);
    assert_radio_state(radio_state_t::write);
}

void test_read_ok()
{
    // If in READ and complete read
    TestFixture tf(static_cast<unsigned int>(radio_state_t::read));
    tf.step(); // 0
    assert_radio_state(radio_state_t::read); // sanity
    tf.step(); // 1
    // then expect execute WRITE on the next cycle
    assert_radio_state(radio_state_t::write);
    assert_fn_num(0);
}

void test_write_ok()
{
    // If in WRITE and complete write
    TestFixture tf(static_cast<unsigned int>(radio_state_t::write));
    tf.step(); // 0
    tf.step(); // 1
    assert_radio_state(radio_state_t::write); // sanity
    tf.step(); // 2
    // then expect execute TRANS on the next cycle
    assert_radio_state(radio_state_t::transceive);
    assert_fn_num(0);
}


// Transcieve no network
void test_transceive_ok_no_network()
{
    // If in TRANS and complete trans but there is no network
    TestFixture tf(static_cast<unsigned int>(radio_state_t::transceive));
    tf.step(); // 0
    tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[0] = 32;
    tf.step(); // 1
    // then expect try again
    assert_radio_state(radio_state_t::transceive);
    assert_fn_num(0);
}

void test_transceive_ok_no_network_timed_out()
{
    // If in TRANS and complete trans but there is no network
    TestFixture tf(static_cast<unsigned int>(radio_state_t::transceive));
    tf.step(); // 0
    tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[0] = 32;
    tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[2] = 1;
    tf.step(); // 1
    // then expect transition to transceive a couple times
    assert_radio_state(radio_state_t::transceive);
    tf.realSteps(tf.quake_manager->max_transceive_cycles_f.get()/3);
    assert_radio_state(radio_state_t::transceive);
    tf.realSteps(tf.quake_manager->max_transceive_cycles_f.get()/3);
    assert_radio_state(radio_state_t::write);
    assert_fn_num(0);
}

// Transcieve received MT msg
void test_transceive_ok_with_mt()
{
    // If in TRANS and complete trans and have msg
    TestFixture tf(static_cast<unsigned int>(radio_state_t::transceive));
    tf.step(); // 0
    tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[0] = 1;
    tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[2] = 1;
    tf.step(); // 1
    // then expect transition to READ
    assert_radio_state(radio_state_t::read);
    assert_fn_num(0);
}

// Transcieve no MT msg
void test_transceive_ok_no_mt()
{
    // If in TRANS and complete trans and have msg
    TestFixture tf(static_cast<unsigned int>(radio_state_t::transceive));
    tf.step(); // 0
    tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[0] = 2;
    tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[2] = 0;
    tf.step(); // 1
    // then expect transition to WRITE to load the next msg
    assert_radio_state(radio_state_t::write);
    assert_fn_num(0);
}


// ---------------------------------------------------------------------------
// Helper function tests
// ---------------------------------------------------------------------------

// Tests that wait only transitions when it uses up all of its cycles
void test_oldcycles_do_not_change()
{
    TestFixture tf(static_cast<unsigned int>(radio_state_t::wait));
    // Take a step
    tf.realSteps();
    TEST_ASSERT_EQUAL(initCycles, tf.quake_manager->dbg_get_cycle_of_entry());
    // Fake step until we are almost out of cycles
    tf.realSteps(tf.max_wait_cycles_fp->get() - 1);
    // Make sure state is still in wait
    TEST_ASSERT_EQUAL(initCycles, tf.quake_manager->dbg_get_cycle_of_entry());
    // Take the last step
    tf.step();
    // Expect to no longer be in wait
    TEST_ASSERT_EQUAL(TimedControlTaskBase::control_cycle_count, tf.quake_manager->dbg_get_cycle_of_entry());
}

void test_transition_radio_state()
{
    TestFixture tf(static_cast<unsigned int>(radio_state_t::wait));
    // If I call transition radio state
    tf.realSteps(tf.quake_manager->max_wait_cycles_f.get());
    TEST_ASSERT_EQUAL(initCycles, tf.quake_manager->dbg_get_cycle_of_entry());
    tf.quake_manager->dbg_transition_radio_state(radio_state_t::config);
    // then expect my cycles should be updated
    assert_radio_state(static_cast<unsigned int>(radio_state_t::config));
    TEST_ASSERT_EQUAL(initCycles+1, tf.quake_manager->dbg_get_cycle_of_entry());
}

void test_no_more_cycles()
{
    TestFixture tf(static_cast<unsigned int>(radio_state_t::config));
    // If in config and there are no more cycles
    TimedControlTaskBase::control_cycle_count = initCycles + tf.quake_manager->max_config_cycles;
    tf.step();
    // then expect reset counter and transition to wait
    TEST_ASSERT_EQUAL(tf.quake_manager->dbg_get_cycle_of_entry(),
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
    TestFixture tf(static_cast<unsigned int>(radio_state_t::write));
    TEST_ASSERT_EQUAL_STRING(snap1, tf.radio_mo_packet_fp->get());
    TEST_ASSERT_EQUAL(0, tf.quake_manager->dbg_get_mo_idx());
    TEST_ASSERT_EQUAL(0, tf.quake_manager->dbg_get_qct().get_fn_num());
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
    TestFixture tf(static_cast<unsigned int>(radio_state_t::write));
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

void test_update_mo_load_new_snap()
{
    // If writing the last piece of the snapshot
    TestFixture tf(static_cast<unsigned int>(radio_state_t::write));
    tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[0] = 1; // we have network
    tf.execUntilChange();
    tf.execUntilChange();

    tf.quake_manager->dbg_get_mo_idx() = (tf.quake_manager->snapshot_size_fp->get()/70) - 2; // writing the second to last packet
    tf.execUntilChange(); // should be writing DDDD...
    check_buf_bytes(
            "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD",
            tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len());
    assert_radio_state(radio_state_t::transceive);
    tf.execUntilChange();
    // Make sure that non constant array of chars work
    char snap2_copy[strlen(snap2) + 1];
    memcpy(snap2_copy, snap2, strlen(snap2) + 1);
    snap2_copy[strlen(snap2)-1] = '9'; // make the last character a 9, make sure the last snap is 5555...9

    tf.radio_mo_packet_fp->set(snap2_copy);
    assert_radio_state(radio_state_t::write);
    tf.execUntilChange(); // should be writing EEEE...
    check_buf_bytes(
            "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE",
            tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len());
    assert_radio_state(radio_state_t::transceive);
    tf.execUntilChange();
    assert_radio_state(radio_state_t::write);
    tf.execUntilChange(); // should be writing 1111...
    check_buf_bytes(
            "1111111111111111111111111111111111111111111111111111111111111111111111",
            tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len());
    assert_radio_state(radio_state_t::transceive);
    tf.execUntilChange();
    assert_radio_state(radio_state_t::write);
    tf.execUntilChange(); // should be writing 2222...
    assert_radio_state(radio_state_t::transceive);
    tf.execUntilChange();
    assert_radio_state(radio_state_t::write);
    tf.execUntilChange(); // should be writing 3333...
    assert_radio_state(radio_state_t::transceive);
    tf.execUntilChange();
    assert_radio_state(radio_state_t::write);
    tf.execUntilChange(); // should be writing 4444...
    assert_radio_state(radio_state_t::transceive);
    tf.execUntilChange();
    assert_radio_state(radio_state_t::write);
    tf.execUntilChange(); // should be writing 55555...9
    check_buf_bytes(
            "5555555555555555555555555555555555555555555555555555555555555555555559",
            tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len());
    assert_radio_state(radio_state_t::transceive);
    tf.execUntilChange();
    assert_radio_state(radio_state_t::write);
    tf.execUntilChange(); // should be writing 1111... again
    check_buf_bytes(
            "1111111111111111111111111111111111111111111111111111111111111111111111",
            tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len());
}


void test_new_snap_after_sbdix_fail()
{
    // If we write half a packet and then we run out of SBDIX cycles, then the next time we have sbdix, we should be writing new snapshot
    TestFixture tf(static_cast<unsigned int>(radio_state_t::write));
    tf.execUntilChange(); // write AAA
    tf.execUntilChange(); // transcieve
    tf.execUntilChange(); // write BBB
    tf.execUntilChange(); // transcieve
    tf.execUntilChange(); // write CCC
    tf.execUntilChange(); // transcieve
    tf.quake_manager->dbg_get_qct().dbg_get_quake().sbdix_r[0] = 32; // we have no network now
    tf.execUntilChange(); // should be writing DDDD...
    check_buf_bytes(
            "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD",
            tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len());
    assert_radio_state(radio_state_t::transceive);
    tf.radio_mo_packet_fp->set(snap2);
    tf.execUntilChange(); // transceive will keep trying until it fewer than 1/3 max_transcieve_cycles left
    assert_radio_state(radio_state_t::write); // then it will transition to write
    tf.execUntilChange(); // it should be writing the NEW snapshot
    check_buf_bytes(
            "1111111111111111111111111111111111111111111111111111111111111111111111",
            tf.quake_manager->dbg_get_qct().dbg_get_MO_msg(), tf.quake_manager->dbg_get_qct().dbg_get_MO_len());
}

void test_valid_initialization()
{
    // If QuakeManager has just been created in disabled mode
    TestFixture tf(static_cast<unsigned int>(radio_state_t::disabled));

    // Waiting for someone to put us into config
    tf.realSteps(100000);

    tf.radio_state_fp->set(static_cast<unsigned char>(radio_state_t::config));
    tf.realSteps();
    assert_radio_state(radio_state_t::config);
    assert_fn_num(1);
    TEST_ASSERT_EQUAL(0, tf.quake_manager->dbg_get_mo_idx());
    TEST_ASSERT_FALSE(tf.quake_manager->dbg_get_unexpected_flag());
}

void test_mt_ready_set_after_sbdrb_success()
{
    TestFixture tf(static_cast<unsigned int>(radio_state_t::read));
    assert_radio_state(radio_state_t::read);
    TEST_ASSERT_EQUAL(0, tf.radio_mt_len_fp->get());
    tf.realSteps(1);
    tf.realSteps(1);
    TEST_ASSERT_EQUAL(tf.quake_manager->dbg_get_qct().get_MT_length(), tf.radio_mt_len_fp->get());
}

int test_quake_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_wait_unexpected);
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
    RUN_TEST(test_oldcycles_do_not_change);
    RUN_TEST(test_transition_radio_state);
    RUN_TEST(test_no_more_cycles);
    RUN_TEST(test_write_load_message);
    RUN_TEST(test_update_mo_same_snap);
    RUN_TEST(test_update_mo_load_new_snap);
    RUN_TEST(test_new_snap_after_sbdix_fail);
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