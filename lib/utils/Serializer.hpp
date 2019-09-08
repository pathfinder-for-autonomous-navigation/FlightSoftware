/**
 * @file Serializer.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_
#include <memory>
#include <string>
#include "GPSTime.hpp"
#include "fixed_array.hpp"

class SerializerConstants {
   public:
    /**
     * @brief Set of constants defining fixed and minimum bitsizes for compressed
     * objects.
     */
    constexpr static size_t bool_sz = 1;
    constexpr static size_t f_quat_sz = 30;
    constexpr static size_t d_quat_sz = 30;
    constexpr static size_t gps_time_sz = 30;
    constexpr static size_t temp_sz = 30;

    constexpr static size_t f_vec_min_sz = 30;
    constexpr static size_t f_vec_quat_component_sz = 9;
    constexpr static size_t d_vec_min_sz = 30;
    constexpr static size_t d_vec_quat_component_sz = 9;

    static const std::vector<std::string> serializable_types;

    // Dummy GPS time used by GPS time serializers
    static const gps_time_t dummy_gpstime;
};

/**
 * @brief Base class that manages memory for a serializer. Specifically, it ensures that the
 * bit array used to store the results of serialization is allocated at most once.
 *
 * @tparam T Type of stored value.
 */
template <typename T>
class SerializerBase : protected SerializerConstants {
   public:
    /**
     * @brief String length that is necessary to print the value
     * of the stored contents.
     */
    static constexpr size_t strlen = 0;

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
    char* printed_val;
    bool is_printed_val_allocated = false;

    /**
     * @brief Argumented constructor. This is protected to prevent construction of this
     * implementation-less base class.
     *
     * If the provided size is less than 0 (which could happen if you're constructing the magnitude
     * serializer for a vector serializer), return from the constructor and do not resize the
     * serialized bit array.
     */
    SerializerBase(T min, T max, size_t compressed_size) : _min(min), _max(max) {
        if (static_cast<int>(compressed_size) < 0) return;
        serialized_val.resize(compressed_size);

        printed_val = new char[this->strlen];
        is_printed_val_allocated = true;
    }

    /**
     * @brief Copy constructor.
     */
    SerializerBase(const SerializerBase& cpy) {
        _min = cpy._min;
        _max = cpy._max;
        serialized_val.resize(cpy.serialized_val.size());

        if (is_printed_val_allocated) delete[] printed_val;
        is_printed_val_allocated = false;
        printed_val = new char[cpy.strlen];
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
    virtual bool deserialize(const char* val, std::shared_ptr<T>& dest) = 0;

    /**
     * @brief Deserializes the bit array and stores the result in the provided
     * object pointer.
     *
     * @param dest Where to store the result of the deserialization.
     *
     * @return True if serialization succeeded, false if serializer was
     *         uninitialized.
     */
    virtual void deserialize(std::shared_ptr<T>& dest) const = 0;

    /**
     * @brief Outputs a string representation of the source value into
     * the given destination string. 
     * 
     * In order to enable good use of memory, the maximum length of the string
     * representation can be found in the member variable "strlen".
     *
     * @param src  Source value
     *
     * @return C-style string containing printed value.
     */
    virtual const char* print(const T& src) const = 0;

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
     * @brief Set the internally stored serialized value. Do nothing if the source bit arary does
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
    ~SerializerBase() { delete[] printed_val; }
};

template <typename T>
class Serializer : public SerializerBase<T> {
    Serializer(T min, T max, size_t compressed_size)
        : SerializerBase<T>(min, max, compressed_size) {}
};

#include "SerializerTypes.inl"

#endif
