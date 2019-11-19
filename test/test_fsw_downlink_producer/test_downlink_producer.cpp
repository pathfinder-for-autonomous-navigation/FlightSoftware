#include <StateFieldRegistry.hpp>
#include "../StateFieldRegistryMock.hpp"
#include "../../src/FCCode/DownlinkProducer.hpp"
#include <unity.h>
#include <iostream>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    std::unique_ptr<DownlinkProducer> downlink_producer;

    InternalStateField<char*>* snapshot_ptr_fp;
    InternalStateField<size_t>* snapshot_size_bytes_fp;

    TestFixture() : registry() {}

    void init(const std::vector<DownlinkProducer::FlowData>& flow_data) {
        registry.create_readable_field<unsigned int>("pan.cycle_no");
        downlink_producer = std::make_unique<DownlinkProducer>(registry, 0, flow_data);
        snapshot_ptr_fp = registry.find_internal_field_t<char*>("downlink_producer.mo_ptr");
        snapshot_size_bytes_fp = registry.find_internal_field_t<size_t>(
                                    "downlink_producer.snap_size");
    }
};

/**
 * @brief Test initialization with no flows and with one empty flow.
 */
void test_task_initialization() {
    {
        TestFixture tf;

        // Empty flow data
        std::vector<DownlinkProducer::FlowData> flow_data = {};

        tf.init(flow_data);
        TEST_ASSERT_NOT_NULL(tf.snapshot_ptr_fp);
        TEST_ASSERT_NOT_NULL(tf.snapshot_size_bytes_fp);
        TEST_ASSERT_EQUAL(5, tf.snapshot_size_bytes_fp->get());
    }

    {
        TestFixture tf;

        // Empty flow data
        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                0,
                true,
                {}
            } // Flow size: 1 bit
        };

        tf.init(flow_data);
        TEST_ASSERT_NOT_NULL(tf.snapshot_ptr_fp);
        TEST_ASSERT_NOT_NULL(tf.snapshot_size_bytes_fp);

        // ceil((1 + 32 + 1) / 8)
        TEST_ASSERT_EQUAL(5, tf.snapshot_size_bytes_fp->get());
    }
}

/**
 * @brief Test initialization with one small flow and a large flow that forces a multi-downlink
 * packet.
 */
void test_initialization_one_flow() {
    {
        TestFixture tf;

        tf.registry.create_readable_field<gps_time_t>("foo1");
        tf.registry.create_readable_field<gps_time_t>("foo2");
        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                0,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size: 119 bits (118 + 1)
            }
        };
        tf.init(flow_data);
        TEST_ASSERT_EQUAL(19, tf.snapshot_size_bytes_fp->get());
    }

    {
        TestFixture tf;

        tf.registry.create_writable_field<gps_time_t>("foo1");
        tf.registry.create_writable_field<gps_time_t>("foo2");
        tf.registry.create_readable_field<gps_time_t>("foo3");
        tf.registry.create_readable_field<gps_time_t>("foo4");
        tf.registry.create_readable_field<gps_time_t>("foo5");
        tf.registry.create_readable_field<gps_time_t>("foo6");
        tf.registry.create_readable_field<gps_time_t>("foo7");
        tf.registry.create_readable_field<gps_time_t>("foo8");
        tf.registry.create_readable_field<gps_time_t>("foo9");
        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                0,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                    "foo3", // 59 bits
                    "foo4", // 59 bits
                    "foo5", // 59 bits
                    "foo6", // 59 bits
                    "foo7", // 59 bits
                    "foo8", // 59 bits
                    "foo9", // 59 bits
                } // Flow size: 532 bits (531 + 1)
            }
        };
        tf.init(flow_data);

        // ceil((1 + 32 + (532) + 1) / 8)
        TEST_ASSERT_EQUAL(71, tf.snapshot_size_bytes_fp->get());
    }
}

/**
 * @brief Test initialization with a flow containing data of many different kinds, and ensure that
 * the size of the resulting downlink is correct.
 */
