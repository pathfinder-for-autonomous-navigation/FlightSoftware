#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/DownlinkProducer.hpp>

#include <common/StateFieldRegistry.hpp>
#include <unity.h>

struct TestFixture {
    StateFieldRegistryMock registry;
    std::unique_ptr<DownlinkProducer> downlink_producer;

    std::shared_ptr<ReadableStateField<unsigned int>> foo1_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> cycle_count_fp;
    InternalStateField<char*>* snapshot_ptr_fp;
    InternalStateField<size_t>* snapshot_size_bytes_fp;

    TestFixture() : registry() {}

    void init(const std::vector<DownlinkProducer::FlowData>& flow_data) {
        // Create required field(s)
        cycle_count_fp = registry.create_readable_field<unsigned int>("pan.cycle_no");

        // Create field(s) for serialization and initialize them to
        // default values
        foo1_fp = registry.create_readable_field<unsigned int>("foo1");
        cycle_count_fp->set(20);
        foo1_fp->set(400);

        downlink_producer = std::make_unique<DownlinkProducer>(registry, 0);
        downlink_producer->init_flows(flow_data);
        snapshot_ptr_fp = registry.find_internal_field_t<char*>("downlink.ptr");
        snapshot_size_bytes_fp = registry.find_internal_field_t<size_t>(
                                    "downlink.snap_size");
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

        // ceil((1 + 32 + 0) / 8)
        TEST_ASSERT_EQUAL(5, tf.snapshot_size_bytes_fp->get());
    }

    {
        TestFixture tf;

        // Empty flow data
        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                1,
                true,
                {}
            } // Flow size: 1 bit
        };

        tf.init(flow_data);
        TEST_ASSERT_NOT_NULL(tf.snapshot_ptr_fp);
        TEST_ASSERT_NOT_NULL(tf.snapshot_size_bytes_fp);

        // ceil((1 + 32 + 1) / 8)
        TEST_ASSERT_EQUAL(5, tf.snapshot_size_bytes_fp->get());
        tf.cycle_count_fp->set(20);
        tf.downlink_producer->execute();
        const char expected_output[5] = {'\x94', '\x00', '\x00', '\x00', '\x40'};
        TEST_ASSERT_EQUAL_MEMORY(expected_output, tf.snapshot_ptr_fp->get(), 5);
    }
}

/**
 * @brief Test with one small flow and a large flow.
 */
void test_one_flow() {
    {
        TestFixture tf;

        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                1,
                true,
                {
                    "foo1", // 32 bits
                } // Flow size: 32 bits
            }
        };
        tf.init(flow_data);
        // ceil((1 + 32 + (32))/8)
        TEST_ASSERT_EQUAL(9, tf.snapshot_size_bytes_fp->get());

        tf.downlink_producer->execute();
        const char expected_outputs[9] = {'\x94', '\x00', '\x00', '\x00', '\x42', '\x60', '\x00', '\x00', '\x00'};
        TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 9);
    }

    {
        TestFixture tf;
        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                1,
                true,
                {
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                } // Flow size: 160 bits
            }
        };
        tf.init(flow_data);

        // ceil((1 + 32 + (160) / 8)
        TEST_ASSERT_EQUAL(25, tf.snapshot_size_bytes_fp->get());

        tf.downlink_producer->execute();
        const char expected_outputs[25] = {'\x94', '\x00', '\x00', '\x00', '\x42', '\x60', '\x00',
            '\x00', '\x02', '\x60', '\x00', '\x00', '\x02', '\x60', '\x00', '\x00', '\x02', '\x60',
            '\x00', '\x00', '\x02', '\x60', '\x00', '\x00', '\x00'};
        TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 25);
    }
}

/**
 * @brief Test with a flow containing data of many different kinds, and ensure that
 * the size and data of the resulting downlink is correct.
 */
void test_one_flow_multityped() {
    TestFixture tf;

    // foo1_fp was already created in TestFixture
    auto foo2_fp = tf.registry.create_readable_field<gps_time_t>("foo2");
    auto foo3_fp = tf.registry.create_readable_field<f_quat_t>("foo3");
    auto foo4_fp = tf.registry.create_writable_vector_field<float>("foo4", 0, 3, 25);
    auto foo5_fp = tf.registry.create_readable_field<signed char>("foo5", 0, 2, 3);
    auto foo6_fp = tf.registry.create_readable_field<unsigned char>("foo6", 25);
    auto foo7_fp = tf.registry.create_readable_field<bool>("foo7");
    auto foo8_fp = tf.registry.create_readable_field<signed int>("foo8", -2, 8);
    std::vector<DownlinkProducer::FlowData> flow_data = {
        {
            1,
            true,
            {
                "foo1", // 32 bits
                "foo2", // 68 bits
                "foo3", // 29 bits
                "foo4", // 25 bits
                "foo5", // 3 bits
                "foo6", // 5 bits
                "foo7", // 1 bits
                "foo8", // 4 bit
            } // Flow size: 168 bits (1 + 167)
        }
    };
    tf.init(flow_data);

    // ceil((1 + 32 + (168)) / 8)
    TEST_ASSERT_EQUAL(26, tf.snapshot_size_bytes_fp->get());

    // TODO add testing of actual downlink data once the quaternion and
    // vector serializers work properly.
}

