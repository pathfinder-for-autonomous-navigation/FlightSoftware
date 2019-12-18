#include "../StateFieldRegistryMock.hpp"
#define DEBUG
#include "../../src/GroundCode/UplinkProducer.h"
#include <unity.h>
#include <fstream>
#include <json.hpp>

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::unique_ptr<UplinkProducer> uplink_producer;
    
    // Create a TestFixture instance of QuakeManager with the following parameters
    TestFixture() : registry() {
         uplink_producer = std::make_unique<UplinkProducer>(registry);
        // Create dummy fields
    }
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


void test_create_from_json() {
    TestFixture tf;
    size_t arr_size = tf.uplink_producer->get_max_possible_packet_size();
    char tmp [arr_size];
    bitstream bs(tmp, arr_size);
    tf.uplink_producer->create_from_json(bs, "test/test_gsw_uplink_producer/test_1.json");
    tf.uplink_producer->to_string(bs);
    tf.uplink_producer->_update_fields(bs);
    tf.check_json_registry("test/test_gsw_uplink_producer/test_1.json");
}

void test_to_file()
{
    TestFixture tf;
    size_t arr_size = tf.uplink_producer->get_max_possible_packet_size();
    char tmp [arr_size];
    bitstream bs(tmp, arr_size);
    tf.uplink_producer->create_from_json(bs, "test/test_gsw_uplink_producer/test_1.json");
    tf.uplink_producer->_update_fields(bs);
    const std::string& filename = std::string("test/test_gsw_uplink_producer/test1.sbd");
    tf.uplink_producer->to_file(bs, filename);
}

void test_task_initialization() {
  TestFixture tf;
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_create_from_json);
    RUN_TEST(test_to_file);
    return UNITY_END();
}
