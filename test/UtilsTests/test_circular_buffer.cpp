#include <unity_fixture.h>
#include <circular_buffer.hpp>
#include <array>

TEST_GROUP(CircularBufferTests);
TEST_SETUP(CircularBufferTests) {

}
TEST_TEAR_DOWN(CircularBufferTests) {}

// Tests if buffer works fine with just one element.
TEST(CircularBufferTests, test_one_element) {
    circular_buffer<unsigned int, 1> buf;
    TEST_ASSERT(buf.empty());
    buf.put(2);
    TEST_ASSERT_FALSE(buf.empty());
    unsigned int v1 = buf.get();
    TEST_ASSERT_EQUAL(v1, 2);
    TEST_ASSERT(buf.empty());
}

// Tests FIFO functionality of buffer.
TEST(CircularBufferTests, test_multiple_elements) {
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
TEST(CircularBufferTests, test_multiple_elements_array) {
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

TEST_GROUP_RUNNER(CircularBufferTests) {
    RUN_TEST_CASE(CircularBufferTests, test_one_element);
    RUN_TEST_CASE(CircularBufferTests, test_multiple_elements);
    RUN_TEST_CASE(CircularBufferTests, test_multiple_elements_array);
}