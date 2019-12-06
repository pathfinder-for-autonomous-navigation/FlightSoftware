#include "../StateFieldRegistryMock.hpp"
#define DEBUG
#include "../../src/FCCode/UplinkConsumer.h"
#include "../../src/FCCode/BitStream.h"
#include <unity.h>
#include <map>
extern int test_bitstream();

// convert n to bits
void from_ull(WritableStateFieldBase* w, uint64_t n)
{
    std::vector<bool> feels_bad (w->get_bit_array().size(), 0);
    for (size_t i = 0; i < w->get_bit_array().size(); ++i)
    {
        feels_bad[i] = n&1;
        n >>= 1;
    }
    change_bit_arr(w, feels_bad);
}

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::unique_ptr<UplinkConsumer> uplink_consumer;
    InternalStateField<size_t>* radio_mt_packet_len_fp;
    InternalStateField<char*>* radio_mt_packet_fp;

    std::shared_ptr<WritableStateField<unsigned int>> cycle_no_fp;
    std::shared_ptr<WritableStateField<unsigned char>> adcs_mode_fp;
    std::shared_ptr<WritableStateField<f_quat_t>> adcs_cmd_attitude_fp;
    std::shared_ptr<ReadableStateField<float>> adcs_ang_rate_fp;
    std::shared_ptr<WritableStateField<float>> adcs_min_stable_ang_rate_fp;
    std::shared_ptr<WritableStateField<unsigned char>> mission_mode_fp;
    std::shared_ptr<WritableStateField<unsigned char>> sat_designation_fp;

    // Test Helper function will map field names to indices
    std::map<std::string, size_t> field_map;

    size_t index_size;
    
    // Create a TestFixture instance of QuakeManager with the following parameters
    TestFixture() : registry() {
        // Create dummy fields
        cycle_no_fp = registry.create_writable_field<unsigned int>("pan.cycle_no");
        adcs_mode_fp = registry.create_writable_field<unsigned char>("adcs.mode", 10);
        adcs_cmd_attitude_fp = registry.create_writable_field<f_quat_t>("adcs.cmd_attitude");
        adcs_ang_rate_fp = registry.create_readable_field<float>("adcs.ang_rate", 0, 10, 4);
        adcs_min_stable_ang_rate_fp = registry.create_writable_field<float>("adcs.min_stable_ang_rate", 0, 10, 4);
        mission_mode_fp = registry.create_writable_field<unsigned char>("pan.mode");
        sat_designation_fp = registry.create_writable_field<unsigned char>("pan.sat_designation");

        // Make external fields
        char mt_buffer[350]; 

        // Initialize internal fields
        uplink_consumer = std::make_unique<UplinkConsumer>(registry, 0);
        radio_mt_packet_len_fp = registry.find_internal_field_t<size_t>("uplink_consumer.mt_len");
        radio_mt_packet_fp = registry.find_internal_field_t<char*>("uplink_consumer.mt_ptr");

        radio_mt_packet_fp->set(mt_buffer);
        field_map = std::map<std::string, size_t>();

        // Setup field_map and assign some values
        unsigned int i = 0;
        for (; i < registry.writable_fields.size(); ++i)
        {
            auto w = registry.writable_fields[i];
            field_map[w->name().c_str()] = i;
            w->name();
            from_ull(w, rand());
        }

        for (index_size = 1; i / (1 << index_size) > 0; ++index_size){}

    }
    /**
     * @param out The BitStream representation of the packet that is being constructed
     * @param value The value that we'd like to put into the packet
     * @param index The index of the writable field that we want to make the data for
     */
    void create_uplink( BitStream& out, uint8_t* value, size_t index)
    {
        auto bit_arr = registry.writable_fields[index]->get_bit_array();
        size_t field_size = bit_arr.size();
        BitStream bs_idx(reinterpret_cast<char*>(&index), index_size);
        BitStream bs_data(reinterpret_cast<char*>(value), field_size);
        bs_idx << out;
        bs_data << out;
    }

};

void test_create_uplink()
{
    TestFixture tf;
}

void test_valid_initialization() 
{
    TestFixture tf;
    TEST_ASSERT_EQUAL(0, tf.radio_mt_packet_len_fp->get());
    TEST_ASSERT_NOT_NULL(tf.radio_mt_packet_fp->get());
}

// ----------------------------------------------------------------------------
// Test UplinkConsumer helper functions
// ----------------------------------------------------------------------------


void test_get_field_length()
{
    TestFixture tf;
    // If bad idx is specified
    // Then return 0
    TEST_ASSERT_EQUAL(0, tf.uplink_consumer->get_field_length(-1));
    TEST_ASSERT_EQUAL(0, tf.uplink_consumer->get_field_length(tf.registry.writable_fields.size()));

    // If a valid idx is specified
    size_t idx = tf.field_map["adcs.mode"];
    // Then return the length of the bit array
    TEST_ASSERT_EQUAL(4, tf.uplink_consumer->get_field_length(idx));
}

