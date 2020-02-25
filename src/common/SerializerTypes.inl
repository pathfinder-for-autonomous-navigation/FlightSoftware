#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "GPSTime.hpp"
#include "Serializer.hpp"
#include "types.hpp"

#include <lin.hpp> // for norm

/**
 * @brief Specialization of Serializer for booleans.
 */
template <>
class Serializer<bool> : public SerializerBase<bool> {
  public:
    constexpr static size_t bool_sz = 1;
    constexpr static size_t print_sz = 6; // "false" has length 5. +1 for null terminator

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
        if (resolution == 0)
            this->serialized_val.set_int(0);  // Prevent divide-by-zero error
        else
            this->serialized_val.set_int((src_copy - this->_min) / resolution);
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
    constexpr static size_t print_size = 5; // -2^3 - 1 has 4 characters. +1 for NULL

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
    constexpr static size_t print_size = 4; // 2^8 - 1 has 10 characters. +1 for NULL

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
    constexpr static size_t print_size = 11; // 2^32 - 1 has 10 characters. +1 for NULL

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
    constexpr static size_t temp_sz = 30; // Size of a temperature field
    constexpr static size_t print_size = 12; // -2^31 - 1 has 11 characters. +1 for NULL

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
  static_assert(std::is_same<T, float>::value || 
                std::is_same<T, double>::value,
                "Must use double or float type when constructing a float-double serializer.");

  protected:
    template<typename U, size_t N, size_t qsz, size_t vcsz, size_t qcsz>
    friend class VectorSerializer;

    constexpr static size_t print_size = 14; // 6 digits before and after the decimal point, and a NULL character.

    FloatDoubleSerializer(T min, T max, size_t compressed_size)
        : SerializerBase<T>(min, max, compressed_size, print_size)
    {
        assert(min <= max);
        if (min > max) this->_min = max;
    }

