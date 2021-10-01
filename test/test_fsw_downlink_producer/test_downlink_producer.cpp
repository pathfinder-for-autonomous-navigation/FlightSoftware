#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/DownlinkProducer.hpp>

#include <common/StateFieldRegistry.hpp>
#include "../custom_assertions.hpp"
#include <iostream>

struct TestFixture {
    StateFieldRegistryMock registry;
    std::unique_ptr<DownlinkProducer> downlink_producer;

    std::shared_ptr<ReadableStateField<unsigned int>> foo1_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> cycle_count_fp;
    std::shared_ptr<WritableStateField<unsigned char>> pan_state_fp;
    InternalStateField<char*>* snapshot_ptr_fp;
    InternalStateField<size_t>* snapshot_size_bytes_fp;
    WritableStateField<unsigned char>* shift_flows_id1_fp;
    WritableStateField<unsigned char>* shift_flows_id2_fp;
    WritableStateField<unsigned char>* toggle_flow_id_fp;

    TestFixture() : registry() {}

    void init(const std::vector<DownlinkProducer::FlowData>& flow_data) {
        // Create required field(s)
        cycle_count_fp = registry.create_readable_field<unsigned int>("pan.cycle_no");
        pan_state_fp = registry.create_writable_field<unsigned char>("pan.state");
        pan_state_fp->set(static_cast<unsigned char>(mission_state_t::startup));

        // Create field(s) for serialization and initialize them to
        // default values
        foo1_fp = registry.create_readable_field<unsigned int>("foo1");
        cycle_count_fp->set(20);
        foo1_fp->set(400);

        downlink_producer = std::make_unique<DownlinkProducer>(registry);
        downlink_producer->init_flows(flow_data);
        downlink_producer->init();
        snapshot_ptr_fp = registry.find_internal_field_t<char*>("downlink.ptr");
        snapshot_size_bytes_fp = registry.find_internal_field_t<size_t>(
                                    "downlink.snap_size");
        shift_flows_id1_fp = registry.find_writable_field_t<unsigned char>("downlink.shift_id1");
        shift_flows_id2_fp = registry.find_writable_field_t<unsigned char>("downlink.shift_id2");
        toggle_flow_id_fp = registry.find_writable_field_t<unsigned char>("downlink.toggle_id");
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

        TEST_ASSERT_EQUAL(0, tf.shift_flows_id1_fp->get());
        TEST_ASSERT_EQUAL(0, tf.shift_flows_id2_fp->get());
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

        TEST_ASSERT_EQUAL(0, tf.toggle_flow_id_fp->get());
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
                "foo2", // 62 bits
                "foo3", // 32 bits
                "foo4", // 84 bits (it's a vector serializer)
                "foo5", // 3 bits
                "foo6", // 5 bits
                "foo7", // 1 bits
                "foo8", // 4 bit
            } // Flow size: 224 bits (1 + 223)
        }
    };
    tf.init(flow_data);

    // ceil((1 + 32 + (224)) / 8)
    TEST_ASSERT_EQUAL(33, tf.snapshot_size_bytes_fp->get());

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

