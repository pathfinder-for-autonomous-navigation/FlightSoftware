#include "../StateFieldRegistryMock.hpp"

#define DEBUG
#include <fsw/FCCode/UplinkConsumer.h>

#include <common/bitstream.h>
#include <unity.h>
#include <map>

using namespace std;

// convert n to bits
void from_ull(WritableStateFieldBase* w, uint64_t n)
{
    std::vector<bool> feels_bad (w->get_bit_array().size(), 0);
    for (size_t i = 0; i < w->get_bit_array().size(); ++i)
    {
        feels_bad[i] = n&1;
        n >>= 1;
    }
    w->set_bit_array(feels_bad);
    w->deserialize();
}

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::unique_ptr<UplinkConsumer> uplink_consumer;
    std::shared_ptr<InternalStateField<size_t>> radio_mt_packet_len_fp;
    std::shared_ptr<InternalStateField<char*>> radio_mt_packet_fp;

    std::shared_ptr<WritableStateField<unsigned int>> cycle_no_fp;
    std::shared_ptr<WritableStateField<unsigned char>> adcs_state_fp;
    std::shared_ptr<WritableStateField<f_quat_t>> adcs_cmd_attitude_fp;
    std::shared_ptr<ReadableStateField<float>> adcs_ang_rate_fp;
    std::shared_ptr<WritableStateField<float>> adcs_min_stable_ang_rate_fp;
    std::shared_ptr<WritableStateField<unsigned char>> mission_mode_fp;
    std::shared_ptr<WritableStateField<unsigned char>> sat_designation_fp;

    // Test Helper function will map field names to indices
    std::map<std::string, size_t> field_map;
   
   // Make external fields
    char mt_buffer[350];
    
    // Create a TestFixture instance of QuakeManager with the following parameters
    TestFixture() : registry() {
        srand(1);
        // Create dummy fields
        cycle_no_fp = registry.create_writable_field<unsigned int>("pan.cycle_no");
        adcs_state_fp = registry.create_writable_field<unsigned char>("adcs.state", 10);
        adcs_cmd_attitude_fp = registry.create_writable_field<f_quat_t>("adcs.cmd_attitude");
        adcs_ang_rate_fp = registry.create_readable_field<float>("adcs.ang_rate", 0, 10, 4);
        adcs_min_stable_ang_rate_fp = registry.create_writable_field<float>("adcs.min_stable_ang_rate", 0, 10, 4);
        mission_mode_fp = registry.create_writable_field<unsigned char>("pan.state");
        sat_designation_fp = registry.create_writable_field<unsigned char>("pan.sat_designation"); // should be 6 writable fields --> 3 bits 

        radio_mt_packet_len_fp = registry.create_internal_field<size_t>("uplink.len");
        radio_mt_packet_fp = registry.create_internal_field<char*>("uplink.ptr");

        // Initialize internal fields
        uplink_consumer = std::make_unique<UplinkConsumer>(registry, 0);

        radio_mt_packet_fp->set(mt_buffer);
        field_map = std::map<std::string, size_t>();

        // Setup field_map and assign some values
        for (size_t i = 0; i < registry.writable_fields.size(); ++i)
        {
            auto w = registry.writable_fields[i];
            field_map[w->name().c_str()] = i;
            w->name();
            from_ull(w, rand()); // no seed so should be the same each time
        }
        uplink_consumer->init_uplink();
    }
    /**
     * @param out The uplink packet as a bitstream
     * @param in The bitstream representation of the single value to be set for the field
     * given by index
     * @param index The index of the writable field that we want to update
     */
    size_t create_uplink( bitstream& out, bitstream& in, size_t index)
    {
        size_t bits_written = 0;
        auto bit_arr = registry.writable_fields[index]->get_bit_array();
        ++index; // indices are offset by 1
        bits_written += out.editN(uplink_consumer->index_size, (uint8_t*)&index);
        bits_written += out.editN(bit_arr.size(), in);
        return bits_written;
    }

    /**
     * @param out The uplink packet as a bitstream
     * @param val Array of chars containing the data to be assigned to the field
     * @param val_size The number of bits of val to assign to the field
     * @param index The index of the writable field that we want to update
     * @param return 0 if val_size > field size, else the number of bits written
     */
    size_t create_uplink( bitstream& out, char* val, size_t index)
    {
        size_t bits_written = 0;
        size_t field_size = uplink_consumer->get_field_length(index);
        // Write the index
        ++index; // indices are offset by 1
        bits_written += out.editN(uplink_consumer->index_size, (uint8_t*)&index);
        // Write the specified number of bits from val
        bits_written += out.editN(field_size, reinterpret_cast<uint8_t*>(val));
        return bits_written;
    }
};

