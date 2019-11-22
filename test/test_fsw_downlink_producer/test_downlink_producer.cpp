#include <StateFieldRegistry.hpp>
#include "../StateFieldRegistryMock.hpp"
#include "../../src/FCCode/DownlinkProducer.hpp"
#include <unity.h>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++11-narrowing"

struct TestFixture {
    StateFieldRegistryMock registry;
    std::unique_ptr<DownlinkProducer> downlink_producer;

    std::shared_ptr<ReadableStateField<unsigned int>> cycle_no_fp;
    InternalStateField<char*>* snapshot_ptr_fp;
    InternalStateField<size_t>* snapshot_size_bytes_fp;

    TestFixture() : registry() {}

    void init(const std::vector<DownlinkProducer::FlowData>& flow_data) {
        cycle_no_fp = registry.create_readable_field<unsigned int>("pan.cycle_no");
        downlink_producer = std::make_unique<DownlinkProducer>(registry, 0, flow_data);
        snapshot_ptr_fp = registry.find_internal_field_t<char*>("downlink_producer.mo_ptr");
        snapshot_size_bytes_fp = registry.find_internal_field_t<size_t>(
                                    "downlink_producer.snap_size");
    }
};

void print_hex(const char *s, const size_t len)
{
  for(size_t i = 0; i < len; i++)
    printf("0x%02x ", (unsigned int) *s++);
  printf("\n");
}

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
 * @brief Test with one small flow and a large flow.
 */
void test_one_flow() {
    {
        TestFixture tf;

        auto foo1_fp = tf.registry.create_readable_field<gps_time_t>("foo1");
        auto foo2_fp = tf.registry.create_readable_field<gps_time_t>("foo2");
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

        foo1_fp->set(gps_time_t(1000, 200, 0));
        foo2_fp->set(gps_time_t(1000, 500, 0));
        tf.cycle_no_fp->set(20);
        tf.downlink_producer->execute();
        // const char expected_outputs[19] = {0x80, 0x00, 0x00, 0x00, 0x45, 0xF0, 0x04, 0xC0, 0x00,
        //     0x00, 0x00, 0x08, 0xBE, 0x01, 0x7C, 0x00, 0x00, 0x00, 0x01};
        print_hex(tf.snapshot_ptr_fp->get(), 19);
        //TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 19);
    }

    {
        TestFixture tf;

        auto foo1_fp = tf.registry.create_writable_field<gps_time_t>("foo1");
        auto foo2_fp = tf.registry.create_writable_field<gps_time_t>("foo2");
        auto foo3_fp = tf.registry.create_readable_field<gps_time_t>("foo3");
        auto foo4_fp = tf.registry.create_readable_field<gps_time_t>("foo4");
        auto foo5_fp = tf.registry.create_readable_field<gps_time_t>("foo5");
        auto foo6_fp = tf.registry.create_readable_field<gps_time_t>("foo6");
        auto foo7_fp = tf.registry.create_readable_field<gps_time_t>("foo7");
        auto foo8_fp = tf.registry.create_readable_field<gps_time_t>("foo8");
        auto foo9_fp = tf.registry.create_readable_field<gps_time_t>("foo9");
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

        // ceil((1 + 32 + (532)) / 8)
        TEST_ASSERT_EQUAL(71, tf.snapshot_size_bytes_fp->get());

        foo1_fp->set(gps_time_t(1000, 200, 0));
        foo2_fp->set(gps_time_t(1000, 500, 0));
        foo3_fp->set(gps_time_t(1000, 800, 0));
        foo4_fp->set(gps_time_t(1000, 1100, 0));
        foo5_fp->set(gps_time_t(1000, 1400, 0));
        foo6_fp->set(gps_time_t(1000, 1700, 0));
        foo7_fp->set(gps_time_t(1000, 2000, 0));
        foo8_fp->set(gps_time_t(1000, 2500, 0));
        foo9_fp->set(gps_time_t(1000, 3000, 0));
        tf.cycle_no_fp->set(20);
        tf.downlink_producer->execute();
        // const char expected_outputs[71] = {0x80, 0x00, 0x00, 0x00, 0x45, 0xF0, 0x04, 0xC0, 0x00,
        //     0x00, 0x00, 0x08, 0xBE, 0x01, 0x7C, 0x00, 0x00, 0x00, 0x01, 0x17, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        print_hex(tf.snapshot_ptr_fp->get(), 71);
        // TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 71);
    }
}