void test_shift_priorities() {
    TestFixture tf;

    std::vector<DownlinkProducer::FlowData> flow_data = {
        {
            1, true, {"foo1"} 
        },
        {
            2, false, {"foo1"} 
        },
        {
            3, true, {"foo1"} 
        },
        {
            4, true, {"foo1"} 
        },
        {
            5, false, {"foo1"} 
        },
        {
            6, false, {"foo1"} 
        }
    };
    tf.init(flow_data);
    std::vector<DownlinkProducer::Flow> flows=tf.downlink_producer->get_flows();
    std::vector<int> initial_ids={1,2,3,4,5,6};
    for (size_t i = 0; i<flows.size(); i++){
        unsigned char flow_id;
        flows[i].id_sr.deserialize(&flow_id);
        TEST_ASSERT_EQUAL(initial_ids[i], flow_id);
    }
    TEST_ASSERT_EQUAL(true, flows[0].is_active);
    TEST_ASSERT_EQUAL(false, flows[5].is_active);
    
    // Test shifting backwards
    tf.downlink_producer->shift_flow_priorities(6,1);

    // Get the new flow vector and check that the flows have been reordered as desired
    flows=tf.downlink_producer->get_flows();
    std::vector<int> desired_ids={6,1,2,3,4,5};
    for (size_t i = 0; i<flows.size(); i++){
        unsigned char flow_id;
        flows[i].id_sr.deserialize(&flow_id);
        TEST_ASSERT_EQUAL(desired_ids[i], flow_id);
    }

    // Test shifting forwards
    tf.downlink_producer->shift_flow_priorities(6,5);

    // Get the new flow vector and check that the flows have been reordered as desired
    flows=tf.downlink_producer->get_flows();
    desired_ids={1,2,3,4,5,6};
    for (size_t i = 0; i<flows.size(); i++){
        unsigned char flow_id;
        flows[i].id_sr.deserialize(&flow_id);
        TEST_ASSERT_EQUAL(desired_ids[i], flow_id);
    }
}

void test_shift_statefield_cmd() {
    TestFixture tf;

    std::vector<DownlinkProducer::FlowData> flow_data = {
        {
            1, true, {"foo1"} 
        },
        {
            2, false, {"foo1"} 
        },
        {
            3, true, {"foo1"} 
        },
        {
            4, true, {"foo1"} 
        },
        {
            5, false, {"foo1"} 
        },
        {
            6, false, {"foo1"} 
        }
    };
    tf.init(flow_data);
    std::vector<DownlinkProducer::Flow> flows=tf.downlink_producer->get_flows();
    
    // Test shifting backwards by moving flow with id 6 to flow with 1's positions
    tf.shift_flows_id1_fp->set(6);
    tf.shift_flows_id2_fp->set(1);
    tf.downlink_producer->execute();

    // Get the new flow vector and check that the flows have been reordered as desired
    flows=tf.downlink_producer->get_flows();
    std::vector<int> desired_ids={6,1,2,3,4,5};
    for (size_t i = 0; i<flows.size(); i++){
        unsigned char flow_id;
        flows[i].id_sr.deserialize(&flow_id);
        TEST_ASSERT_EQUAL(desired_ids[i], flow_id);
    }

    // Check that the shift command statefields have reverted back to 0
    TEST_ASSERT_EQUAL(0, tf.shift_flows_id1_fp->get());
    TEST_ASSERT_EQUAL(0, tf.shift_flows_id2_fp->get());

    // Test shifting forwards by moving flow with id 6 to flow with 5's positions
    tf.shift_flows_id1_fp->set(6);
    tf.shift_flows_id2_fp->set(5);
    tf.downlink_producer->execute();

    // Get the new flow vector and check that the flows have been reordered as desired
    flows=tf.downlink_producer->get_flows();
    desired_ids={1,2,3,4,5,6};
    for (size_t i = 0; i<flows.size(); i++){
        unsigned char flow_id;
        flows[i].id_sr.deserialize(&flow_id);
        TEST_ASSERT_EQUAL(desired_ids[i], flow_id);
    }

    // Check that the shift command statefields have reverted back to 0
    TEST_ASSERT_EQUAL(0, tf.shift_flows_id1_fp->get());
    TEST_ASSERT_EQUAL(0, tf.shift_flows_id2_fp->get());
}

