#include "../StateFieldRegistryMock.hpp"

#define DEBUG
#include <fsw/GroundCode/UplinkProducer.h>

#include <unity.h>
#include <fstream>
#include <json.hpp>

#define TEST_ASSERT_THROW(x){try{x;TEST_ASSERT_TRUE(0);}catch(const std::exception& e){TEST_ASSERT_TRUE(1);}}
#define TEST_ASSERT_NO_THROW(x){try{x;TEST_ASSERT_TRUE(1);}catch(const std::exception& e){TEST_ASSERT_TRUE(0);}}
class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::unique_ptr<UplinkProducer> uplink_producer;
    
    // Create a TestFixture instance of UplinkProducer with the following parameters
    TestFixture() : registry() {
         uplink_producer = std::make_unique<UplinkProducer>(registry);
        // Create dummy fields
    }
    // Checks that each of the witable fields specified in json file are set to the
    // value specified in the json file
    void check_json_registry(const char* filename)
    {
      using json = nlohmann::json;
      json j;
      std::ifstream fs (filename);
      fs >> j;
      fs.close();

      for (auto& e : j.items())
      {
          // Check whether the requested field exists
          std::string key = e.key();

          // Get the field's index in writable_fields
          size_t field_index = uplink_producer->field_map[key];
          // std::cout << "Checking " << key << " at index " << field_index << std::endl;
          auto ef = registry.writable_fields[field_index]->get_bit_array().to_ulong();
          TEST_ASSERT_EQUAL(e.value(), ef);
      }
    }
};

void test_task_initialization() {
  TestFixture tf;
}

// Test that we can create files from json
void test_create_from_json() {
    TestFixture tf;
    size_t arr_size = tf.uplink_producer->get_max_possible_packet_size();
    char tmp [arr_size];
    bitstream bs(tmp, arr_size);
    TEST_ASSERT_NO_THROW(tf.uplink_producer->create_from_json(bs, "test/test_gsw_uplink_producer/test_1.json"));
    tf.uplink_producer->_update_fields(bs);
    tf.check_json_registry("test/test_gsw_uplink_producer/test_1.json");
}

// Test that we can write valid packets to files
void test_to_file()
{
    TestFixture tf;
    size_t arr_size = tf.uplink_producer->get_max_possible_packet_size();
    char tmp [arr_size];
    bitstream bs(tmp, arr_size);
    TEST_ASSERT_NO_THROW(tf.uplink_producer->create_from_json(bs, "test/test_gsw_uplink_producer/test_1.json"));
    tf.uplink_producer->_update_fields(bs);
    const std::string& filename = std::string("test/test_gsw_uplink_producer/test1.sbd");
    TEST_ASSERT_NO_THROW(tf.uplink_producer->to_file(bs, filename));
}

// Test that we get an runtime error when trying to save invalid packets
void test_to_file_invalid()
{
    TestFixture tf;
    size_t arr_size = tf.uplink_producer->get_max_possible_packet_size();
    char tmp [arr_size];
    bitstream bs(tmp, arr_size);
    const std::string& filename = std::string("test/test_gsw_uplink_producer/test_invalid.sbd");
    TEST_ASSERT_THROW( tf.uplink_producer->to_file(bs, filename));
}

// Test that creating SBD from json works
void test_create_sbd_from_json()
{
    TestFixture tf;
    const std::string& json_name = std::string("test/test_gsw_uplink_producer/test_2.json");
    const std::string& filename = std::string("test/test_gsw_uplink_producer/test2.sbd");
    TEST_ASSERT_TRUE(tf.uplink_producer->create_sbd_from_json(json_name, filename));
}

// Test that throwing exceptions when creating invalid stuff from json
void test_invalid_values()
{
    TestFixture tf;
    size_t arr_size = tf.uplink_producer->get_max_possible_packet_size();
    char tmp [arr_size];
    bitstream bs(tmp, arr_size);
    TEST_ASSERT_THROW(tf.uplink_producer->create_from_json(bs, "test/test_gsw_uplink_producer/test_3.json"));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_create_from_json);
    RUN_TEST(test_to_file);
    RUN_TEST(test_to_file_invalid);
    RUN_TEST(test_create_sbd_from_json);
    RUN_TEST(test_invalid_values);
    return UNITY_END();
}
