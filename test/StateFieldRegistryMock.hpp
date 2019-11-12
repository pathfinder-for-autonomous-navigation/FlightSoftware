#ifndef STATE_FIELD_REGISTRY_MOCK_HPP_
#define STATE_FIELD_REGISTRY_MOCK_HPP_

#include <StateFieldRegistry.hpp>

/**
 * @brief Provides a state field registry with additional field creation
 * utilities for use in unit testing.
 */
class StateFieldRegistryMock : public StateFieldRegistry {
  public:
    StateFieldRegistryMock() : StateFieldRegistry() {}

    /**
     * @brief Finds a internal state field of the given name.
     */
    template<typename T>
    std::shared_ptr<InternalStateField<T>> find_internal_field_t(const std::string& name) {
        return std::static_pointer_cast<InternalStateField<T>>(find_internal_field(name));
    }

    /**
     * @brief Finds a readable state field of the given name.
     */
    template<typename T>
    std::shared_ptr<ReadableStateField<T>> find_readable_field_t(const std::string& name) {
        return std::static_pointer_cast<ReadableStateField<T>>(find_readable_field(name));
    }

    /**
     * @brief Finds a writable state field of the given name.
     */
    template<typename T>
    std::shared_ptr<WritableStateField<T>> find_writable_field_t(const std::string& name) {
        return std::static_pointer_cast<WritableStateField<T>>(find_writable_field(name));
    }

