#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "GPSTime.hpp"
#include "Serializer.hpp"
#include "types.hpp"
#include "constant_tracker.hpp"
#include <lin.hpp>

/**
 * @brief Specialization of Serializer for booleans.
 */
template <>
class Serializer<bool> : public SerializerBase<bool> {
  public:
    TRACKED_CONSTANT_SC(size_t, bool_sz, 1);
    TRACKED_CONSTANT_SC(size_t, print_sz, 6); // "false" has length 5. +1 for null terminator

    Serializer() : SerializerBase<bool>(false, true, 1, print_sz) {}

    Serializer(const Serializer<bool>& other) : SerializerBase<bool>(other) {}

    void serialize(const bool& src) override { serialized_val[0] = src; }

    bool deserialize(const char* val, bool* dest) override {
        if (strcmp(val, "false") == 0)
            *dest = false;
        else if (strcmp(val, "true") == 0)
            *dest = true;
        else
            return false;
        return true;
    }

    void deserialize(bool* dest) const override { *dest = serialized_val[0]; }

    const char* print(const bool& src) const override {
        if (src)
            strcpy(this->printed_val, "true");
        else
            strcpy(this->printed_val, "false");
        return this->printed_val;
    }
};

/**
 * @brief Specialization of Serializer for unsigned ints.
 */
template <typename T>
class IntegerSerializer : public SerializerBase<T> {
  static_assert(std::is_same<T, unsigned int>::value || 
                std::is_same<T, signed int>::value ||
                std::is_same<T, unsigned char>::value ||
                std::is_same<T, signed char>::value,
                "Must use integer or char type when constructing an integer serializer.");

  protected:
    static constexpr unsigned int log2i(unsigned int n) {
        unsigned int i = 0;
        while(n > 0) {
            n = n >> 1;
            i++;
        }
        return i;
    }

    IntegerSerializer(T min, T max, size_t compressed_size, size_t print_size)
        : SerializerBase<T>(min, max, std::min(compressed_size, 8*sizeof(T)), print_size)
    {
        assert(min <= max);
    }

    IntegerSerializer(T min, T max, size_t print_size)
        : SerializerBase<T>(min, max, log2i(max - min), print_size)
    {
        assert(min <= max);
    }

  public:
    unsigned int _resolution() const {
        unsigned int range = this->_max - this->_min;
        const unsigned int num_bits = this->serialized_val.size();
        
        unsigned int num_intervals;
        if (num_bits < 32) num_intervals = (1 << num_bits) - 1;
        else num_intervals = 4294967295; // 2^32 - 1
        if (num_intervals == 0) return 0;  // No resolution available.

        T interval_per_bit = range / num_intervals;
        if (interval_per_bit * num_intervals < range) interval_per_bit += 1;
        return interval_per_bit;
    }

    void serialize(const T& src) override {
        T src_copy = src;
        if (src_copy > this->_max) src_copy = this->_max;
        if (src_copy < this->_min) src_copy = this->_min;

        unsigned int resolution = _resolution();
        if (resolution != 0)
            this->serialized_val.set_ullong((src_copy - this->_min) / resolution);
        else
            // Can't divide by zero!
            this->serialized_val.set_ullong(0);
    }

    bool deserialize(const char* val, T* dest) override {
        int scanned_val = 0;
        size_t num_values_found = sscanf(val, "%d", &scanned_val);
        if (num_values_found != 1) return false;
        *dest = static_cast<T>(scanned_val);

        // Store result into current bitset
        serialize(*dest);
        return true;
    }

    void deserialize(T* dest) const override {
        *dest = this->_min + this->serialized_val.to_ulong() * _resolution();
    }

    const char* print(const T& src) const override {
        if (std::is_same<T, unsigned int>::value || std::is_same<T, unsigned char>::value)
            sprintf(this->printed_val, "%u", src);
        else
            sprintf(this->printed_val, "%d", src);
        return this->printed_val;
    }
};

/**
 * @brief Specialization of Serializer for signed characters.
 */
template <>
class Serializer<signed char> : public IntegerSerializer<signed char> {
  public:
    TRACKED_CONSTANT_SC(size_t, print_size, 5); // -2^3 - 1 has 4 characters. +1 for NULL