void test_create_uplink()
{
    TestFixture tf;
    // Create the data for the fields
    char data[4];
    memcpy(data, "\xfa\xaf\x34\xab", 4);
    bitstream in(data, 4);

    // Create the output packet bitstream
    char backer[8];
    memset(backer, 0, 8); 
    bitstream out(backer, 8);

    // Create the expected result
    size_t idx = tf.field_map["adcs.state"];
    size_t packet_size = tf.uplink_consumer->index_size + tf.uplink_consumer->get_field_length(idx);

    std::vector<bool> expect(packet_size, 0);
    expect[0] = 0;
    expect[1] = 1;
    expect[2] = 0;
    expect[3] = 0;
    expect[4] = 1;
    expect[5] = 0;
    expect[6] = 1;
    // Create an entry in output packet bitstream to update adcs.state
    size_t bits_written = tf.create_uplink(out, data, idx);
    TEST_ASSERT_EQUAL(packet_size, bits_written);
    out.reset();

    // Retrieve data from the packet to see if it's there
    std::vector<bool> actual(bits_written, 0);
    out >> actual;

    for (size_t i = 0; i < bits_written; ++i)
    {
        //cout << expect[i] <<  " " << actual[i] << endl;
        TEST_ASSERT_EQUAL(expect[i], actual[i]);
    }
}

void test_create_uplink_other()
{
    TestFixture tf;
    // Create the data for the fields
    char data[4];
    memcpy(data, "\xfa\xaf\x34\xab", 4);
    bitstream in(data, 4);

    // Create the output packet bitstream
    char backer[8];
    memset(backer, 0, 8); 
    bitstream out(backer, 8);

    // Create the expected result
    size_t idx = tf.field_map["adcs.state"];
    size_t packet_size = tf.uplink_consumer->index_size + tf.uplink_consumer->get_field_length(idx);

    std::vector<bool> expect(packet_size, 0);
    expect[0] = 0;
    expect[1] = 1;
    expect[2] = 0;
    expect[3] = 0;
    expect[4] = 1;
    expect[5] = 0;
    expect[6] = 1;
    // Create an entry in output packet bitstream to update adcs.state
    size_t bits_written = tf.create_uplink(out, in, idx);
    TEST_ASSERT_EQUAL(packet_size, bits_written);
    out.reset();

    // Retrieve data from the packet to see if it's there
    std::vector<bool> actual(bits_written, 0);
    out >> actual;

    for (size_t i = 0; i < bits_written; ++i)
    {
        //cout << expect[i] <<  " " << actual[i] << endl;
        TEST_ASSERT_EQUAL(expect[i], actual[i]);
    }
}

void test_valid_initialization() 
{
    TestFixture tf;
    TEST_ASSERT_EQUAL(0, tf.radio_mt_packet_len_fp->get());
    TEST_ASSERT_NOT_NULL(tf.radio_mt_packet_fp->get());
    TEST_ASSERT_EQUAL(3, tf.uplink_consumer->index_size);
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
    size_t idx = tf.field_map["adcs.state"];
    // Then return the length of the bit array
    TEST_ASSERT_EQUAL(4, tf.uplink_consumer->get_field_length(idx));
}

void test_update_field()
{
    TestFixture tf;

    // field size is 8, initial value is 0x88
    // 0001 0001
    // idx = 4
    size_t idx = tf.field_map["pan.state"]; 

    // idx = 3, initial value is 0x6, field len is 4
    // 0101
    size_t idx2 = tf.field_map["adcs.min_stable_ang_rate"];
    
    // 4    3    6    5
    // 0010 1100 0110 1010
    // cout << "idx " << idx << " field len " << field_bit_arr.size() << " current val " << field->get_bit_array().to_uint() << endl;
    char* fake_data = (char*)"\x34\x56\xef\xbe\xaa\xbb\xcc\xdd"; 
    
    size_t bits_written = 0;
    
    size_t packet_size = 8 + 3 + 4 + 3;
    size_t packet_bytes = (packet_size + 7)/8;

    char tmp[packet_bytes];
    memset(tmp, 0, packet_bytes);
    bitstream bs_mo(tmp, packet_bytes);
    bitstream bs_in(fake_data, 8);
    
    // Create uplink for pan.state
    bits_written += tf.create_uplink(bs_mo, bs_in, idx); 

    // Check that bitstream is at the right bit offset
    TEST_ASSERT_EQUAL((8 + 3)%8, bs_mo.bit_offset);

    // Seek for fun
    bs_in.seekG(1, bs_end);
    // Create uplink for adcs.min_stable_ang_rate
    bits_written += tf.create_uplink(bs_mo, bs_in, idx2); // 110 1 --> 0xb
    TEST_ASSERT_EQUAL(packet_size, bits_written);
   
    // Copy packet into the radio-mt_packet_fp->get()
    memcpy(tf.radio_mt_packet_fp->get(), tmp, packet_bytes);
    tf.radio_mt_packet_len_fp->set(packet_bytes);
    tf.uplink_consumer->update_fields();

    // Check results
    unsigned long field1 = tf.registry.writable_fields[idx]->get_bit_array().to_uint();
    unsigned long field2 = tf.registry.writable_fields[idx2]->get_bit_array().to_uint();
    // Make expected result
    TEST_ASSERT_EQUAL(0x34, field1);
    TEST_ASSERT_EQUAL(0xb, field2);
}

// ----------------------------------------------------------------------------
// Specifications (functionality) Tests
// ----------------------------------------------------------------------------

