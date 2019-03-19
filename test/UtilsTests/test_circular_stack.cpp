#include <unity_fixture.h>
#include <circular_stack.hpp>
#include <array>

TEST_GROUP(CircularStackTests);
TEST_SETUP(CircularStackTests) {

}
TEST_TEAR_DOWN(CircularStackTests) {}

// Tests if buffer works fine with just one element.
TEST(CircularStackTests, test_one_element) {
    circular_stack<unsigned int, 1> buf;
    TEST_ASSERT(buf.empty());
    buf.put(2);
    unsigned int v1 = buf.get();
    TEST_ASSERT_EQUAL(2, v1);
    TEST_ASSERT(buf.empty());
}

// Tests FIFO functionality of buffer.
TEST(CircularStackTests, test_multiple_elements) {
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
TEST(CircularStackTests, test_multiple_elements_array) {
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

TEST_GROUP_RUNNER(CircularStackTests) {
    RUN_TEST_CASE(CircularStackTests, test_one_element);
    RUN_TEST_CASE(CircularStackTests, test_multiple_elements);
    RUN_TEST_CASE(CircularStackTests, test_multiple_elements_array);
}