    Serializer() : IntegerSerializer<signed char>(-128, 127, print_size) {}

    Serializer(signed char min, signed char max, size_t compressed_size)
        : IntegerSerializer<signed char>(min, max, compressed_size, print_size) {}

    Serializer(signed char min, signed char max)
        : IntegerSerializer<signed char>(min, max, print_size) {}
};

/**
 * @brief Specialization of Serializer for unsigned characters.
 */
template <>
class Serializer<unsigned char> : public IntegerSerializer<unsigned char> {
  public:
    TRACKED_CONSTANT_SC(size_t, print_size, 4); // 2^8 - 1 has 10 characters. +1 for NULL

    Serializer() : IntegerSerializer<unsigned char>(0, 255, print_size) {}

    Serializer(unsigned char max) : IntegerSerializer<unsigned char>(0, max, print_size) {}

    Serializer(unsigned char min, unsigned char max, size_t compressed_size)
        : IntegerSerializer<unsigned char>(min, max, compressed_size, print_size) {}

    Serializer(unsigned char min, unsigned char max)
        : IntegerSerializer<unsigned char>(min, max, print_size) {}
};

/**
 * @brief Specialization of Serializer for unsigned integers.
 */
template <>
class Serializer<unsigned int> : public IntegerSerializer<unsigned int> {
  public:
    TRACKED_CONSTANT_SC(size_t, print_size, 11); // 2^32 - 1 has 10 characters. +1 for NULL

    Serializer() : IntegerSerializer<unsigned int>(0, 4294967295, print_size) {}

    Serializer(unsigned int max) : IntegerSerializer<unsigned int>(0, max, print_size) {}

    Serializer(unsigned int min, unsigned int max, size_t compressed_size)
        : IntegerSerializer<unsigned int>(min, max, compressed_size, print_size) {}

    Serializer(unsigned int min, unsigned int max)
        : IntegerSerializer<unsigned int>(min, max, print_size) {}
};

/**
 * @brief Specialization of Serializer for signed integers.
 */
template <>
class Serializer<signed int> : public IntegerSerializer<signed int> {
  public:
    TRACKED_CONSTANT_SC(size_t, temp_sz, 30); // Size of a temperature field
    TRACKED_CONSTANT_SC(size_t, print_size, 12); // -2^31 - 1 has 11 characters. +1 for NULL

    Serializer() : IntegerSerializer<signed int>(-2147483648, 2147483647, print_size) {}

    Serializer(signed int min, signed int max, size_t compressed_size)
        : IntegerSerializer<signed int>(min, max, compressed_size, print_size) {}

    Serializer(signed int min, signed int max)
        : IntegerSerializer<signed int>(min, max, print_size) {}
};

/**
 * @brief Specialization of Serializer for floats and doubles.
 */
template <typename T>
class FloatDoubleSerializer : public SerializerBase<T> {
  static_assert(std::is_floating_point<T>::value,
                "Must use double or float type when constructing a float-double serializer.");

  protected:

    TRACKED_CONSTANT_SC(size_t, print_size, 14); // 6 digits before and after the decimal point, and a NULL character.

    FloatDoubleSerializer(T min, T max, size_t compressed_size)
        : SerializerBase<T>(min, max, compressed_size, print_size)
    {
        assert(min <= max);
        if (min > max) this->_min = max;
    }

  public:
    void serialize(const T& src) override {
        const unsigned long long num_intervals = std::pow(2, this->serialized_val.size()) - 1;

        T src_copy = src;
        if (src_copy > this->_max) src_copy = this->_max;
        if (src_copy < this->_min) src_copy = this->_min;

        T resolution = 0;
        if (num_intervals > 0) resolution = (this->_max - this->_min) / num_intervals;

        const unsigned long long result_int = (src_copy - this->_min) / resolution;

        this->serialized_val.set_ullong(result_int);
    }

    bool deserialize(const char* val, T* dest) override {
        static char const* format_specifier;
        static constexpr char const* float_format_specifier = "%f";
        static constexpr char const* double_format_specifier = "%lf";
        if (std::is_same<T, float>::value) format_specifier = float_format_specifier;
        else format_specifier = double_format_specifier;

        size_t num_values_found = sscanf(val, format_specifier, dest);
        if (num_values_found != 1) return false;

        // Store result into current bitset
        serialize(*dest);
        return true;
    }