void test_update_field()
{
    TestFixture tf;
    // Given a valid field and value
    size_t idx = tf.field_map["pan.mode"];
    auto field = tf.registry.writable_fields[idx];

    // Get field sizes and the bit array
    size_t field_size = field->get_bit_array().size();
    auto field_bit_arr = field->get_bit_array();

    // Set fake data
    char* fake_data = (char*)"\xce\xfa\xef\xbe\xaa\xbb\xcc\xdd"; 
    tf.radio_mt_packet_fp->set(fake_data);
    tf.radio_mt_packet_len_fp->set(8);

    tf.uplink_consumer->update_field(idx);
    
    // Check results
    field_bit_arr = field->get_bit_array();
    // Make expected result
    BitStream bs2(fake_data, 8);
    vector<bool> expected = vector<bool>(field_size, 0);
    bs2 >> expected;

    for (int i = 0; i < field_size; ++i)
    {
        TEST_ASSERT_EQUAL(field_bit_arr[i], expected[i]);
    }
}

// ----------------------------------------------------------------------------
// Specifications (functionality) Tests
// ----------------------------------------------------------------------------

void test_clear_mt_packet_len()
{
    TestFixture tf;
    tf.radio_mt_packet_len_fp->set(1);
    // If mt_packet_len is not 0, 
    TEST_ASSERT_TRUE(tf.radio_mt_packet_len_fp->get());
    tf.uplink_consumer->execute();

    // Then it should be 0 after execution
    TEST_ASSERT_FALSE(tf.radio_mt_packet_len_fp->get());
}

void test_perisist_mt_packet_len()
{
    TestFixture tf;
    // If mt_packet_len is 0
    TEST_ASSERT_FALSE(tf.radio_mt_packet_len_fp->get());
    tf.uplink_consumer->execute();
    
    // Then it should be 0 after execution
    TEST_ASSERT_FALSE(tf.radio_mt_packet_len_fp->get());
}

void test_check_ready()
{
    TestFixture tf;
    // If mt_packet_len is 0
    TEST_ASSERT_FALSE(tf.radio_mt_packet_len_fp->get());
    
    // Then registry should not be updated
}

void test_do_not_update_non_writable()
{
    TestFixture tf;
    // If mt_packet_len is not 0 but field specified by packet is not writable
    size_t idx = tf.field_map["adcs.ang_rate"];
    auto field = tf.registry.writable_fields[idx];

    char* not_writable_data = (char*)"\xa1";  
    tf.radio_mt_packet_fp->set(not_writable_data);

    auto old_val = field->get_bit_array().to_ullong();

    // Attempt update
    tf.uplink_consumer->update_field(idx);

    // Then that field should not be updated after execution
    auto new_val = field->get_bit_array().to_ullong();
    TEST_ASSERT_EQUAL(old_val, new_val);
}

void test_update_writable_field()
{
    TestFixture tf;
    // If mt_packet_len is not 0 && packet indicates an update to writable field

    // Given a valid field and value
    size_t idx = tf.field_map["adcs.mode"];
    auto field = tf.registry.writable_fields[idx];

    // Set fake packet and set ready
    // TODO: serialize the field_index somehow and add it to this packet
    char* fake_packet = (char*)"\xa1"; 
    tf.radio_mt_packet_fp->set(fake_packet);
    tf.radio_mt_packet_len_fp->set(1);

    // Call execute
    tf.uplink_consumer->execute();

    // Make expected result
    size_t field_size = tf.uplink_consumer->get_field_length(idx);
    BitStream bs2(tf.radio_mt_packet_fp->get(), 6); // TODO: packet size should be standard
    vector<bool> expected = vector<bool>(field_size, 0);
    bs2 >> expected;

    // Then that field should be updated to the expected value after execution
    auto res  = field->get_bit_array();
    for (int i = 0; i < field_size; ++i)
    {
      //  cout << " [test] new_bit_arr " << (uint16_t) res[i] << " expected " << (uint16_t) expected[i] << endl;
        TEST_ASSERT_EQUAL(res[i], expected[i]);
    }
}

void test_multiple_updates()
{
    TestFixture tf;
    // If mt_packet_len is not 0 && packet requests multiple updates
    char* multi_packet = (char*)"\xa1";  // TODO: 
    tf.radio_mt_packet_fp->set(multi_packet);
    tf.radio_mt_packet_len_fp->set(1);

    tf.uplink_consumer->execute();

    // Then all writable fields should be updated
}

// ----------------------------------------------------------------------------
// Invalid Requests
// ----------------------------------------------------------------------------

void test_mixed_validity_updates()
{
    TestFixture tf;
    // If mt_packet_len is not 0 && packet requests multiple updates && some updates
    // are on not writable fields
    char* mixed_packet = (char*)"\xa1";  // TODO: 
    tf.radio_mt_packet_fp->set(mixed_packet);
    tf.radio_mt_packet_len_fp->set(1);

    tf.uplink_consumer->execute();

    // Then do not update any field because packet is probably bad
}

void test_bad_request()
{
    TestFixture tf;
    // If mt_packet_len != 0 && packet is bad request
    char* bad_packet = (char*)"\xa1";  // TODO: 
    tf.radio_mt_packet_fp->set(bad_packet);
    tf.radio_mt_packet_len_fp->set(1);

    tf.uplink_consumer->execute();

    // Then hopefully detect the bad request and do not update
}


int test_uplink_consumer() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_get_field_length);
    RUN_TEST(test_update_field);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    test_bitstream();
    return test_uplink_consumer();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_bitstream();
    test_uplink_consumer();
}

void loop() {}
#endif

//cat test/test_uplink_consumer//test_uplink_consumer.cpp | grep "void test_" | sed 's/^void \(.*\)$/\1/' | sed 's/()/);/g'| sed -e 's/^/RUN_TEST(/'