void test_clear_mt_packet_len()
{
    TestFixture tf;
    tf.radio_mt_packet_len_fp->set(3);
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

void test_update_writable_field()
{
    TestFixture tf;
    // idx 0 field len 32 current val 585640194
    size_t idx = tf.field_map["cmd_attitude"]; 
    auto field = tf.registry.writable_fields[idx];

    // idx2 5 field len 8 current val 184
    size_t idx2 = tf.field_map["pan.sat_designation"];
    auto field2 = tf.registry.writable_fields[idx2];

    // idx3 1 field len 4 current val 5
    size_t idx3 = tf.field_map["adcs.state"];
    auto field3 = tf.registry.writable_fields[idx3];

    size_t packet_size = field->get_bit_array().size() + field2->get_bit_array().size() + field3->get_bit_array().size() + 3*3;
    size_t packet_bytes =(packet_size + 7)/8;
    
    // Create output bitstream
    char backer[packet_bytes];
    memset(backer, 0, packet_bytes);
    bitstream out(backer, packet_bytes);

    uint32_t new_field = (uint32_t)0xcafefade;
    tf.create_uplink(out, reinterpret_cast<char*>(&new_field), idx);

    uint8_t new_field_2 = (uint8_t)0xf117;
    tf.create_uplink(out, reinterpret_cast<char*>(&new_field_2), idx2);

    uint8_t new_field_3 = (uint8_t)0xf15;
    tf.create_uplink(out, reinterpret_cast<char*>(&new_field_3), idx3);

    // Pretend to set shared pointers
    memcpy(tf.radio_mt_packet_fp->get(), backer, packet_bytes);
    tf.radio_mt_packet_len_fp->set(packet_bytes);

    // Execute
    tf.uplink_consumer->execute();

    // Check values
    TEST_ASSERT_EQUAL(new_field, field->get_bit_array().to_ullong());
    TEST_ASSERT_EQUAL(new_field_2, field2->get_bit_array().to_ulong());
    TEST_ASSERT_EQUAL(0x5, field3->get_bit_array().to_uint());
}

// ----------------------------------------------------------------------------
// Invalid Requests
// ----------------------------------------------------------------------------

void test_mixed_validity_updates()
{
    TestFixture tf;
    // If mt_packet_len is not 0 && packet requests multiple updates && some updates
    // are on not writable fields

    size_t idx = tf.field_map["cmd_attitude"]; 
    auto field = tf.registry.writable_fields[idx];
    uint64_t old1 = field->get_bit_array().to_ullong();

    // create invalid field
    size_t idx2 = tf.field_map["adcs.ang_rate"]; 
    auto field2 = tf.registry.writable_fields[idx2];
    uint64_t old2 = field2->get_bit_array().to_ullong();

    size_t idx3 = tf.field_map["adcs.state"];
    auto field3 = tf.registry.writable_fields[idx3];
    uint64_t old3 = field3->get_bit_array().to_ullong();

    size_t packet_size = field->get_bit_array().size() + field2->get_bit_array().size() + field3->get_bit_array().size() + 3*3;

    size_t packet_bytes =(packet_size + 7)/8;
    
    // Create output bitstream
    char backer[packet_bytes];
    memset(backer, 0, packet_bytes);
    bitstream out(backer, packet_bytes);

    uint32_t new_field = (uint32_t)0xcafefade;
    tf.create_uplink(out, reinterpret_cast<char*>(&new_field), idx);

    uint8_t new_field_2 = (uint8_t)0xf117;
    tf.create_uplink(out, reinterpret_cast<char*>(&new_field_2), idx2);
   
    // Rig the game and add corrupt the packet here
    out.seekG(2, bs_end);

    uint8_t new_field_3 = (uint8_t)0xf15;
    tf.create_uplink(out, reinterpret_cast<char*>(&new_field_3), idx3);

    // Pretend to set shared pointers
    memcpy(tf.radio_mt_packet_fp->get(), backer, packet_bytes);
    tf.radio_mt_packet_len_fp->set(packet_bytes);

    // Execute
    tf.uplink_consumer->execute();

    // Check that values are not updated values
    TEST_ASSERT_EQUAL(old1, field->get_bit_array().to_ullong());
    TEST_ASSERT_EQUAL(old2, field2->get_bit_array().to_ullong());
    TEST_ASSERT_EQUAL(old3, field3->get_bit_array().to_ullong());
    TEST_ASSERT_EQUAL(0,  tf.radio_mt_packet_len_fp->get());
}



int test_uplink_consumer() {
    UNITY_BEGIN();
    RUN_TEST(test_create_uplink);
    RUN_TEST(test_create_uplink_other);
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_get_field_length);
    RUN_TEST(test_update_field);
    RUN_TEST(test_clear_mt_packet_len);
    RUN_TEST(test_perisist_mt_packet_len);
    RUN_TEST(test_update_writable_field);
    RUN_TEST(test_mixed_validity_updates);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    //test_bitstream();
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

//cat test/test_uplink_consumer//test_uplink_consumer.cpp | grep "void test_" | sed 's/^void \(.*\)$/\1/' | sed 's/()/);/g'| sed -e 's/^/RUN_TEST(/'