    void deserialize(T* dest) const override {
        const unsigned long long num_intervals = std::pow(2, this->serialized_val.size()) - 1;

        const unsigned long long f_bits = this->serialized_val.to_ullong();
        T resolution;
        if (num_intervals > 0)
            resolution = (this->_max - this->_min) / num_intervals;
        else
            resolution = 0;

        *dest = this->_min + resolution * f_bits;
    }

    const char* print(const T& src) const override {
        sprintf(this->printed_val, "%6.6f", src);
        return this->printed_val;
    }
};

/**
 * @brief Specialization of Serializer for floats.
 */
template <>
class Serializer<float> : public FloatDoubleSerializer<float> {
  public:
    Serializer(float min, float max, float size)
        : FloatDoubleSerializer<float>(min, max, size) {}
};

/**
 * @brief Specialization of Serializer for doubles.
 */
template <>
class Serializer<double> : public FloatDoubleSerializer<double> {
  public:
    Serializer(double min, double max, double size)
        : FloatDoubleSerializer<double>(min, max, size) {}
};

/**
 * Common functions used by vector and quaternion serializers.
 */
namespace VectorSerializerFns
{
    /**
     * Print vector to a string.
     */
    template<typename T, size_t N>
    const char* vector_print(const std::array<T, N>& src, char* dest) {
        static_assert(std::is_floating_point<T>::value, "");

        size_t str_idx = 0;
        for (size_t i = 0; i < N; i++) {
            str_idx += sprintf(dest + str_idx, "%6.6f,", src[i]);
        }
        return dest;
    }

    /**
     * Return the number of characters required to print an N-dimensional vector
     * to a string.
     */
    constexpr size_t vector_print_size(int N)
    {
        return 13 * N + (N - 1) + 1; // 13 characters per value in the array, N-1 commas, 1 null character
    }

    /**
     * Convert a string representation of a vector to an std::array.
     */
    template<typename T, size_t N>
    bool deserialize_vector_str(const char* val, std::array<T, N>* dest) {
        static_assert(std::is_floating_point<T>::value, "");

        size_t i = 0;
        std::array<T, N> temp_dest;
        char temp_val[150];
        memcpy(temp_val, val, sizeof(temp_val));

        char *tok = strtok(temp_val, ",");
        while (tok != NULL) {
            if (std::is_same<T, float>::value) {
                temp_dest[i++] = static_cast<T>(strtof(tok, NULL));
            }
            else {
                temp_dest[i++] = static_cast<T>(strtod(tok, NULL));
            }
            tok = strtok(NULL, ",");
        }
        if (i != N) return false;
        *dest = temp_dest;
        return true;
    }
};

/**
 * Vector serializer. Works best with vectors whose values are confined
 * to a shell subspace.
 */
template <typename T>
class VectorSerializer : public SerializerBase<std::array<T, 3>> {
  static_assert(std::is_floating_point<T>::value,
    "Vector serializers can only be constructed for floats or doubles.");

  public:
    TRACKED_CONSTANT_SC(T, pi, 3.141592653589793);

  protected:
    /**
     * @brief Serializer for vector components
     * 
     * These are only used as temporary objects within the serialize and
     * deserialize objects, but to prevent de-allocation and re-allocation
     * of memory these objects are made member objects.
     */
    mutable Serializer<T> magnitude_serializer;
    mutable Serializer<T> theta_serializer; // Using physics coordinates: this is angle w.r.t. z axis
    mutable Serializer<T> phi_serializer; // Using physics coordinates: this is xy angle

