#include "cartesian_product.hpp"
#include <unity.h>

void test_cartesian_product() {
    // Edge case testing
    std::array<int, 0> x0 {};
    auto x0_1 = NthCartesianProduct<int, 0, 1>::of(x0);
    bool t0_1 = x0_1 == std::array<std::array<int, 1>, 0>{ {} };
    TEST_ASSERT(t0_1);

    std::array<int, 1> x1 {1};
    auto x1_0 = NthCartesianProduct<int, 1, 0>::of(x1);
    bool t1_0 = x1_0 == std::array<std::array<int, 0>, 1>{ {{}} };
    TEST_ASSERT(t1_0);

    std::array<int, 2> x2 {1,2};
    auto x2_0 = NthCartesianProduct<int, 2, 0>::of(x2);
    bool t2_0 = x2_0 == std::array<std::array<int, 0>, 1>{ {{}} };
    TEST_ASSERT(t2_0);

    // Simple cases
    auto x1_1 = NthCartesianProduct<int, 1, 1>::of(x1);
    bool t1_1 = x1_1 == std::array<std::array<int, 1>, 1>{ {{1}} };
    TEST_ASSERT_TRUE(t1_1);

    auto x1_2 = NthCartesianProduct<int, 1, 2>::of(x1);
    bool t1_2 = x1_2 == std::array<std::array<int, 2>, 1>{ {{1, 1}} };
    TEST_ASSERT_TRUE(t1_2);

    auto x2_1 = NthCartesianProduct<int, 2, 1>::of(x2);
    bool t2_1 = x2_1 == std::array<std::array<int, 1>, 2>{ {{1}, {2}} };
    TEST_ASSERT_TRUE(t2_1);

    // Slightly more complex cases
    auto x2_2 = NthCartesianProduct<int, 2, 2>::of(x2);
    bool t2_2 = x2_2 == std::array<std::array<int, 2>, 4>{ {{1,1},{1,2},{2,1},{2,2}} };
    TEST_ASSERT_TRUE(t2_2);

    auto x2_3 = NthCartesianProduct<int, 2, 3>::of(x2);
    bool t2_3 = x2_3 == std::array<std::array<int, 3>, 8> { 
        {{1,1,1},{1,1,2},{1,2,1},{1,2,2},{2,1,1},{2,1,2},{2,2,1},{2,2,2}} };
    TEST_ASSERT_TRUE(t2_3);

    // Stress test
    std::array<int, 8> x8 {1,2,3,4,5,6,7,8};
    auto x8_4 = NthCartesianProduct<int, 8, 4>::of(x8);
    TEST_ASSERT_EQUAL(4096, x8_4.size());
    auto x8_5 = NthCartesianProduct<int, 8, 5>::of(x8);
    TEST_ASSERT_EQUAL(32768, x8_5.size());
}