/**
 * @brief Test with a flow containing data of many different kinds, and ensure that
 * the size and data of the resulting downlink is correct.
 */
void test_one_flow_multityped() {
    TestFixture tf;

    auto foo1_fp = tf.registry.create_writable_field<gps_time_t>("foo1");
    auto foo2_fp = tf.registry.create_readable_field<gps_time_t>("foo2");
    auto foo3_fp = tf.registry.create_readable_field<unsigned int>("foo3");
    auto foo4_fp = tf.registry.create_readable_field<f_quat_t>("foo4");
    auto foo5_fp = tf.registry.create_writable_vector_field<float>("foo5", 0, 3, 25);
    auto foo6_fp = tf.registry.create_readable_field<signed char>("foo6", 0, 2, 3);
    auto foo7_fp = tf.registry.create_readable_field<unsigned char>("foo7", 25);
    auto foo8_fp = tf.registry.create_readable_field<bool>("foo8");
    auto foo9_fp = tf.registry.create_readable_field<signed int>("foo9", -2, 8);
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

    // TODO add testing of actual downlink data once the quaternion and
    // vector serializers work properly.

    print_hex(tf.snapshot_ptr_fp->get(), 32);
}

/**
 * @brief Test with multiple flows.
 */
void test_multiple_flows() {
    // Test with multiple flows that fit within a downlink packet.
    {
        TestFixture tf;

        auto foo1_fp = tf.registry.create_readable_field<gps_time_t>("foo1");
        auto foo2_fp = tf.registry.create_readable_field<gps_time_t>("foo2");
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

        foo1_fp->set(gps_time_t(1000, 200, 0));
        foo2_fp->set(gps_time_t(1000, 500, 0));
        tf.cycle_no_fp->set(20);
        tf.downlink_producer->execute();
        // const char expected_outputs[34] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        print_hex(tf.snapshot_ptr_fp->get(), 34);
        // TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 34);        
    }

    // Test with multiple flows that require more than one downlink packet.
    {
        TestFixture tf;

        auto foo1_fp = tf.registry.create_readable_field<gps_time_t>("foo1");
        auto foo2_fp = tf.registry.create_readable_field<gps_time_t>("foo2");

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

        foo1_fp->set(gps_time_t(1000, 200, 0));
        foo2_fp->set(gps_time_t(1000, 500, 0));
        tf.cycle_no_fp->set(20);
        tf.downlink_producer->execute();
        // const char expected_outputs[80] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //     0x00};
        print_hex(tf.snapshot_ptr_fp->get(), 80);
        // TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 80);
    }
}

/**
 * @brief Verify that setting some flows inactive does not change the initial snapshot size,
 * but does change the snapshot size after one execution. Test that inactive flows don't
 * appear in the downlink snapshot.
 */
void test_some_flows_inactive() {
    TestFixture tf;

    auto foo1_fp = tf.registry.create_readable_field<gps_time_t>("foo1");
    auto foo2_fp = tf.registry.create_readable_field<gps_time_t>("foo2");

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

    // The flow is the same as the previous test, so the
    // size should be the same
    TEST_ASSERT_EQUAL(80, tf.snapshot_size_bytes_fp->get());
    tf.downlink_producer->execute();
    // ceil((1 + 32 + (121 + 121) + 1) / 8)
    TEST_ASSERT_EQUAL(35, tf.snapshot_size_bytes_fp->get());

    foo1_fp->set(gps_time_t(1000, 200, 0));
    foo2_fp->set(gps_time_t(1000, 500, 0));
    tf.cycle_no_fp->set(20);
    tf.downlink_producer->execute();
    // const char expected_outputs[35] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    print_hex(tf.snapshot_ptr_fp->get(), 35);
    // TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), 35);
}

#pragma GCC diagnostic pop

int test_downlink_producer_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_one_flow);
    RUN_TEST(test_one_flow_multityped);
    RUN_TEST(test_multiple_flows);
    RUN_TEST(test_some_flows_inactive);
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
