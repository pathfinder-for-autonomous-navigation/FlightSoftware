#ifndef FIXED_ARRAY_HPP_
#define FIXED_ARRAY_HPP_

#include <algorithm>
#include <array>
#include <bitset>
#include <cmath>
#include <vector>

/**
 * Behaves like an array (i.e. the size cannot be modified), but the size is specified upon
 * construction rather than as a template argument. So the memory for the array is allocated, not
 * created statically.
 *
 * This is useful for serializers, because we want to ensure that bitsets for serialization cannot
 * keep allocating and re-allocating memory, but we don't want to clunkily carry around the bitset's
 * size as a template argument for the serializer (like we would have to if we used std::bitset).
 *
 * Notice that this class is not publicly constructible; we rely on fixed_array below to provide a
 * public interface. This is so that we can share implementation of a fixed array between generic
 * fixed arrays and the specialized boolean fixed array (below), which is the bitset used in
 * serialization/deserialization.
 *
 * @tparam T Type held within array.
 */
template <typename T>
class fixed_array_base : public std::vector<T> {
   protected:
    /**
     * @brief Default constructor.
     */
    fixed_array_base() {}

    /**
     * @brief Construct a new fixed array object
     *
     * @param size (Unchanged) size of the object.
     */
    explicit fixed_array_base(const size_t size) : std::vector<T>(size) {}

    /**
     * @brief Explicit copy constructor for a fixed array. Constructs the fixed array to be of the
     * same size as the argument.
     *
     * @tparam sz
     * @param set
     */
    fixed_array_base(const fixed_array_base<T>& arr) : fixed_array_base<T>(arr.size()) {
        *this = arr;
    }

    /**
     * @brief Explicit copy constructor for a fixed array from an STL vector. Constructs the fixed
     * array to be of the same size as the argument.
     *
     * @tparam sz
     * @param set
     */
    fixed_array_base(const std::vector<T>& arr) : fixed_array_base<T>(arr.size()) {
        *this = arr;
    }

    /**
     * @brief Allows assignment-by-value using another fixed array. If the arrays are not of the
     * same length, nothing happens.
     *
     * @tparam sz
     * @param set
     * @return fixed_array&
     */
    fixed_array_base& operator=(const fixed_array_base<T>& arr) {
        if (arr.size() != this->size()) return *this;
        for (size_t i = 0; i < arr.size(); i++) (*this)[i] = arr[i];
        return *this;
    }

    /**
     * @brief Allows assignment-by-value using an STL vector. If the arrays are not of the
     * same length, nothing happens.
     *
     * @tparam sz
     * @param set
     * @return fixed_array&
     */
    fixed_array_base& operator=(const std::vector<T>& arr) {
        if (arr.size() != this->size()) return *this;
        for (size_t i = 0; i < arr.size(); i++) (*this)[i] = arr[i];
        return *this;
    }

   private:
    /*
     * Make all length-changing functions unavailable to class's users and children, except for
     * resize(). Resize may be used in the constructors of functions to resize the array.
     */
    using std::vector<T>::operator=;
    using std::vector<T>::clear;
    using std::vector<T>::insert;
    using std::vector<T>::emplace;
    using std::vector<T>::erase;
    using std::vector<T>::push_back;
    using std::vector<T>::emplace_back;
    using std::vector<T>::pop_back;
    using std::vector<T>::swap;
};

/**
 * @brief Instantiable version of fixed array.
 *
 * @tparam T Type held by array.
 */
template <typename T>
class fixed_array : public fixed_array_base<T> {
   public:
    /**
     * Same constructors as fixed_array_base.
     */
    explicit fixed_array() : fixed_array_base<T>() {}
    explicit fixed_array(const size_t size) : fixed_array_base<T>(size) {}
    fixed_array(const fixed_array<T>& arr) : fixed_array_base<T>(arr) {}
    fixed_array(const std::vector<T>& arr) : fixed_array_base<T>(arr) {}
};