    /*
     * Let precision equal p. The meaning of the number is that
     * the vector should be specified to within the Euclidean
     * distance given by dx = (max - min) / 2^p.
     * 
     * We can approximate
     * dx = sqrt((r dtheta)^2 + (r sin(theta) dphi)^2 + dr^2)
     *    = max (dtheta + dphi) + dr
     * 
     * We want to choose the angle and magnitude serializer bounds so that
     * they use a small number of bits but satisfy this precision
     * constraint. The solution to this puzzle is just to distribute
     * the errors equally among the three serializers: choose bitsizes
     * b1, b2 such that
     * 
     * (max - min) / 2^(b1) < dr < 1/3 dx
     * max / 2^(b2) < max dtheta, max dphi < 1/3 dx
     * 
     * These three functions compute b1, b2, and b1 + b2 + b2 (the total 
     * bitsize of the array.)
     */
    static constexpr size_t b1(size_t precision)
    {
        // 3 (max - min) / 2^(b1) < dx, dx = (max - min) / 2^p
        // => b1 > log_2(3) + p
        return 2 + precision;
    }
    static constexpr size_t b2(T min, T max, size_t precision)
    {
        // 3 max / 2^(b2) < dx, dx = (max - min) / 2^p
        // => b2 > log_2(3 * max / (max - min)) + p
        return std::ceil(std::log(3 * max / (max - min)) / std::log(2)) + precision;
    }
    static constexpr size_t total_size(T min, T max, size_t precision)
    {
        // 1 extra bit for ascertaining the xy-quadrant of the vector.
        return 1 + b1(precision) + 2 * b2(min, max, precision);
    }

  public:
    VectorSerializer(T min, T max, size_t precision) :
        SerializerBase<std::array<T, 3>>(
            std::array<T, 3>(),
            std::array<T, 3>(),
            total_size(min, max, precision),
            VectorSerializerFns::vector_print_size(3)
        ),
        magnitude_serializer(min, max, b1(precision)),
        theta_serializer(0, pi, b2(min, max, precision)),
        phi_serializer(-pi/2, pi/2, b2(min, max, precision))
    {
        // Required for the logarithm in the computations of b1 and b2 to be defined.
        assert(max > 0); assert(max > min);

        // This line exists so that TelemetryInfoGenerator is able to determine the
        // minimum and maximum radius.
        this->_min[0] = min;
        this->_max[0] = max;
    }

  public:
    void serialize(const std::array<T,3>& src) override {
        lin::Vector<T, 3> normalized_vec {src[0], src[1], src[2]};
        T magnitude = lin::norm(normalized_vec);
        normalized_vec = normalized_vec / magnitude;

        T theta = std::acos(normalized_vec(2));
        T phi = std::atan(normalized_vec(1) / normalized_vec(0));
        bool xsign = normalized_vec(0) > 0;

        magnitude_serializer.serialize(magnitude);
        theta_serializer.serialize(theta);
        phi_serializer.serialize(phi);

        bit_array::iterator serialized_position = this->serialized_val.begin();
        serialized_position[0] = xsign;
        serialized_position++;
        auto copy_bits = [&serialized_position](Serializer<T>& sr) {
            auto const& bit_array = sr.get_bit_array();
            std::copy(bit_array.begin(), bit_array.end(), serialized_position);
            std::advance(serialized_position, bit_array.size());
        };
        copy_bits(magnitude_serializer);
        copy_bits(theta_serializer);
        copy_bits(phi_serializer);
    }

    bool deserialize(const char* val, std::array<T, 3>* dest) override {
        bool success = VectorSerializerFns::deserialize_vector_str<T, 3>(val, dest);
        if (success) serialize(*dest);
        return success;
    }

    void deserialize(std::array<T, 3>* dest) const override {
        bit_array::const_iterator serialized_position = this->serialized_val.begin();
        bool xsign = serialized_position[0];
        serialized_position++;
        auto copy_bits = [&serialized_position](Serializer<T>& sr) {
            auto& bit_array = sr.get_bit_array();
            for(size_t i = 0; i < bit_array.size(); i++)
            {
                bit_array[i] = *serialized_position;
                serialized_position++;
            }
        };
        copy_bits(magnitude_serializer);
        copy_bits(theta_serializer);
        copy_bits(phi_serializer);

        T magnitude, theta, phi;
        magnitude_serializer.deserialize(&magnitude);
        theta_serializer.deserialize(&theta);
        phi_serializer.deserialize(&phi);
        
        int xfactor = xsign ? 1 : -1;
        (*dest)[0] = xfactor * magnitude * std::sin(theta) * std::cos(phi);
        (*dest)[1] = xfactor * magnitude * std::sin(theta) * std::sin(phi);
        (*dest)[2] = magnitude * std::cos(theta);
    }