void test_initialization_one_flow_multityped() {
    TestFixture tf;

    tf.registry.create_writable_field<gps_time_t>("foo1");
    tf.registry.create_readable_field<gps_time_t>("foo2");

    tf.registry.create_readable_field<unsigned int>("foo3");
    tf.registry.create_readable_field<f_quat_t>("foo4");
    tf.registry.create_writable_vector_field<float>("foo5", 0, 3, 25);
    tf.registry.create_readable_field<signed char>("foo6", 0, 2, 3);
    tf.registry.create_readable_field<unsigned char>("foo7", 25);
    tf.registry.create_readable_field<bool>("foo8");
    tf.registry.create_readable_field<signed int>("foo9", -2, 8);
    std::vector<DownlinkProducer::FlowData> flow_data = {
        {
            0,
            true,
            {
                "foo1", // 59 bits
                "foo2", // 59 bits
                "foo3", // 32 bits
                "foo4", // 29 bits
                "foo5", // 25 bits
                "foo6", // 3 bits
                "foo7", // 5 bits
                "foo8", // 1 bit
                "foo9", // 4 bits
            } // Flow size: 218 bits (217 + 1)
        }
    };
    tf.init(flow_data);

    // ceil((1 + 32 + (218)) / 8)
    TEST_ASSERT_EQUAL(32, tf.snapshot_size_bytes_fp->get());
}

/**
 * @brief Test initialization with multiple flows. Try a set of flows that doesn't span a downlink
 * packet, and then try one that does span multiple downlink packets.
 */
void test_initialization_multiple_flows() {
    {
        TestFixture tf;

        tf.registry.create_readable_field<gps_time_t>("foo1");
        tf.registry.create_readable_field<gps_time_t>("foo2");
        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                0,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size 119 bits (118 + 1)
            },
            {
                1,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size: 119 bits (118 + 1)
            }
        };
        tf.init(flow_data);

        // ceil((1 + 32 + (119 + 119)) / 8)
        TEST_ASSERT_EQUAL(34, tf.snapshot_size_bytes_fp->get());
    }

    {
        TestFixture tf;

        tf.registry.create_readable_field<gps_time_t>("foo1");
        tf.registry.create_readable_field<gps_time_t>("foo2");

        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                0,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size 121 bits (118 + 3)
            },
            {
                1,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size: 121 bits (118 + 3)
            },
            {
                2,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size: 121 bits (118 + 3)
            },
            {
                3,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size: 121 bits (118 + 3)
            },
            {
                4,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size: 121 bits (118 + 3)
            }
        };
        tf.init(flow_data);

        // ceil((1 + 32 + (121 + 121 + 121 + 121 + 121) + 1) / 8)
        TEST_ASSERT_EQUAL(80, tf.snapshot_size_bytes_fp->get());
    }
}

/**
 * @brief Verify that setting some flows inactive can change the snapshot
 * size.
 */
void test_initialization_some_inactive() {
    {
        TestFixture tf;

        tf.registry.create_readable_field<gps_time_t>("foo1");
        tf.registry.create_readable_field<gps_time_t>("foo2");

        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                0,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size 121 bits (118 + 3)
            },
            {
                1,
                true,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size: 121 bits (118 + 3)
            },
            {
                2,
                false,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size: 121 bits (118 + 3)
            },
            {
                3,
                false,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size: 121 bits (118 + 3)
            },
            {
                4,
                false,
                {
                    "foo1", // 59 bits
                    "foo2", // 59 bits
                } // Flow size: 121 bits (118 + 3)
            }
        };
        tf.init(flow_data);

        // ceil((1 + 32 + (121 + 121) + 1) / 8)
        TEST_ASSERT_EQUAL(35, tf.snapshot_size_bytes_fp->get());
    }
}


int test_downlink_producer_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_initialization_one_flow);
    RUN_TEST(test_initialization_one_flow_multityped);
    RUN_TEST(test_initialization_multiple_flows);
    RUN_TEST(test_initialization_some_inactive);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_downlink_producer_task();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_downlink_producer_task();
}

void loop() {}
#endif
