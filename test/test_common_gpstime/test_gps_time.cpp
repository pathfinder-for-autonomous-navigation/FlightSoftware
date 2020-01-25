#include <unity.h>
#include <common/GPSTime.hpp>

static constexpr unsigned long nanoseconds_in_week = 7 * 24 * 60 * 60 * (unsigned long) (1000000000);

void test_basic_constructors() {
    gps_time_t t;
    TEST_ASSERT_FALSE(t.is_set);
    TEST_ASSERT_EQUAL(0, t.wn);
    TEST_ASSERT_EQUAL(0, t.tow);
    TEST_ASSERT_EQUAL(0, t.ns);

    gps_time_t t2(2075, 572522, 2000); // Sometime in the morning of Oct. 19, 2019
    TEST_ASSERT(t2.is_set);
    TEST_ASSERT_EQUAL(2075, t2.wn);
    TEST_ASSERT_EQUAL(572522, t2.tow);
    TEST_ASSERT_EQUAL(2000, t2.ns);
}

void test_copy_constructors_and_assignment_operators() {
    gps_time_t t;
    gps_time_t t2(2075, 572522, 2000); // Sometime in the morning of Oct. 19, 2019
    msg_gps_time_t t3 {2075, 572522, 2000};
    
    gps_time_t t4 = t;
    TEST_ASSERT_EQUAL(t.is_set, t4.is_set);
    TEST_ASSERT_EQUAL(t.wn, t4.wn);
    TEST_ASSERT_EQUAL(t.tow, t4.tow);
    TEST_ASSERT_EQUAL(t.ns, t4.ns);

    gps_time_t t5 = t2;
    TEST_ASSERT_EQUAL(t2.is_set, t5.is_set);
    TEST_ASSERT_EQUAL(t2.wn, t5.wn);
    TEST_ASSERT_EQUAL(t2.tow, t5.tow);
    TEST_ASSERT_EQUAL(t2.ns, t5.ns);

    gps_time_t t6 = t3;
    TEST_ASSERT(t6.is_set);
    TEST_ASSERT_EQUAL(t3.wn, t6.wn);
    TEST_ASSERT_EQUAL(t3.tow, t6.tow);
    TEST_ASSERT_EQUAL(t3.ns, t6.ns);

    t6 = t2;
    TEST_ASSERT_EQUAL(t2.is_set, t6.is_set);
    TEST_ASSERT_EQUAL(t2.wn, t6.wn);
    TEST_ASSERT_EQUAL(t2.tow, t6.tow);
    TEST_ASSERT_EQUAL(t2.ns, t6.ns);

    t5 = t3;
    TEST_ASSERT(t5.is_set);
    TEST_ASSERT_EQUAL(t3.wn, t5.wn);
    TEST_ASSERT_EQUAL(t3.tow, t5.tow);
    TEST_ASSERT_EQUAL(t3.ns, t5.ns);
}

void test_cast() {
    gps_time_t t;
    TEST_ASSERT_EQUAL(0, static_cast<unsigned long>(t)); 

    gps_time_t t2(2, 2, 2);
    const unsigned long e2 = t2.wn * nanoseconds_in_week + t2.tow * 1000000
                             + t2.ns;
    TEST_ASSERT_EQUAL(e2, static_cast<unsigned long>(t2)); 

    gps_time_t t3(2075, 572522, 2000);
    const unsigned long e3 = t3.wn * nanoseconds_in_week + t3.tow * 1000000
                             + t3.ns;
    TEST_ASSERT_EQUAL(e3, static_cast<unsigned long>(t3)); 
}

// Helper function for test_bool_operators.
// Compares a GPS time to itself and verifies that the boolean assertions work correctly.
void test_bool_operator_reflexive(gps_time_t t) {
    t.is_set = false;
    TEST_ASSERT_FALSE(t == t);
    t.is_set = true;
    TEST_ASSERT(t == t);
    TEST_ASSERT_FALSE(t != t);
    TEST_ASSERT_FALSE(t < t);
    TEST_ASSERT_FALSE(t > t);
    TEST_ASSERT(t <= t);
    TEST_ASSERT(t >= t);
}