    const char* print(const std::array<T, 3>& src) const override {
        return VectorSerializerFns::vector_print<T, 3>(src, this->printed_val);
    }
};

/**
 * @brief Base class for float/double quaternion specializations of serializer.
 * 
 * This class uses the serialization method of "smallest three" for quaternions.
 * - Since the quaternion has a magnitude of size 1, we only need the three smallest components
 *   to specify the direction of the unit vector. We allow two bits to store the index of
 *   the largest component in the quaternion, and then we serialize the three smallest components
 *   in the bounds +/- 1 and with enough bitsize for angular resolution to be OK.
 */
template <typename T>
class QuaternionSerializer : public SerializerBase<std::array<T, 4>> {
  static_assert(std::is_floating_point<T>::value,
      "Quaternion serializers can only be constructed for floats or doubles.");

  protected:
    /**
     * @brief Serializer for quaternion components.
     * 
     * These are only used as temporary objects within the serialize and
     * deserialize objects, but to prevent de-allocation and re-allocation
     * of memory these objects are made member objects.
     */
    std::array<std::shared_ptr<Serializer<T>>, 3> quaternion_element_serializers;

    /**
     * Bitsize with which to serialize an individual component of the quaternion.
     * The number 10 is determined experimentally.
     */
    static constexpr size_t quat_component_sz() { return 10; }

    /**
     * @brief Default constructor, appropriate for quaternions.
     */
    QuaternionSerializer() :
        SerializerBase<std::array<T, 4>>(
            std::array<T, 4>(),
            std::array<T, 4>(),
            quat_component_sz() * 3 + 2,
        VectorSerializerFns::vector_print_size(4))
    {
        for (size_t i = 0; i < 3; i++) {
            quaternion_element_serializers[i] = std::make_unique<Serializer<T>>(-1, 1, quat_component_sz());
        }
    }

  public:
    void serialize(const std::array<T, 4>& src) override {
        lin::Vector<T, 4> normalized_quat {src[0], src[1], src[2], src[3]};
        normalized_quat = normalized_quat / lin::norm(normalized_quat);
        std::array<T, 4> src_normalized(src);
        for(unsigned int i = 0; i<4; i++) src_normalized[i] = normalized_quat(i);
        
        // Get and store index of maximum-valued component
        T max_element_mag = 0;
        unsigned int max_component_idx = 0;
        for (size_t i = 0; i < 4; i++) {
            T component_abs = std::abs(src_normalized[i]);
            if (max_element_mag < component_abs) {
                max_element_mag = component_abs;
                max_component_idx = i;
            }
        }

        std::bitset<2> max_component(max_component_idx);

        bit_array::iterator serialized_position = this->serialized_val.begin();
        this->serialized_val[0] = max_component[0];
        this->serialized_val[1] = max_component[1];
        std::advance(serialized_position, 2);

        // Store serialized non-maximal components
        size_t component_number = 0;
        for (size_t i = 0; i < 4; i++) {
            if (i != max_component_idx) {
                // The negative of a quaternion is the same quaternion.
                // Thus to indicate sign of the largest component, negate all other components.
                if(src[max_component_idx] < 0) src_normalized[i] *= -1;

                auto& element_sr = quaternion_element_serializers[component_number];

                element_sr->serialize(src_normalized[i]);

                std::copy(element_sr->get_bit_array().begin(),
                          element_sr->get_bit_array().end(),
                          serialized_position);
                std::advance(serialized_position, element_sr->bitsize());
                component_number++;
            }
        }      
    }

    bool deserialize(const char* val, std::array<T, 4>* dest) override {
        bool success = VectorSerializerFns::deserialize_vector_str<T, 4>(val, dest);
        if (success) serialize(*dest);
        return success;
    }