/**
 * @brief Test with multiple flows.
 */
void test_multiple_flows() {
    // Test with multiple flows that fit within a downlink packet.
    {
        TestFixture tf;
        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                1,
                true,
                {
                    "foo1", // 32 bits
                } // Flow size 33 bits (32 + 1)
            },
            {
                2,
                true,
                {
                    "foo1", // 32 bits
                } // Flow size: 33 bits (32 + 1)
            }
        };
        tf.init(flow_data);

        // ceil((1 + 32 + (33 + 33)) / 8)
        TEST_ASSERT_EQUAL(13, tf.snapshot_size_bytes_fp->get());

        tf.downlink_producer->execute();
        const char expected_outputs[13] = {'\x94', '\x00', '\x00', '\x00', '\x41', '\x30', '\x00',
            '\x00', '\x08', '\x4c', '\x00', '\x00', '\x00'};
        TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 13);        
    }

    // Test with multiple flows that require more than one downlink packet.
    {
        TestFixture tf;

        std::vector<DownlinkProducer::FlowData> flow_data = {
            {
                1,
                true,
                {
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                } // Flow size 98 bits (96 + 2)
            },
            {
                2,
                true,
                {
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                } // Flow size: 226 bits (224 + 2)
            },
            {
                3,
                true,
                {
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                    "foo1", // 32 bits
                } // Flow size: 226 bits (224 + 2)
            }
        };
        tf.init(flow_data);

        // ceil((1 + 32 + (98 + 226 + 226) + 1) / 8)
        TEST_ASSERT_EQUAL(73, tf.snapshot_size_bytes_fp->get());
        tf.downlink_producer->execute();
        const char expected_outputs[73] = {'\x94', '\x00', '\x00', '\x00', '\x41', '\x30', '\x00',
            '\x00', '\x01', '\x30', '\x00', '\x00', '\x01', '\x30', '\x00', '\x00', '\x08', '\x4c',
            '\x00', '\x00', '\x00', '\x4c', '\x00', '\x00', '\x00', '\x4c', '\x00', '\x00', '\x00',
            '\x4c', '\x00', '\x00', '\x00', '\x4c', '\x00', '\x00', '\x00', '\x4c', '\x00', '\x00',
            '\x00', '\x4c', '\x00', '\x00', '\x06', '\x13', '\x00', '\x00', '\x00', '\x13', '\x00',
            '\x00', '\x00', '\x13', '\x00', '\x00', '\x00', '\x13', '\x00', '\x00', '\x00', '\x13',
            '\x00', '\x00', '\x00', '\x13', '\x00', '\x00', '\x00', '\x13', '\x00', '\x00', '\x00'};
        TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 73);
    }
}

/**
 * @brief Verify that setting some flows inactive does not change the initial snapshot size,
 * but does change the snapshot size after one execution. Test that inactive flows don't
 * appear in the downlink snapshot.
 */
void test_some_flows_inactive() {
    TestFixture tf;

    std::vector<DownlinkProducer::FlowData> flow_data = {
        {
            1,
            true,
            {
                "foo1", // 59 bits
            } // Flow size 34 bits (32 + 2)
        },
        {
            2,
            true,
            {
                "foo1", // 32 bits
            } // Flow size: 34 bits (32 + 2)
        },
        {
            3,
            false,
            {
                "foo1", // 32 bits
            } // Flow size: 34 bits (32 + 2)
        }
    };
    tf.init(flow_data);

    // ceil((1 + 32 + (34 + 34 + 34) ) / 8 )
    TEST_ASSERT_EQUAL(17, tf.snapshot_size_bytes_fp->get());
    tf.downlink_producer->execute();
    // ceil((1 + 32 + (34 + 34)) / 8 )
    TEST_ASSERT_EQUAL(13, tf.snapshot_size_bytes_fp->get());
    tf.downlink_producer->execute();
    const char expected_outputs[13] = {'\x94', '\x00', '\x00', '\x00', '\x41', '\x30', '\x00',
        '\x00', '\x08', '\x4c', '\x00', '\x00', '\x00'};
    TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 13);
}

/**
 * @brief If the state field data changes, the downlink snapshot also
 * changes on the next control cycle.
 */
void test_downlink_changes() {
    TestFixture tf;

    std::vector<DownlinkProducer::FlowData> flow_data = {
        {
            1,
            true,
            {
                "foo1", // 32 bits
            } // Flow size: 33 bits
        }
    };
    tf.init(flow_data);
    tf.downlink_producer->execute();
    char expected_outputs[9] = {'\x94', '\x00', '\x00', '\x00', '\x42', '\x60', '\x00', '\x00',
        '\x00'};
    TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 9);

    tf.foo1_fp->set(800);
    TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 9); // No change if execution didn't occur
    tf.downlink_producer->execute();
    expected_outputs[4] = '\x41';
    expected_outputs[5] = '\x30';
    TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 9); // Downlink data changed
}

int test_downlink_producer_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_one_flow);
    RUN_TEST(test_one_flow_multityped);
    RUN_TEST(test_multiple_flows);
    RUN_TEST(test_some_flows_inactive);
    RUN_TEST(test_downlink_changes);
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
