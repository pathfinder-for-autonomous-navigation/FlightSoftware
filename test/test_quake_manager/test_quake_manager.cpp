#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../../src/FCCode/QuakeManager.h"
#include "../../src/FCCode/radio_mode_t.enum"

#include <unity.h>
unsigned int cycleNumber = 4294967295;
class TestFixture {
  public:
    StateFieldRegistryMock registry;
    // Input state fields to quake manager
    std::shared_ptr<WritableStateField<unsigned int>> cycle_no_fp;

    // Output state fields from quake manager
    std::shared_ptr<WritableStateField<unsigned int>> radio_mode_fp;
    std::shared_ptr<WritableStateField<char*>> mt_msg_fp;
    std::shared_ptr<WritableStateField<char*>> mo_msg_fp;

    std::unique_ptr<QuakeManager> quake_manager;
    // Not sure if this is how it works


    TestFixture() : registry() {
        cycle_no_fp = registry.create_writable_field<unsigned int>("pan.cycle_no", 0, cycleNumber, 32);
        mt_msg_fp = registry.create_writable_field<char*>("radio.mt_msg_queue");
        mo_msg_fp = registry.create_writable_field<char*>("radio.mo_msg_queue");
        radio_mode_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("radio.mode"));
  
        quake_manager = std::make_unique<QuakeManager>(registry);
    }
  // Make a step in the world
  void step(unsigned int amt = 1) {
    cycleNumber += amt;
    cycle_no_fp->set(static_cast<unsigned int>(cycleNumber)); 
    quake_manager->execute(); 
  }
};

void test_valid_initialization() {
    TestFixture tf;
    tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::startup));
}

void test_dispatch_startup()
{
  TestFixture tf;
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::startup));
  
}


void test_dispatch_waiting()
{
  TestFixture tf;
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::waiting));
  tf.step();
  TEST_ASSERT_EQUAL(static_cast<unsigned int>(radio_mode_t::waiting), tf.radio_mode_fp->get());
  tf.step(2999);
  // Make sure we are still in waiting mode when 3000 cycles have not past
  TEST_ASSERT_EQUAL(static_cast<unsigned int>(radio_mode_t::waiting), tf.radio_mode_fp->get());

  tf.step(); // query_sbdwb_1 fails bc no message
  TEST_ASSERT_EQUAL(static_cast<unsigned int>(radio_mode_t::transceiving), tf.radio_mode_fp->get());
}

void test_dispatch_manual()
{
  TestFixture tf;
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::manual));

}

void test_dispatch_transceiving_comms()
{
  TestFixture tf;
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::transceiving));
  tf.step(); // should fail
  TEST_ASSERT_EQUAL(IDLE, static_cast<unsigned int>(tf.quake_manager->qct.get_current_state()));
  char mymessage [] = "here's a message";
  tf.mo_msg_fp->set(mymessage);
  tf.step();  // should be good this time
  TEST_ASSERT_EQUAL(SBDWB, static_cast<unsigned int>(tf.quake_manager->qct.get_current_state()));
  tf.step(); // sbdwb_query_2
  tf.step(); // get sbdwb response

  TEST_ASSERT_EQUAL(SBDIX, static_cast<unsigned int>(tf.quake_manager->qct.get_current_state())); 
  // query sbdix
  tf.step(); // query_sbdix_1
  TEST_ASSERT_EQUAL(SBDIX, static_cast<unsigned int>(tf.quake_manager->qct.get_current_state()));

  int* sbdix_response = tf.quake_manager->qct.quake.sbdix_r;
  sbdix_response[0] = 0;
  sbdix_response[1] = 1;
  tf.step(); // get_sbdix
  TEST_ASSERT_EQUAL(IDLE, static_cast<unsigned int>(tf.quake_manager->qct.get_current_state()));
  tf.step();
  TEST_ASSERT_EQUAL(SBDRB, static_cast<unsigned int>(tf.quake_manager->qct.get_current_state()));
  tf.step();
  TEST_ASSERT_EQUAL(IDLE, static_cast<unsigned int>(tf.quake_manager->qct.get_current_state()));

}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_dispatch_startup);
    RUN_TEST(test_dispatch_manual);
    RUN_TEST(test_dispatch_transceiving_comms);
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