    /**
     * @brief Create a readable state field for booleans, GPS times, and float/double quaternions.
     * 
     * @param name Name of field.
     * @return Pointer to field that was created.
     */
    template<typename T>
    std::shared_ptr<ReadableStateField<T>> create_readable_field(const std::string& name) {
        static_assert(std::is_same<T, bool>::value ||
                      std::is_same<T, gps_time_t>::value ||
                      std::is_same<T, f_quat_t>::value ||
                      std::is_same<T, d_quat_t>::value ||
                      std::is_same<T, unsigned int>::value ||
                      std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr;
        auto field_ptr = std::make_shared<ReadableStateField<T>>(name, field_sr);
        add_readable_field(field_ptr);
        return field_ptr;
    }

    /**
     * @brief Create a writable state field for booleans, GPS times, and float/double quaternions.
     * 
     * @param name Name of field.
     * @return Pointer to field that was created.
     */
    template<typename T>
    std::shared_ptr<WritableStateField<T>> create_writable_field(const std::string& name) {
        static_assert(std::is_same<T, bool>::value ||
                      std::is_same<T, gps_time_t>::value ||
                      std::is_same<T, f_quat_t>::value ||
                      std::is_same<T, d_quat_t>::value ||
                      std::is_same<T, unsigned int>::value ||
                      std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr;
        auto field_ptr = std::make_shared<WritableStateField<T>>(name, field_sr);
        add_writable_field(field_ptr);
        return field_ptr;
    }

    /**
     * @brief Create a readable field for signed/unsigned ints, and floats/doubles and
     * add it to the registry.
     * 
     * @tparam T Type of object.
     * @param name Name of field to create.
     * @param min Minimum value of field.
     * @param max Maximum value of field.
     * @param bitsize Number of bits with which to represent field internally.
     * @return Pointer to field that was created.
     */
    template<typename T>
    std::shared_ptr<ReadableStateField<T>> create_readable_field(const std::string& name, 
        T min, T max, size_t bitsize)
    {
        static_assert(std::is_same<T, signed int>::value ||
            std::is_same<T, unsigned int>::value ||
            std::is_same<T, signed char>::value ||
            std::is_same<T, unsigned char>::value ||
            std::is_same<T, float>::value ||
            std::is_same<T, double>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr(min, max, bitsize);
        auto field_ptr = std::make_shared<ReadableStateField<T>>(name, field_sr);
        add_readable_field(field_ptr);
        return field_ptr;
    }

    /**
     * @brief Create a writable field for signed/unsigned ints, and floats/doubles and
     * add it to the registry.
     * 
     * @tparam T Type of object.
     * @param name Name of field to create.
     * @param min Minimum value of field.
     * @param max Maximum value of field.
     * @param bitsize Number of bits with which to represent field internally.
     * @return Pointer to field that was created.
     */
    template<typename T>
    std::shared_ptr<WritableStateField<T>> create_writable_field(const std::string& name, 
        T min, T max, size_t bitsize)
    {
        static_assert(std::is_same<T, signed int>::value ||
            std::is_same<T, unsigned int>::value ||
            std::is_same<T, signed char>::value ||
            std::is_same<T, unsigned char>::value ||
            std::is_same<T, float>::value ||
            std::is_same<T, double>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr(min, max, bitsize);
        auto field_ptr = std::make_shared<WritableStateField<T>>(name, field_sr);
        add_writable_field(field_ptr);
        return field_ptr;
    }

    template<typename T>
    std::shared_ptr<ReadableStateField<T>> create_readable_field(const std::string& name, 
        T min, T max)
    {
        static_assert(std::is_same<T, signed int>::value ||
            std::is_same<T, unsigned int>::value ||
            std::is_same<T, signed char>::value ||
            std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr(min, max);
        auto field_ptr = std::make_shared<ReadableStateField<T>>(name, field_sr);
        add_readable_field(field_ptr);
        return field_ptr;
    }

    template<typename T>
    std::shared_ptr<WritableStateField<T>> create_writable_field(const std::string& name, 
        T min, T max)
    {
        static_assert(std::is_same<T, signed int>::value ||
            std::is_same<T, unsigned int>::value ||
            std::is_same<T, signed char>::value ||
            std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr(min, max);
        auto field_ptr = std::make_shared<WritableStateField<T>>(name, field_sr);
        add_writable_field(field_ptr);
        return field_ptr;
    }

    template<typename T>
    std::shared_ptr<ReadableStateField<T>> create_readable_field(const std::string& name, T max)
    {
        static_assert(std::is_same<T, unsigned int>::value ||
                      std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr(max);
        auto field_ptr = std::make_shared<ReadableStateField<T>>(name, field_sr);
        add_readable_field(field_ptr);
        return field_ptr;
    }

    template<typename T>
    std::shared_ptr<WritableStateField<T>> create_writable_field(const std::string& name, T max)
    {
        static_assert(std::is_same<T, unsigned int>::value ||
                      std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr(max);
        auto field_ptr = std::make_shared<WritableStateField<T>>(name, field_sr);
        add_writable_field(field_ptr);
        return field_ptr;
    }

    /**
     * @brief Create a readable field for a float- or double-vector and add it to
     * the registry.
     * 
     * @tparam T Type of object.
     * @param name Name of field to create.
     * @param min Minimum magnitude of vector.
     * @param max Maximum magnitude of vector.
     * @param bitsize Number of bits with which to represent field internally.
     * @return Pointer to field that was created.
     */
    template<typename T>
    std::shared_ptr<ReadableStateField<std::array<T,3>>> create_readable_vector_field(const std::string& name,
        T min, T max, size_t bitsize)
    {
        static_assert(std::is_same<T, float>::value ||
                      std::is_same<T, double>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<std::array<T, 3>> field_sr(min, max, bitsize);
        auto field_ptr = std::make_shared<ReadableStateField<std::array<T, 3>>>(name, field_sr);
        add_readable_field(field_ptr);
        return field_ptr;
    }

    /**
     * @brief Create a writable field for a float- or double-vector and add it to
     * the registry.
     * 
     * @tparam T Type of object.
     * @param name Name of field to create.
     * @param min Minimum magnitude of vector.
     * @param max Maximum magnitude of vector.
     * @param bitsize Number of bits with which to represent field internally.
     * @return Pointer to field that was created.
     */
    template<typename T>
    std::shared_ptr<WritableStateField<std::array<T, 3>>> create_writable_vector_field(const std::string& name,
        T min, T max, size_t bitsize)
    {
        static_assert(std::is_same<T, float>::value ||
                      std::is_same<T, double>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<std::array<T, 3>> field_sr(min, max, bitsize);
        auto field_ptr = std::make_shared<WritableStateField<std::array<T, 3>>>(name, field_sr);
        add_writable_field(field_ptr);
        return field_ptr;
    }
};

#endif