    void deserialize(std::array<T, 4>* dest) const override {
        bit_array::const_iterator serialized_position = this->serialized_val.begin();

        // read which component index is highest into a bitset for later use
        std::bitset<2> max_component;
        for(size_t i = 0; i<max_component.size(); i++){
            max_component[i] = *serialized_position;
            serialized_position++;
        }
        unsigned int max_idx = max_component.to_ulong();

        if (std::is_same<T, float>::value) (*dest)[max_idx] = 1.0f;
        else (*dest)[max_idx] = 1.0;

        // loop through each serializer
        for(unsigned int i = 0; i<3; i++){
            auto& element_sr = quaternion_element_serializers[i];
            auto& element_bitarray = element_sr->get_bit_array();

            // loop through each bit belonging to the serializer
            for(size_t j = 0; j < element_sr->bitsize(); j++){       
                element_bitarray[j] = *serialized_position;
                serialized_position++;
            }

            element_sr->set_bit_array(element_bitarray);
        }

        size_t j = 0; // Index of current component being processed
        for (size_t i = 0; i < 4; i++) {
            if (i != max_idx) {
                quaternion_element_serializers[j]->deserialize(&(*dest)[i]);
                (*dest)[max_idx] -= (*dest)[i] * (*dest)[i];
                j++;
            }
        }
        (*dest)[max_idx] = sqrt((*dest)[max_idx]);
    }

    const char* print(const std::array<T, 4>& src) const override {
        return VectorSerializerFns::vector_print<T, 4>(src, this->printed_val);
    }
};

template<>
class Serializer<std::array<float, 3>> : public VectorSerializer<float> {
  public:
    using VectorSerializer<float>::VectorSerializer;
};

template<>
class Serializer<std::array<double, 3>> : public VectorSerializer<double> {
  public:
    using VectorSerializer<double>::VectorSerializer;
};

template<>
class Serializer<std::array<float, 4>> :
public QuaternionSerializer<float>
{
  public:
    using QuaternionSerializer<float>::QuaternionSerializer;
};

template<>
class Serializer<std::array<double, 4>> :
public QuaternionSerializer<double>
{
  public:
    using QuaternionSerializer<double>::QuaternionSerializer;
};

/**
 * Helper functions for lin vector serialization.
 */
template<typename T, size_t N>
class LinVectorSerializer :  public SerializerBase<lin::Vector<T, N>> {
  static_assert(N == 3 || N == 4, "Serializers for float arrays can only be used for arrays of size 3 or 4.");
  static_assert(std::is_floating_point<T>::value, "Serializers are only defined for float or double-valued tuples.");

  protected:
    std::shared_ptr<Serializer<std::array<T, N>>> _arr_sr;

    LinVectorSerializer(...) : 
        SerializerBase<lin::Vector<T, N>>(
            lin::zeros<T, N, 1>(),
            lin::zeros<T, N, 1>(),
        0, 0)
    {}

    /**
     * Store min/max/bitsize information for use by the
     * TelemtryInfoGenerator.
     */
    void set_telemetry_info() {
        if (N == 3)
        {
            this->_min(0) = _arr_sr->min()[0];
            this->_max(0) = _arr_sr->max()[0];
        }
        this->serialized_val.resize(_arr_sr->bitsize());
    }

  public:
    void serialize(const lin::Vector<T, N>& src) override {
        std::array<T, N> src_cpy;
        for(unsigned int i = 0; i < N; i++) src_cpy[i] = src(i);
        _arr_sr->serialize(src_cpy);
    }

    bool deserialize(const char* val, lin::Vector<T, N>* dest) override {
        std::array<T, N> dest_cpy;
        bool ret = _arr_sr->deserialize(val, &dest_cpy);
        if (!ret) return false;
        for(unsigned int i = 0; i < N; i++) (*dest)(i) = dest_cpy[i];
        return true;
    }

    void deserialize(lin::Vector<T, N>* dest) const override {
        std::array<T, N> dest_cpy;
        _arr_sr->deserialize(&dest_cpy);
        for(unsigned int i = 0; i < N; i++) (*dest)(i) = dest_cpy[i];
    }

    const char* print(const lin::Vector<T, N>& src) const override {
        std::array<T, N> src_cpy;
        for(unsigned int i = 0; i < N; i++) src_cpy[i] = src(i);
        return _arr_sr->print(src_cpy);
    }