  public:
    void serialize(const T& src) override {
        const unsigned int num_intervals = (0b1 << this->serialized_val.size()) - 1;

        T src_copy = src;
        if (src_copy > this->_max) src_copy = this->_max;
        if (src_copy < this->_min) src_copy = this->_min;

        T resolution = 0;
        if (num_intervals > 0) resolution = (this->_max - this->_min) / num_intervals;

        const unsigned int result_int = static_cast<unsigned int>((src_copy - this->_min) / resolution);
        this->serialized_val.set_int(result_int);
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
        const unsigned int num_intervals = (0b1 << this->serialized_val.size()) - 1;

        const unsigned int f_bits = this->serialized_val.to_uint();
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
 * @brief Base class for float/double vector/quaternion specializations of serializer.
 * 
 * This class uses the method of "smallest component" serialization.
 * 
 * For vectors:
 * - For a vector v, it collects the magnitude and serializes it separately--this is
 *   what the min, max, and size values are for. 
 * - Since the unit vector has a magnitude of size 1, we only need the two smallest components
 *   to specify the direction of the unit vector. We allow two bits to store the index of
 *   the largest component in the unit vector, and then we serialize the two smallest components
 *   in the bounds +/- sqrt(2) and with bitsize 9.
 * 
 * For quaternions:
 * - Since the quaternion has a magnitude of size 1, we only need the three smallest components
 *   to specify the direction of the unit vector. We allow two bits to store the index of
 *   the largest component in the quaternion, and then we serialize the three smallest components
 *   in the bounds +/- sqrt(2)/2 and with bitsize 9.
 * 
 */
template <typename T,
          size_t N,
          size_t quat_sz,
          size_t vec_component_sz,
          size_t quat_component_sz>
class VectorSerializer : public SerializerBase<std::array<T, N>> {

  static_assert(std::is_same<T, float>::value || std::is_same<T, double>::value,
      "Vector serializers can only be constructed for floats or doubles.");
  static_assert(N == 3 || N == 4,
      "Serializers for float arrays can only be used for arrays of size 3 or 4.");

  private:
    void construct_vector_serializer(T min, T max, T size) {
        size_t magnitude_bitsize;
        if (N == 3) magnitude_bitsize = size - vec_min_sz;
        else magnitude_bitsize = quat_magnitude_sz;
        magnitude_serializer = std::make_unique<Serializer<T>>(min, max, magnitude_bitsize);

        max_component.resize(2);

        //needs to be intialized or else compiler warning-error
        max_component.set_int(0);

        sign_of_max_comp.resize(1);

        size_t component_sz = 0;
        if (N == 3) { component_sz = vec_component_sz; }
        else { component_sz = quat_component_sz; }

        for (size_t i = 0; i < N - 1; i++) {
            component_scaled_values[i].resize(component_sz);
            vector_element_serializers[i] = std::make_unique<Serializer<T>>(
                // components must be able to be negative, thus range from -sqrt(2)/2 to sqrt(2)/2
                -sqrtf(2.0f) / 2, sqrtf(2.0f) / 2 , component_sz);

        }
    }

    /**
     * @brief Do the vector-specific parts of copying one serializer from another. We need
     * this in order to prevent duplication of code across the asisgnment operator and the
     * copy constructor.
     * 
     * @param other Serializer to copy from.
     */
    void copy_vector_values(const Serializer<std::array<T, N>>& other) {
        this->magnitude_min = other.magnitude_min;
        this->magnitude_max = other.magnitude_max;
        this->magnitude_serializer = std::make_unique<Serializer<T>>(*other.magnitude_serializer);
        for(size_t i = 0; i < vector_element_serializers.size(); i++) {
            this->vector_element_serializers[i] =
                std::make_unique<Serializer<T>>(*other.vector_element_serializers[i]);
        }
        this->max_component = other.max_component;
        this->component_scaled_values = other.component_scaled_values;
        this->sign_of_max_comp = other.sign_of_max_comp;
    }

  protected:
    /**
     * We need these variables, since we don't want to use the base-class provided vectors for
     * storing the minimum and maximum magnitude. We also need a dummy value to feed into the
     * base class, as well.
     */
    T magnitude_min;
    T magnitude_max;
    static std::array<T, N> dummy_vector;

    /**
     * @brief Serializer for vector magnitude.
     */
    std::shared_ptr<Serializer<T>> magnitude_serializer;

    /**
     * @brief Serializer for vector components.
     */
    std::array<std::shared_ptr<Serializer<T>>, N - 1> vector_element_serializers;

    /**
     * @brief Bit array that stores which element of the vector is maximal.
     */
    bit_array max_component;

    /**
     * @brief Bit array that stores sign of max. component of a vector
     * 
     * Only has bitsize() == 1;
     */
    bit_array sign_of_max_comp;

    /**
     * @brief Bit arrays that store the scaled-down representations of each vector component.
     */
    std::array<bit_array, N - 1> component_scaled_values;

    constexpr static size_t quat_magnitude_sz = (quat_sz - 3 * quat_component_sz);
    constexpr static size_t vec_min_sz = 2 + 2 * vec_component_sz + 1; // + 1 for the sign serializer

    constexpr static size_t print_size = 13 * N + (N - 1) + 1; // 13 characters per value in the array,
                                                               // N - 1 commas, 1 null character

    /**
     * @brief Construct a new Serializer object.
     *
     * @param min Minimum magnitude of float/double vector
     * @param max Maximum value of float/double vector
     * @param size Number of bits to compress the vector into. If this is less
     *             than the minimum possible size for float/double vectors, construction will fail.
     */
    VectorSerializer(T min, T max, T size)
        : SerializerBase<std::array<T, N>>(dummy_vector, dummy_vector, size, print_size),
          magnitude_min(min),
          magnitude_max(max)
    {
        this->_min[0] = min;
        this->_max[0] = max;
        construct_vector_serializer(min, max, size);
    }

    /**
     * @brief Default constructor, appropriate for quaternions.
     */
    VectorSerializer() : SerializerBase<std::array<T, N>>(dummy_vector, dummy_vector, quat_sz, print_size),
        magnitude_min(0),
        magnitude_max(1)
    {
        static_assert(N == 4, "Default constructor may only be used for quaternions.");
        construct_vector_serializer(0, 1, quat_sz);
    }

    VectorSerializer<T, N, quat_sz, vec_component_sz, quat_component_sz>& 
    operator=(const VectorSerializer<T, N, quat_sz, vec_component_sz, quat_component_sz>& other) {
        SerializerBase<std::array<T, N>>::operator=(other);
        copy_vector_values(other);
        return *this;
    }

  public:
    void serialize(const std::array<T, N>& src) override {
        bit_array::iterator serialized_position = this->serialized_val.begin();

        lin::Vector<T, N> normd; // short for normalized; normd = normalized
        std::array<T, N> src_norm(src);

        // src should already be normalized, but normalize again just in case
        // this block of code normalizes iff N == 4
        // otherwise, when N == 3, src_norm is not normalized
        if(N == 4) {
            normd = {src[0], src[1], src[2], src[3]};
        
            normd = normd / lin::norm(normd);
            
            for(unsigned int i = 0; i<N; i++){
                src_norm[i] = normd(i);
            }
        }
        
        // Get and store index of maximum-valued component
        std::array<T, N> v_mags;
        T max_element_mag = 0;
        unsigned int max_component_idx = 0;
        for (size_t i = 0; i < N; i++) {
            v_mags[i] = std::abs(src_norm[i]);
            if (max_element_mag < v_mags[i]) {
                max_element_mag = v_mags[i];
                max_component_idx = i;
            }
        }

        max_component.set_int(max_component_idx);
        std::copy(max_component.begin(), max_component.end(), serialized_position);
        std::advance(serialized_position, max_component.size());
        
        // Store serialized magnitude
        T mag = 0.0f;
        if (N == 4)
            mag = 1.0f;
        else {
            for (size_t i = 0; i < 3; i++) {
                mag += src_norm[i] * src_norm[i];
            }
        }

        // actually turns mag^2 to mag
        mag = std::sqrt(mag);

        // only serialize the magnitude if N == 3
        if(N == 3){
            magnitude_serializer->serialize(mag);
            std::copy(magnitude_serializer->get_bit_array().begin(), 
                    magnitude_serializer->get_bit_array().end(),
                    serialized_position);
            std::advance(serialized_position, magnitude_serializer->bitsize());
        }

        // will only ever be true for N == 4 and max_comp being negative
        bool flip_vals = false;
        if(N == 4 && src[max_component_idx] < 0)
            flip_vals = true;

        // Store serialized non-maximal components
        size_t component_number = 0;
        for (size_t i = 0; i < N; i++) {
            if (i != max_component_idx) {
                T element_scaled = src_norm[i] / mag;

                // the negative of a quaternion is the same quaternion
                // thus to indicate sign of the largest component, negate all other components
                if(flip_vals)
                    element_scaled *= -1;

                vector_element_serializers[component_number]->serialize(element_scaled);

                std::copy(vector_element_serializers[component_number]->get_bit_array().begin(),
                          vector_element_serializers[component_number]->get_bit_array().end(),
                          serialized_position);
                std::advance(serialized_position, vector_element_serializers[component_number]->bitsize());
                component_number++;
            }
        }

        // if it's a vector, you must use an additional bit 
        // to determine the sign of the largest component
        if(N == 3){
            if(src_norm[max_component_idx] < 0){
                sign_of_max_comp.set_int(1);
            }
            std::copy(sign_of_max_comp.begin(), sign_of_max_comp.end(), serialized_position);
            std::advance(serialized_position, sign_of_max_comp.size());
        }        
    }

    bool deserialize(const char* val, std::array<T, N>* dest) override {
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
        if (i < N) return false;
        *dest = temp_dest;
        serialize(*dest);
        return true;
    }

    void deserialize(std::array<T, N>* dest) const override {
        // a counter to keep track of the current serialized val index to process
        unsigned int idx_pointer = 0;

        // read which component index is highest into a bitset for later use
        std::bitset<2> max_comp_bitset(0);
        for(size_t i = 0; i<max_component.size(); i++){
            max_comp_bitset[i] = this->serialized_val[idx_pointer];
            //max_component[i] = this->serialized_val[idx_pointer];
            idx_pointer++;
        }
        
        // if there's a magnitude serializer, save data into the member variable
        if(N == 3){
            bit_array& local_arr_ref = const_cast <bit_array&>(magnitude_serializer->get_bit_array());
            for(size_t i = 0; i < magnitude_serializer->bitsize(); i++){
                local_arr_ref[i] = this->serialized_val[idx_pointer];
                idx_pointer++;
            }
            magnitude_serializer->set_bit_array(local_arr_ref);
        }

        // loop through each serializer
        for(unsigned int i = 0; i<(N-1); i++){
            bit_array& local_arr_ref = const_cast <bit_array&>(vector_element_serializers[i]->get_bit_array());
            // loop through each bit belonging to the serializer
            for(size_t j = 0; j < vector_element_serializers[i]->bitsize(); j++){       
                local_arr_ref[j] = this->serialized_val[idx_pointer];
                idx_pointer++;
            }

            vector_element_serializers[i]->set_bit_array(local_arr_ref);
        }

        // set to true if N == 3 and max. comp is negative
        bool max_comp_is_negative = false;
        if (N == 3){
            max_comp_is_negative = this->serialized_val[idx_pointer];
            idx_pointer++;
        }    

        T magnitude = 0.0;
        if(N == 3)
            magnitude_serializer->deserialize(&magnitude);
        else
            magnitude = 1.0;

        // completely bypass member bit_array max_component;
        unsigned int deser_max_idx = max_comp_bitset.to_ulong();

        (*dest)[deser_max_idx] = 1.0;
        int j = 0;  // Index of current component being processed
        for (size_t i = 0; i < N; i++) {
            if (i != deser_max_idx) {
                vector_element_serializers[j]->deserialize(&(*dest)[i]);

                // at this point in the code, (*dest)[i] contains x/r, y/r
                // assuming z is largest magnitude

                (*dest)[deser_max_idx] -= (*dest)[i] * (*dest)[i]; // subtract off; z^2/r^2 = 1 - x^2/r^2 ...
                j++;
            }
        }
        (*dest)[deser_max_idx] = sqrt((*dest)[deser_max_idx]);

        // multiply by magnitude, only actually does anything for N == 3
        for (size_t i = 0; i < N; i++) (*dest)[i] *= magnitude;

        // flip sign of max comp of vector if necessary
        if(N == 3){
            if(max_comp_is_negative == 1){
                (*dest)[deser_max_idx] *= -1;
            }
        }
    }

    const char* print(const std::array<T, N>& src) const override {
        size_t str_idx = 0;
        for (size_t i = 0; i < N; i++) {
            str_idx += sprintf(this->printed_val + str_idx, "%6.6f,", src[i]);
        }
        return this->printed_val;
    }
};

template<typename T,
         size_t N,
         size_t qsz,
         size_t vcsz,
         size_t qcsz>
std::array<T, N> VectorSerializer<T, N, qsz, vcsz, qcsz>::dummy_vector = {};

namespace SerializerConstants {
    /**
     * @brief Specialization of Serializer for float vectors and quaternions.
     */
    constexpr size_t fqsz = 29; // Compressed size for a float-based quaternion
    constexpr size_t fvcsz = 9; // Compressed size for a normalized component of a float-based vector
    constexpr size_t fqcsz = 9; // Compressed size for a normalized component of a float-based quaternion

    // TODO FIX BITSIZE
    constexpr size_t min_fvsz = 2 + fvcsz * 2; // Minimum size for a float vector.
}

template <size_t N>
class Serializer<std::array<float, N>> : public VectorSerializer<float, N,
                                                    SerializerConstants::fqsz,
                                                    SerializerConstants::fvcsz,
                                                    SerializerConstants::fqcsz> {
  public:
    /**
     * @brief Construct a new serializer object for float vectors.
     * 
     * @param min Minimum magnitude of vector.
     * @param max Maximum magnitude of vector.
     * @param size Minimum compressed bitsize of vector. Should be larger than the minimum vector size.
     */
    Serializer<std::array<float, N>>(float min, float max, size_t size)
        : VectorSerializer<float, N, SerializerConstants::fqsz,
                                     SerializerConstants::fvcsz,
                                     SerializerConstants::fqcsz>(min, max, size)
    {
        assert(size > SerializerConstants::min_fvsz);
    }

    /**
     * @brief Construct a new serializer for float quaternions.
     */
    Serializer<std::array<float, N>>() : VectorSerializer<float, N, 
                                            SerializerConstants::fqsz,
                                            SerializerConstants::fvcsz,
                                            SerializerConstants::fqcsz>()
    {
        static_assert(N == 4, "A default constructor can only be used for a quaternion.");                                        
    }
};

namespace SerializerConstants {
    /**
     * @brief Specialization of Serializer for double vectors and quaternions.
     */
    constexpr size_t dqsz = 29; // Compressed size for a double-based quaternion
    constexpr size_t dvcsz = 9; // Compressed size for a normalized component of a double-based vector
    constexpr size_t dqcsz = 9; // Compressed size for a normalized component of a double-based quaternion

    constexpr size_t min_dvsz = 2 + dvcsz * 2; // Minimum size for a double vector.
}

template <size_t N>
class Serializer<std::array<double, N>> : public VectorSerializer<double, N, 
                                                    SerializerConstants::dqsz, 
                                                    SerializerConstants::dvcsz,
                                                    SerializerConstants::dqcsz> {
  public:
    /**
     * @brief Construct a new serializer object for double vectors.
     * 
     * @param min Minimum magnitude of vector.
     * @param max Maximum magnitude of vector.
     * @param size Minimum compressed bitsize of vector. Should be larger than the minimum vector size.
     */
    Serializer<std::array<double, N>>(double min, double max, double size)
        : VectorSerializer<double, N, SerializerConstants::dqsz,
                                      SerializerConstants::dvcsz,
                                      SerializerConstants::dqcsz>(min, max, size)
    {
        assert(size > SerializerConstants::min_dvsz);
    }

    /**
     * @brief Construct a new serializer for double quaternions.
     */
    Serializer<std::array<double, N>>() : VectorSerializer<double, N, 
                                            SerializerConstants::dqsz,
                                            SerializerConstants::dvcsz,
                                            SerializerConstants::dqcsz>()
    {
        static_assert(N == 4, "A default constructor can only be used for a quaternion.");
    }
};

/**
 * @brief Specialization of Serializer for GPS time.
 */
template <>
class Serializer<gps_time_t> : public SerializerBase<gps_time_t> {
  public:
    constexpr static size_t gps_time_sz = 68;
    static const gps_time_t dummy_gpstime;

    constexpr static size_t print_size = 25; // wn: 5, tow: 10, ns: 7, 2 commas, 1 NULL character. 

    Serializer()
        : SerializerBase<gps_time_t>(dummy_gpstime, dummy_gpstime, gps_time_sz, print_size)
    {}

    void serialize(const gps_time_t& src) override {
        if (!src.is_set) {
            serialized_val[0] = false;
            return;
        }
        serialized_val[0] = true;
        std::bitset<16> wn((unsigned short int)src.wn);
        std::bitset<32> tow(src.tow);
        std::bitset<20> ns(src.ns);
        for (size_t i = 0; i < wn.size(); i++) serialized_val[i + 1] = wn[i];
        for (size_t i = 0; i < tow.size(); i++) serialized_val[i + 1 + wn.size()] = tow[i];
        for (size_t i = 0; i < ns.size(); i++)
            serialized_val[i + 1 + wn.size() + tow.size()] = ns[i];
    }

    bool deserialize(const char* val, gps_time_t* dest) override {
        size_t num_values_found = sscanf(val, "%hu,%d,%d", &(dest->wn),
                                         &(dest->tow), &(dest->ns));
        if (num_values_found != 3) return false;

        serialize(*dest);
        return true;
    }

    void deserialize(gps_time_t* dest) const override {
        if (!serialized_val[0]) {
            dest->is_set = false;
            return;
        }
        else {
            dest->is_set = true;
        }
        std::bitset<16> wn;
        std::bitset<32> tow;
        std::bitset<20> ns;
        for (size_t i = 0; i < 16; i++) wn.set(i, serialized_val[i + 1]);
        for (size_t i = 0; i < 32; i++) tow.set(i, serialized_val[wn.size() + i + 1]);
        for (size_t i = 0; i < 20; i++) ns.set(i, serialized_val[wn.size() + tow.size() + i + 1]);
        dest->wn = (unsigned int)wn.to_ulong();
        dest->tow = (unsigned int)tow.to_ulong();
        dest->ns = (unsigned int)ns.to_ulong();
    }

    const char* print(const gps_time_t& src) const override {
        sprintf(this->printed_val, "%hu,%d,%d", src.wn, src.tow, src.ns);
        return this->printed_val;
    }
};
