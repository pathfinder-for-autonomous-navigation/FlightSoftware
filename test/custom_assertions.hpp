#ifndef CUSTOM_ASSERTIONS_HPP_
#define CUSTOM_ASSERTIONS_HPP_

#include <unity.h>

#define PAN_TEST_ASSERT_EQUAL_FLOAT_ARR(expected, actual, delta, n) { \
    char err_str[25]; \
    for(size_t i = 0; i < n; i++) { \
        sprintf(err_str, "Fail on element %d", (int) i); \
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(delta, expected[i], actual[i], err_str); \
    } \
}

#define PAN_TEST_ASSERT_EQUAL_DOUBLE_ARR(expected, actual, delta, n) { \
    char err_str[25]; \
    for(size_t i = 0; i < n; i++) { \
        sprintf(err_str, "Fail on element %d", (int) i); \
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(delta, expected[i], actual[i], err_str); \
    } \
}

#define PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(expected, actual, delta)   PAN_TEST_ASSERT_EQUAL_FLOAT_ARR(expected, actual, delta, 3)
#define PAN_TEST_ASSERT_EQUAL_FLOAT_QUAT(expected, actual, delta)  PAN_TEST_ASSERT_EQUAL_FLOAT_ARR(expected, actual, delta, 4)
#define PAN_TEST_ASSERT_EQUAL_DOUBLE_VEC(expected, actual, delta)  PAN_TEST_ASSERT_EQUAL_DOUBLE_ARR(expected, actual, delta, 3)
#define PAN_TEST_ASSERT_EQUAL_DOUBLE_QUAT(expected, actual, delta) PAN_TEST_ASSERT_EQUAL_DOUBLE_ARR(expected, actual, delta, 4)

#define PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(expected, actual, delta) { \
    char err_str[25]; \
    for(size_t i = 0; i < 3; i++) { \
        sprintf(err_str, "Fail on element %d", (int) i); \
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(delta, expected(i), actual(i), err_str); \
    } \
}

#define PAN_TEST_ASSERT_EQUAL_DOUBLE_LIN_VEC(expected, actual, delta) { \
    char err_str[25]; \
    for(size_t i = 0; i < 3; i++) { \
        sprintf(err_str, "Fail on element %d", (int) i); \
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(delta, expected(i), actual(i), err_str); \
    } \
}


/**
 * Lin Tensor Printing function
 * */
template<typename T>
inline void printtensor(T x){
    int n= x.rows();
    int m= x.cols();
    for (int i=0; i<n; i++){
        char linemessage[256];
        char *point= linemessage;
        for (int j=0; j<m; j++){
	            point+= sprintf(point,"%.10e, ",x(i,j));
	    }
        TEST_MESSAGE(linemessage);
    }
}


/** 
 * Asserts (lin::norm(expected-actual)<=delta) and prints debug info on failure
 */
#define PAN_TEST_ASSERT_LIN_3VECT_WITHIN(delta, expected, actual) do {\
            if (!(lin::norm(expected-actual)<=delta)){ \
                char errormessage[256];\
                TEST_MESSAGE("TEST_ASSERT_LIN_3VECT_WITHIN Failed:");\
                sprintf(errormessage, "    expected " #expected " is %.10e, %.10e, %.10e",expected(0),expected(1),expected(2));\
                TEST_MESSAGE(errormessage);\
                sprintf(errormessage, "    actual " #actual " is %.10e, %.10e, %.10e",actual(0),actual(1),actual(2));\
                TEST_MESSAGE(errormessage);\
                sprintf(errormessage, "    expected-actual: %.10e, %.10e, %.10e",expected(0)-actual(0),expected(1)-actual(1),expected(2)-actual(2));\
                TEST_MESSAGE(errormessage);\
                sprintf(errormessage, "    norm(expected-actual): %.10e",lin::norm(expected-actual));\
                TEST_MESSAGE(errormessage);\
                sprintf(errormessage, "    max error: %.10e",delta);\
                TEST_MESSAGE(errormessage);\
                TEST_FAIL();\
            }\
           } while(0) 

#endif