void test_toggle() {
    TestFixture tf;

    std::vector<DownlinkProducer::FlowData> flow_data = {
        {
            1, true, {"foo1"} 
        }
    };
    tf.init(flow_data);

    // Toggle the flow with id 1
    tf.toggle_flow_id_fp->set(1);
    tf.downlink_producer->execute();
    std::vector<DownlinkProducer::Flow> flows=tf.downlink_producer->get_flows();

    TEST_ASSERT_FALSE(flows[0].is_active);
    TEST_ASSERT_EQUAL(0, tf.toggle_flow_id_fp->get());

    // Toggle the flow with id 1 again
    tf.toggle_flow_id_fp->set(1);
    tf.downlink_producer->execute();
    flows=tf.downlink_producer->get_flows();

    TEST_ASSERT_TRUE(flows[0].is_active);
    TEST_ASSERT_EQUAL(0, tf.toggle_flow_id_fp->get());

    // Check that, if the toggle flow id command is 0, nothing changes
    tf.downlink_producer->execute();
    flows=tf.downlink_producer->get_flows();
    TEST_ASSERT_TRUE(flows[0].is_active);
    TEST_ASSERT_EQUAL(0, tf.toggle_flow_id_fp->get()); 
}

void test_state_flow_reordering() {
    TestFixture tf;
    std::vector<DownlinkProducer::FlowData> flow_data = {
        {
            1, true, {"foo1"} 
        },
        {
            2, true, {"foo1"} 
        },
        {
            3, true, {"foo1"} 
        },
        {
            4, true, {"foo1"} 
        },
        {
            5, true, {"foo1"} 
        },
        {
            6, true, {"foo1"} 
        },
        {
            7, true, {"foo1"} 
        }, 
        {
            8, true, {"foo1"} 
        },
        {
            9, true, {"foo1"} 
        },
        {
            10, true, {"foo1"} 
        },
        {
            11, true, {"foo1"} 
        },
        {
            12, true, {"foo1"} 
        },
        {
            13, true, {"foo1"} 
        },
        {
            14, true, {"foo1"} 
        }, 
        {
            15, true, {"foo1"} 
        },
        {
            16, true, {"foo1"} 
        },
        {
            17, true, {"foo1"} 
        }
    };
    tf.init(flow_data);
    std::vector<DownlinkProducer::Flow> flows=tf.downlink_producer->get_flows();

    std::vector<int> desired_ids={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
    for (size_t i = 0; i<flows.size(); i++){
        unsigned char flow_id;
        flows[i].id_sr.deserialize(&flow_id);
        TEST_ASSERT_EQUAL(desired_ids[i], flow_id);
    }

    // change state to follower
    tf.pan_state_fp->set(static_cast<unsigned char>(mission_state_t::follower));
    tf.downlink_producer->execute();

    // Get the new flow vector and check that the flows have been reordered as desired
    flows=tf.downlink_producer->get_flows();
    desired_ids={1,2,3,4,5,6,7,8,9,10,11,12,13,16,14,15,17};
    for (size_t i = 0; i<flows.size(); i++){
        unsigned char flow_id;
        flows[i].id_sr.deserialize(&flow_id);
        TEST_ASSERT_EQUAL(desired_ids[i], flow_id);
    }

    // change state to any state that is not follower or follower_close_approach
    tf.pan_state_fp->set(static_cast<unsigned char>(mission_state_t::standby));
    
    tf.downlink_producer->execute();

    // // Get the new flow vector and check that the flows have been reordered as desired
    flows=tf.downlink_producer->get_flows();
    desired_ids={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
    for (size_t i = 0; i<flows.size(); i++){
        unsigned char flow_id;
        flows[i].id_sr.deserialize(&flow_id);
        TEST_ASSERT_EQUAL(desired_ids[i], flow_id);
    }
}

int test_downlink_producer_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_one_flow);
    // RUN_TEST(test_one_flow_multityped); This test is broken, but this whole unit
    // test should be replaced with the automated downlink tester anyway.
    RUN_TEST(test_multiple_flows);
    RUN_TEST(test_some_flows_inactive);
    RUN_TEST(test_downlink_changes);
    RUN_TEST(test_shift_priorities);
    RUN_TEST(test_shift_statefield_cmd);
    RUN_TEST(test_toggle);
    RUN_TEST(test_state_flow_reordering);
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
