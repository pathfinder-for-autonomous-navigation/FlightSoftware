#ifndef CARTESIAN_PRODUCT_HPP_
#define CARTESIAN_PRODUCT_HPP_

#include <array>
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
// Usage: A^N = NthCartesianProduct<T, |A|, N>::of(A);
//
template<typename T, size_t X, size_t N>
class NthCartesianProduct {
  public:
    static std::array<std::array<T, N>, intpow<X, N>()>
    of(const std::array<T, X>& A)
    {
    	// Returned value
    	std::array<std::array<T, N>, intpow<X, N>()> AtoN {};

    	// Get result of subproblem
	    auto AtoN_1 = NthCartesianProduct<T, X, N - 1>::of(A);

	    // Get result of current problem
	    for(size_t i = 0; i < AtoN_1.size(); i++) {
	        for(size_t j = 0; j < X; j++) {
	            for(size_t k = 0; k < N - 1; k++) {
	                AtoN[i * X + j][k] = AtoN_1[i][k];
	            }
	            AtoN[i * X + j][N - 1] = A[j];
	        }
	    }

    	return AtoN;
    }
};

// Recursive base cases.
template<typename T, size_t N>
class NthCartesianProduct<T, 0, N> {
  public:
    static std::array<std::array<T, N>, intpow<0,N>()>
    of(const std::array<T, 0>& A)
    { 
        return {};
    }
};
template<typename T, size_t X>
class NthCartesianProduct<T, X, 0> {
  public:
    static std::array<std::array<T, 0>, intpow<X,0>()>
    of(const std::array<T, X>& A)
    {
        return {{}};
    }
};

void test_cartesian_product();

#endif