    const bit_array& get_bit_array() const { return _arr_sr->get_bit_array(); }
    bit_array& get_bit_array() { return _arr_sr->get_bit_array(); }

    void set_bit_array(const bit_array& src) {
        _arr_sr->set_bit_array(src);
    }
};

template<typename T>
class Serializer<lin::Vector<T, 4>> : public LinVectorSerializer<T, 4>
{
  public:
    Serializer() : LinVectorSerializer<T, 4>()
    {
        this->_arr_sr = std::make_shared<Serializer<std::array<T,4>>>();
        this->set_telemetry_info();
    }
};

template<typename T>
class Serializer<lin::Vector<T, 3>> : public LinVectorSerializer<T, 3>
{
  public:
    Serializer(T min, T max, size_t bitsize) :
        LinVectorSerializer<T, 3>(min, max, bitsize)
    {
        this->_arr_sr = std::make_shared<Serializer<std::array<T,3>>>(min, max, bitsize);
        this->set_telemetry_info();
    }
};

/**
 * @brief Specialization of Serializer for GPS time.
 */
template <>
class Serializer<gps_time_t> : public SerializerBase<gps_time_t> {
  public:
    TRACKED_CONSTANT_SC(size_t, print_size, 25); // wn: 5, tow: 10, ns: 7, 2 commas, 1 NULL character. 

    mutable Serializer<unsigned int> wn_sz;
    mutable Serializer<unsigned int> tow_sz;
    mutable Serializer<signed int> ns_sz;

    Serializer()
        : SerializerBase<gps_time_t>(gps_time_t(), gps_time_t(), 1, print_size),
          wn_sz(2000, 3000),
          tow_sz(NANOSECONDS_IN_WEEK / 1'000'000),
          ns_sz(-1'000'000, 1'000'000)
    {
        unsigned int bitsize = 1 + wn_sz.bitsize() + tow_sz.bitsize() + ns_sz.bitsize();
        this->serialized_val.resize(bitsize);
    }

    void serialize(const gps_time_t& src) override {
        bit_array::iterator it = serialized_val.begin();

        if (src.is_set) { *it = true; }
        else { *it = false; return; }
        it += 1;

        wn_sz.serialize(src.wn);
        tow_sz.serialize(src.tow);
        ns_sz.serialize(src.ns);

        const bit_array& wn_bits = wn_sz.get_bit_array();
        const bit_array& tow_bits = tow_sz.get_bit_array();
        const bit_array& ns_bits = ns_sz.get_bit_array();
        std::copy(wn_bits.begin(), wn_bits.end(), it); it += wn_sz.bitsize();
        std::copy(tow_bits.begin(), tow_bits.end(), it); it += tow_sz.bitsize();
        std::copy(ns_bits.begin(), ns_bits.end(), it);
    }

    bool deserialize(const char* val, gps_time_t* dest) override {
        size_t num_values_found = sscanf(val, "%hu,%d,%d", &(dest->wn),
                                         &(dest->tow), &(dest->ns));
        if (num_values_found != 3) return false;

        serialize(*dest);
        return true;
    }

    void deserialize(gps_time_t* dest) const override {
        bit_array::const_iterator it = serialized_val.begin();
        if (*it) { dest->is_set = true; }
        else { dest->is_set = false; return; }
        it += 1;

        bit_array& wn_bits = wn_sz.get_bit_array();
        bit_array& tow_bits = tow_sz.get_bit_array();
        bit_array& ns_bits = ns_sz.get_bit_array();
        wn_bits.assign(it, it + wn_sz.bitsize()); it += wn_sz.bitsize();
        tow_bits.assign(it, it + tow_sz.bitsize()); it += tow_sz.bitsize();
        ns_bits.assign(it, it + ns_sz.bitsize());

        unsigned int wn;
        wn_sz.deserialize(&wn); dest->wn = static_cast<unsigned short>(wn);
        tow_sz.deserialize(&(dest->tow));
        ns_sz.deserialize(&(dest->ns));
    }

    const char* print(const gps_time_t& src) const override {
        printf("%hu, %d, %d", src.wn, src.tow, src.ns);
        sprintf(this->printed_val, "%hu,%d,%d", src.wn, src.tow, src.ns);
        return this->printed_val;
    }
};
