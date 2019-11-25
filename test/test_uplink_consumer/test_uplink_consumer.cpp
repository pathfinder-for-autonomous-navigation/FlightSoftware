#include "../StateFieldRegistryMock.hpp"
#define DEBUG
#include "../../src/FCCode/UplinkConsumer.h"
#include <unity.h>
class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::unique_ptr<UplinkConsumer> uplink_consumer;
    
    // Create a TestFixture instance of QuakeManager with the following parameters
    TestFixture() : registry() {
        // Create dummy writable fields


        // Initialize external fields
        char mt_buffer[350]; 

        // Initialize internal fields
        uplink_consumer = std::make_unique<UplinkConsumer>(registry, 0);
        uplink_consumer->get_mt_ready()->set(0);
        uplink_consumer->get_mt_ptr()->set(mt_buffer);
    }
};

// Some fake packets here

void test_valid_initialization() 
{
    TestFixture tf;
}

// ----------------------------------------------------------------------------
// Test UplinkConsumer helper functions
// ----------------------------------------------------------------------------

void test_parse_packet()
{
    // Given a packet

    // Parse the packet into the set of requests

}

void test_check_field()
{
    // If a non writable field is specified

    // Then return false

    // If a writable field is specified

    // Then return true
}

void test_update_field()
{
    // Given a valid field and value

    // Then that field should be updated
}

// ----------------------------------------------------------------------------
// Error Handling Tests
// ----------------------------------------------------------------------------

void test_no_registry()
{
    // If we do not have write access to statefieldregistry, then complain
}

void test_null_mt_packet_ptr()
{
    // If mt_ptr is null, then complain (even if mt_ready is true)
}

// ----------------------------------------------------------------------------
// Specifications (functionality) Tests
// ----------------------------------------------------------------------------

void test_clear_mt_ready()
{
    // If mt_ready is true, 
    
    // Then it should be false after execution
}

void test_perisist_mt_ready()
{
    // If mt_ready is false
    
    // Then it should be false after execution
}

void test_check_ready()
{
    // If mt_ready is false
    
    // Then registry should not be updated
}
void test_do_not_update_non_writable()
{
    // If mt_ready is true but field specified by packet is not writable
    
    // Then that field should not be updated after execution
}

void test_update_writable_field()
{
    // If mt_ready is true && packet indicates an update to writable field

    // Then that field should be updated to the expected value after execution
}

void test_multiple_updates()
{
    // If mt_ready is true && packet requests multiple updates

    // Then all writable fields should be updated
}

// ----------------------------------------------------------------------------
// Invalid Requests
// ----------------------------------------------------------------------------

void test_mixed_validity_updates()
{
    // If mt_ready is true && packet requests multiple updates && some updates
    // are on not writable fields

    // Then do not update any field because packet is probably bad
}

void test_bad_request()
{
    // If mt_ready && packet is bad request

    // Then hopefully detect the bad request and do not update
}


int test_uplink_consumer() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_uplink_consumer();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_uplink_consumer();
}

void loop() {}
#endif