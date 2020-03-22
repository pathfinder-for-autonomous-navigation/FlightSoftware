/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @brief Simple utility for testing out serializers.
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <common/Serializer.hpp>
#include <iostream>

int main() {
    // CHANGE THESE NEXT TWO LINES TO WHATEVER YOU WANT
    Serializer<f_vector_t> x(0, 10, 40);
    x.serialize({2,0,0});

    const bit_array& ba = x.get_bit_array();
    for(const bool b : ba) {
        std::cout << (b ? 1 : 0);
    }
    std::cout << std::endl;

    return 0;
}
