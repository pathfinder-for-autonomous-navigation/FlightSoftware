#include "cartesian_product.hpp"
#include "../custom_assertions.hpp"
#undef isnan
#undef isinf
#include <iostream>

void test_cartesian_product() {
    // Edge case testing
    std::array<int, 0> x0 {};
    std::vector<std::array<int, 1>> x0_1 = NthCartesianProduct<1>::of(x0);
    bool t0_1 = x0_1 == std::vector<std::array<int, 1>>{};
    TEST_ASSERT(t0_1);

    std::array<int, 1> x1 {1};
    std::vector<std::array<int, 0>> x1_0 = NthCartesianProduct<0>::of(x1);
    bool t1_0 = x1_0 == std::vector<std::array<int, 0>>{ {{}} };
    TEST_ASSERT(t1_0);

    std::array<int, 2> x2 {1,2};
    std::vector<std::array<int, 0>> x2_0 = NthCartesianProduct<0>::of(x2);
    bool t2_0 = x2_0 == std::vector<std::array<int, 0>>{ {{}} };
    TEST_ASSERT(t2_0);

    // Simple cases
    std::vector<std::array<int, 1>> x1_1 = NthCartesianProduct<1>::of(x1);
    bool t1_1 = x1_1 == std::vector<std::array<int, 1>>{ {{1}} };
    TEST_ASSERT_TRUE(t1_1);

    std::vector<std::array<int, 2>> x1_2 = NthCartesianProduct<2>::of(x1);
    bool t1_2 = x1_2 == std::vector<std::array<int, 2>>{ {{1, 1}} };
    TEST_ASSERT_TRUE(t1_2);

    std::vector<std::array<int, 1>> x2_1 = NthCartesianProduct<1>::of(x2);
    bool t2_1 = x2_1 == std::vector<std::array<int, 1>>{ {{1}, {2}} };
    TEST_ASSERT_TRUE(t2_1);

    // Slightly more complex cases
    std::vector<std::array<int, 2>> x2_2 = NthCartesianProduct<2>::of(x2);
    bool t2_2 = x2_2 == std::vector<std::array<int, 2>>{ {{1,1},{1,2},{2,1},{2,2}} };
    TEST_ASSERT_TRUE(t2_2);

    std::vector<std::array<int, 3>> x2_3 = NthCartesianProduct<3>::of(x2);
    bool t2_3 = x2_3 == std::vector<std::array<int, 3>> { 
        {{1,1,1},{1,1,2},{1,2,1},{1,2,2},{2,1,1},{2,1,2},{2,2,1},{2,2,2}} };
    TEST_ASSERT_TRUE(t2_3);

    // Stress test
    std::array<int, 10> x10 {1,2,3,4,5,6,7,8,9,10};
    std::vector<std::array<int, 4>> x10_4 = NthCartesianProduct<4>::of(x10);
    TEST_ASSERT_EQUAL(10000, x10_4.size());
    std::vector<std::array<int, 5>> x10_5 = NthCartesianProduct<5>::of(x10);
    TEST_ASSERT_EQUAL(100000, x10_5.size());
}
