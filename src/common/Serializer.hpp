/**
 * @file Serializer.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_
#include <memory>
#include <string>
#include <cstring>
#include "GPSTime.hpp"
#include "fixed_array.hpp"

/**
 * @brief Set of constants defining fixed and minimum bitsizes for compressed
 * objects.
 */
class SerializerType {
   public:
    static const std::vector<std::string> serializable_types;
};

/**
 * @brief Base class that manages memory for a serializer. Specifically, it ensures that the
 * bit array used to store the results of serialization is allocated at most once.
 *
 * This class shouldn't be instantiated. Instead, instantiate specializations of Serializer, which
 * is derived from this base class.
 *
 * @tparam T Type of stored value.
 */
template <typename T>
class SerializerBase : public SerializerType {
   private:
     /**
      * @brief Length of string returned by print().
      */
     size_t strlength;
   protected:
    /**
     * @brief Minima and maxima used for fixed-point compression of objects.
     *
     * @{
     */
    T _min;
    T _max;
    /**
     * @}
     */

    /**
     * @brief Container for bit array containing serialized value of object.
     */
    bit_array serialized_val;

    /**
     * @brief Container for printed value of serialized object.
     */
    char* printed_val = nullptr;

    /**
     * @brief Argumented constructor. This is protected to prevent construction of this
     * implementation-less base class.
     *
     * If the provided size is less than 0 (which could happen if you're constructing the magnitude
     * serializer for a vector serializer), return from the constructor and do not resize the
     * serialized bit array.
     */
    SerializerBase(T min, T max, size_t compressed_size, size_t _strlength) :
        strlength(_strlength),
        _min(min),
        _max(max),
        serialized_val()
    {
        if (static_cast<signed int>(compressed_size) < 0) return;
        serialized_val.resize(compressed_size);

        this->printed_val = new char[strlength];
        memset(printed_val, '\0', strlength);
    }

    /**
     * @brief Copy constructor.
     */
    SerializerBase(const SerializerBase& other) {
        *this = other;
    }

    /**
     * @brief Copy assignment operator.
     */
    SerializerBase& operator=(const SerializerBase& rhs) {
        _min = rhs._min;
        _max = rhs._max;
        serialized_val.resize(rhs.serialized_val.size());
        serialized_val = rhs.serialized_val;

        delete[] printed_val;
        strlength = rhs.strlength;
        printed_val = new char[strlength];
        memcpy(printed_val, rhs.printed_val, strlength);

        return *this;
    }

    /**
     * @brief Move constructor.
     */
    SerializerBase(SerializerBase&& other) {
        *this = other;
    }

    /**
     * @brief Move assignment operator.
     */
    SerializerBase& operator=(SerializerBase&& rhs) {
        _min = rhs._min;
        _max = rhs._max;
        serialized_val.resize(rhs.serialized_val.size());
        serialized_val = std::move(rhs.serialized_val);

        delete[] printed_val;
        strlength = rhs.strlength;
        printed_val = rhs.printed_val;
        rhs.printed_val = nullptr;
        return *this;
    }

   public:
    /**
     * @brief Serializes a given object and stores the compressed object
     * into the member bitset.
     *
     * @param src
     *
     * @return True if serialization succeeded, false if serializer was
     *         uninitialized.
     */
    virtual void serialize(const T& src) = 0;

    /**
     * @brief Deserializes the contents stored in the provided character array
     * and stores the result in the provided object pointer. Also updates
     * the internally stored bitset.
     *
     * @param val  String containing value to process.
     * @param dest Value stored in string will be stored into here.
     *
     * @return If deserialization from the provided string was successful.
     */
    virtual bool deserialize(const char* val, T* dest) = 0;

    /**
     * @brief Deserializes the bit array and stores the result in the provided
     * object pointer.
     *
     * @param dest Where to store the result of the deserialization.
     *
     * @return True if serialization succeeded, false if serializer was
     *         uninitialized.
     */
    virtual void deserialize(T* dest) const = 0;

    /**
     * @brief Outputs a string representation of the source value into
     * the given destination string.
     *
     * In order to enable good use of memory, the maximum length of the string
     * representation can be found in the member variable "str_len".
     *
     * @param src  Source value
     *
     * @return C-style string containing printed value.
     */
    virtual const char* print(const T& src) const = 0;

    T min() const { return _min; }
    T max() const { return _max; }

    /**
     * @brief Get the stored bit array containing the serialized value.
     *
     * @return const bit_array& Reference to internal bit array.
     */
    const bit_array& get_bit_array() const { return serialized_val; }

    /**
     * @brief Return size of bit array held by this serializer.
     *
     * @return size_t Size of bit array.
     */
    size_t bitsize() const { return serialized_val.size(); }

    /**
     * @brief Set the internally stored serialized value. Do nothing if the source bit array does
     * not have the same size as the internally stored bit array.
     *
     * @param src Bit array to copy.
     */
    void set_bit_array(const bit_array& src) {
        if (src.size() != serialized_val.size()) return;
        serialized_val = src;
    }

    /**
     * @brief Destructor.
     */
    virtual ~SerializerBase();
};

template <typename T>
SerializerBase<T>::~SerializerBase() {
    delete[] printed_val;
}

/**
 * @brief Public facing, constructible version of SerializerBase.
 *
 * @tparam T Type of value to serialize.
 */
template <typename T> class Serializer;

#include "SerializerTypes.inl"

#endif
