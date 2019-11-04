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
    std::shared_ptr<WritableStateField<char[340]>> mt_msg_fp;
    std::shared_ptr<WritableStateField<char[340]>> mo_msg_fp;

    std::unique_ptr<QuakeManager> quake_manager;

    TestFixture() : registry() {
        cycle_no_fp = registry.create_writable_field<unsigned int>("pan.cycle_no", 0, cycleNumber, 32);
        mt_msg_fp = registry.create_writable_field<char[340]>("radio.mt_msg_queue");
        mo_msg_fp = registry.create_writable_field<char[340]>("radio.mo_msg_queue");
        radio_mode_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("radio.mode"));
        quake_manager = std::make_unique<QuakeManager>(registry);
    }
  void incr_time(unsigned int amt = 1) {
    cycleNumber += amt;
    cycle_no_fp->set(static_cast<unsigned int>(cycleNumber));  
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
  tf.quake_manager.execute();
  TEST_ASSERT_EQUAL(static_cast<unsigned int>(radio_mode_t::waiting), tf.radio_mode_fp->get());
  // Pretend time has passed (3001 control cycles later...)
  tf.incr_time(3001);
  tf.quake_manager.execute(); 
  TEST_ASSERT_EQUAL(SBDWB, static_cast<unsigned int>(tf.quake_manager.qct.get_current_state()));

  tf.incr_time();
  tf.quake_manager.execute();
  TEST_ASSERT_EQUAL(static_cast<unsigned int>(radio_mode_t::transceiving), tf.radio_mode_fp->get());
  

}

void test_dispatch_manual()
{
  TestFixture tf;
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::manual));

}

void test_dispatch_transceiving()
{
  TestFixture tf;
  tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::transceiving));

}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_dispatch_startup);
    RUN_TEST(test_dispatch_manual);
    RUN_TEST(test_dispatch_transceiving);
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
