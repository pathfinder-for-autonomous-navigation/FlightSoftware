#ifndef CARTESIAN_PRODUCT_HPP_
#define CARTESIAN_PRODUCT_HPP_

#include <array>
#include <vector>
#include <cstddef>

///////////////////////////////////////////////////////////////////
// Helper math functions for testing combinations of faults
// Used by test_main_fault_handler.cpp
///////////////////////////////////////////////////////////////////

// Integer exponentiation.
template<size_t A, int X>
constexpr size_t intpow() {
    static_assert(X >= 0, "This function computes an integer-valued "
                         "exponentiation and requires a positive exponent"
                         "argument.");
    static_assert(A != 0 || X > 0, "0^0 is undefined.");

    size_t result = 1;
    for(int i = X; i > 0; i--) result *= A;
    return result;
}

// Generate the Nth Cartesian product of an array A with itself,
// where the elements in A are of type T and X = |A|.
//
// Usage: A^N = NthCartesianProduct<N>::of(A);
//
template<size_t N>
class NthCartesianProduct {
  public:
    template<typename T, size_t X>
    static std::vector<std::array<T, N>>
    of(const std::array<T, X>& A)
    {
    	// Returned value
    	std::vector<std::array<T, N>> AtoN(intpow<X, N>());

	    // Get result of current problem
        auto AtoNIt = AtoN.begin();
	    for(const std::array<T,N-1>& An_1 : NthCartesianProduct<N-1>::of(A)) {
	        for(size_t i = 0; i < X; i++, AtoNIt++) {
                for(size_t j = 0; j < N - 1; j++) (*AtoNIt)[j] = An_1[j];
                (*AtoNIt)[N-1] = A[i];
	        }
	    }

    	return AtoN;
    }

    // Trivial case for X.
    template<typename T>
    static std::vector<std::array<T, N>>
    of(const std::array<T, 0>& A)
    {
    	return {};
    }
};

template<>
class NthCartesianProduct<0> {
  public:
    template<typename T, size_t X>
    static std::vector<std::array<T, 0>>
    of(const std::array<T, X>& A)
    {
        return {{}};
    }
};

void test_cartesian_product();

#endif
