#include <array>
#include <static_buffers.hpp>
#include "utils_tests.hpp"

// Tests if buffer works fine with just one element.
static void circular_stack_test_one_element(void) {
    circular_stack<unsigned int, 1> buf;
    TEST_ASSERT(buf.empty());
    buf.put(2);
    unsigned int v1 = buf.get();
    TEST_ASSERT_EQUAL(2, v1);
    TEST_ASSERT(buf.empty());
}

// Tests FIFO functionality of buffer.
static void circular_stack_test_multiple_elements(void) {
    circular_stack<unsigned int, 2> buf;
    TEST_ASSERT(buf.empty());
    buf.put(1);
    buf.put(2);
    unsigned int v1 = buf.get();
    TEST_ASSERT_EQUAL(2, v1);
    unsigned int v2 = buf.get();
    TEST_ASSERT_EQUAL(1, v2);
    TEST_ASSERT(buf.empty());
}

// Tests FIFO functionality of buffer with arrays as elements
static void circular_stack_test_multiple_elements_array(void) {
    circular_stack<std::array<unsigned int, 1>, 2> buf;
    TEST_ASSERT(buf.empty());

    std::array<unsigned int, 1> el1 = {1};
    std::array<unsigned int, 1> el2 = {2};
    buf.put(el1);
    buf.put(el2);
    auto v1 = buf.get();
    TEST_ASSERT_EQUAL(el2[0], v1[0]);
    auto v2 = buf.get();
    TEST_ASSERT_EQUAL(el1[0], v2[0]);
    TEST_ASSERT(buf.empty());
}

// Tests if buffer works fine with just one element.
static void circular_buffer_test_one_element(void) {
    circular_buffer<unsigned int, 1> buf;
    TEST_ASSERT(buf.empty());
    buf.put(2);
    TEST_ASSERT_FALSE(buf.empty());
    unsigned int v1 = buf.get();
    TEST_ASSERT_EQUAL(v1, 2);
    TEST_ASSERT(buf.empty());
}

// Tests FIFO functionality of buffer.
static void circular_buffer_test_multiple_elements(void) {
    circular_buffer<unsigned int, 2> buf;
    TEST_ASSERT(buf.empty());
    buf.put(1);
    buf.put(2);
    TEST_ASSERT_FALSE(buf.empty());
    unsigned int v1 = buf.get();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_FALSE(buf.empty());
    unsigned int v2 = buf.get();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT(buf.empty());
}

// Tests FIFO functionality of buffer with arrays as elements
static void circular_buffer_test_multiple_elements_array(void) {
    circular_buffer<std::array<unsigned int, 1>, 2> buf;
    TEST_ASSERT(buf.empty());

    std::array<unsigned int, 1> el1 = {1};
    std::array<unsigned int, 1> el2 = {2};
    buf.put(el1);
    TEST_ASSERT_FALSE(buf.empty());
    buf.put(el2);
    auto v1 = buf.get();
    TEST_ASSERT_EQUAL(el1[0], v1[0]);
    TEST_ASSERT_FALSE(buf.empty());
    auto v2 = buf.get();
    TEST_ASSERT_EQUAL(el2[0], v2[0]);
    TEST_ASSERT(buf.empty());
}

void test_static_buffers() {
    RUN_TEST(circular_stack_test_one_element);
    RUN_TEST(circular_stack_test_multiple_elements);
    RUN_TEST(circular_stack_test_multiple_elements_array);
    RUN_TEST(circular_buffer_test_one_element);
    RUN_TEST(circular_buffer_test_multiple_elements);
    RUN_TEST(circular_buffer_test_multiple_elements_array);
}