/**
 * @brief Acts like a stripped-down bitset.
 */
template <>
class fixed_array<bool> : public fixed_array_base<bool> {
   public:
    /**
     * Same constructors as fixed_array_base, but made public.
     */
    fixed_array() : fixed_array_base<bool>() {}
    explicit fixed_array(const size_t size) : fixed_array_base<bool>(size) {}
    fixed_array(const fixed_array<bool>& arr) : fixed_array_base<bool>(arr) {}
    fixed_array(const std::vector<bool>& arr) : fixed_array_base<bool>(arr) {}

    /**
     * @brief Explicit copy constructor for a bitset. Constructs the fixed array to be of the same
     * size as the bitset.
     *
     * @tparam sz
     * @param set
     */
    template <size_t sz>
    explicit fixed_array(const std::bitset<sz>& set) : fixed_array_base<bool>(sz) {
        *this = set;
    }

    /**
     * @brief Allows assignment-by-value using a bitset. Does not copy the bitset if it is a
     * different size than the fixed array.
     *
     * @tparam sz
     * @param set
     * @return fixed_array&
     */
    template <size_t sz>
    fixed_array& operator=(const std::bitset<sz>& set) {
        if (sz != size()) return *this;
        for (size_t i = 0; i < sz; i++) (*this)[i] = set[i];
        return *this;
    }

    /**
     * @brief Sets fixed array to an integer value, if there is enough space in the bitset to
     * do so. If there is not, the old value is preserved.
     *
     * @param val Value to initialize bitset to.
     * @return Whether or not it was possible to store the integer into this bitset.
     */
    bool set_int(unsigned int val) {
        size_t val_num_bits = 32;
        for (size_t i = 0; i < 32; i++) {
            if (pow(2, i) > val) {
                val_num_bits = i;
                break;
            }
        }
        if (val_num_bits > size()) return false;

        for (size_t i = 0; i < size(); i++) {
            (*this)[i] = 0;
        }

        int i = 0;
        while (val > 0) {
            (*this)[i] = val % 2;
            val /= 2;
            i++;
        }

        return true;
    }

    /**
     * @brief Converts bitset to integer.
     *
     * @return unsigned int
     */
    unsigned long to_uint() const { return static_cast<unsigned int>(to_ulong()); }

    /**
     * @brief Converts bitset to integer.
     *
     * @return unsigned long
     */
    unsigned long to_ulong() const { return static_cast<unsigned long>(to_ullong()); }

    /**
     * @brief Converts bitset to integer.
     *
     * @return unsigned long
     */
    unsigned long long to_ullong() const {
        unsigned long long val = 0;

        unsigned long long exp = 1;
        for (size_t i = 0; i < size(); i++) {
            val += (*this)[i] * exp;
            exp *= 2;
        }

        return val;
    }

    // Modifies a bit in character 'n' at the position 'p' to the value 'b'
    // The position is zero-indexed.
    // https://www.geeksforgeeks.org/modify-bit-given-position/
    static unsigned char modify_bit(unsigned char n, unsigned char p, bool b) {
        return (n & ~(1 << p)) | ((b << p) & (1 << p));
    }

    /**
     * @brief Writes bitset to the provided array of bytes.
     *
     * @param str Byte array to modify.
     * @param offset The bit offset at which to begin writing
     *               in the bitset, relative to the beginning
     *               of the byte array.
     * @param start Parameter specifying where to start in the
     *              bit array.
     * @param end   Parameter specifying where to end in the bit array.
     */
    void to_string(char*& str, size_t offset, size_t start, size_t end) const {
        for(size_t i = start; i < end; i++, offset++) {
            char& c = str[(offset / 8)];
            c = modify_bit(c, 7 - (offset % 8), (*this)[i]);
        }
    }
    void to_string(char*& str, size_t offset) const {
        to_string(str, offset, 0, size());
    }
};

/**
 * @brief Convenience definition for fixed array of bits.
 */
typedef fixed_array<bool> bit_array;

#endif