// Compares a GPS time to an equal of itself and verifies that the boolean
// assertions work correctly.
void test_bool_operator_reflexive(gps_time_t t1, gps_time_t t2) {
    t1.is_set = false;
    t2.is_set = false;
    TEST_ASSERT_FALSE(t1 == t2);
    t1.is_set = true;
    TEST_ASSERT_FALSE(t1 == t2);
    t2.is_set = true;
    TEST_ASSERT(t1 == t2);

    TEST_ASSERT_FALSE(t1 != t2);
    TEST_ASSERT_FALSE(t1 < t2);
    TEST_ASSERT_FALSE(t1 > t2);
    TEST_ASSERT(t1 <= t2);
    TEST_ASSERT(t1 >= t2);
}

// Test boolean assertions between two GPS times. Requires x > y.
void test_bool_operator_comparison(gps_time_t x, gps_time_t y) {
    TEST_ASSERT_FALSE(x == y);
    TEST_ASSERT(x != y);
    x.is_set = false;
    y.is_set = false;
    TEST_ASSERT_FALSE(x < y);
    x.is_set = true;
    TEST_ASSERT_FALSE(x < y);
    y.is_set = true;
    TEST_ASSERT(x < y);

    TEST_ASSERT(x <= y);
    TEST_ASSERT_FALSE(x > y);
    TEST_ASSERT_FALSE(x >= y);
    TEST_ASSERT_FALSE(y == x);
    TEST_ASSERT(y != x);
    TEST_ASSERT(y > x);
    TEST_ASSERT(y >= x);
    TEST_ASSERT_FALSE(y < x);
    TEST_ASSERT_FALSE(y <= x);
}

// Test boolean assertions between a GPS time and an integer. Requires x < y.
void test_bool_operator_comparison(gps_time_t x, const unsigned long y) {
    TEST_ASSERT_FALSE(x == y);
    TEST_ASSERT(x != y);
    x.is_set = false;
    TEST_ASSERT_FALSE(x < y);
    x.is_set = true;
    TEST_ASSERT(x < y);
    TEST_ASSERT(x <= y);
    TEST_ASSERT_FALSE(x > y);
    TEST_ASSERT_FALSE(x >= y);
}

void test_bool_operators() {
    gps_time_t t;
    test_bool_operator_reflexive(t);
    test_bool_operator_reflexive(t, t);

    gps_time_t t1(2,2,2);
    test_bool_operator_reflexive(t1);
    test_bool_operator_reflexive(t1, t1);
    test_bool_operator_comparison(t, t1);
    test_bool_operator_comparison(t1, static_cast<unsigned long>(t1) + 2);

    gps_time_t t2(2075, 572522, 2000);
    test_bool_operator_reflexive(t2);
    test_bool_operator_reflexive(t2, t2);
    test_bool_operator_comparison(t, t2);
    test_bool_operator_comparison(t1, t2);
    test_bool_operator_comparison(t1, static_cast<unsigned long>(t2) + 2);
}

void test_addition_operators() {
    gps_time_t t;
    gps_time_t t1(2,2,2);
    gps_time_t t2(4,4,4);
    unsigned long dt = 2 * nanoseconds_in_week + 2 * 1000000 + 2;

    TEST_ASSERT(t1 == t + t1);
    TEST_ASSERT(t1 == t1 + t);
    TEST_ASSERT(t2 == t1 + t1);

    TEST_ASSERT(t1 == t + dt);
    TEST_ASSERT(t2 == t1 + dt);
}

void test_subtraction_operators() {
    gps_time_t t;
    t.is_set = true;
    gps_time_t t1(2,2,2);
    gps_time_t t2(4,4,4);
    unsigned long dt = 2 * nanoseconds_in_week + 2 * 1000000 + 2;

    TEST_ASSERT(t1 == t1 - t);
    TEST_ASSERT(t1 == t2 - t1);
    
    TEST_ASSERT(t1 == t2 - dt);
    TEST_ASSERT(t == t1 - dt);

    gps_time_t t3 = t1 - t2;
    TEST_ASSERT(t3.wn == t1.wn);
    TEST_ASSERT(t3.tow == t1.tow);
    TEST_ASSERT(t3.ns == t1.ns);
    TEST_ASSERT_FALSE(t3.is_set);
}

int test_gps_time() {
    UNITY_BEGIN();
    RUN_TEST(test_basic_constructors);
    RUN_TEST(test_copy_constructors_and_assignment_operators);
    RUN_TEST(test_cast);
    RUN_TEST(test_bool_operators);
    RUN_TEST(test_addition_operators);
    RUN_TEST(test_subtraction_operators);
    return UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char *argv[]) {
    return test_gps_time();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_gps_time();
}

void loop() {}
#endif
