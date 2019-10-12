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
     * @brief Create a readable state field for booleans, GPS times, and float/double quaternions.
     * 
     * @param name Name of field.
     * @return Pointer to field that was created.
     */
    template<typename T>
    std::shared_ptr<ReadableStateField<T>> create_readable_field(const std::string name) {
        /**
         * Due to the definitions of the serializer constructors, this function can
         * only be used to create state fields of specific types.
         */
        static_assert(std::is_same<T, bool> ||
                      std::is_same<T, gps_time_t> ||
                      std::is_same<T, f_quat_t> ||
                      std::is_same<T, d_quat_t>, 
            "This function can only be used to create boolean, GPS-time, float-quaternion"
            "or double-quaternion state fields.");

        Serializer<T> field_sr;
        ReadableStateField<T> field_f(name, field_sr);
        std::shared_ptr<ReadableStateField<T>> field_ptr(
            std::shared_ptr<ReadableStateField<T>>{}, &field_f);
        add_readable(field_ptr);
        return field_ptr;
    }

    /**
     * @brief Create a writable state field for booleans, GPS times, and float/double quaternions.
     * 
     * @param name Name of field.
     * @return Pointer to field that was created.
     */
    template<typename T>
    std::shared_ptr<WritableStateField<T>> create_writable_field(const std::string name) {
        /**
         * Due to the definitions of the serializer constructors, this function can
         * only be used to create state fields of specific types.
         */
        static_assert(std::is_same<T, bool> ||
                      std::is_same<T, gps_time_t> ||
                      std::is_same<T, f_quat_t> ||
                      std::is_same<T, d_quat_t>,
            "This function can only be used to create boolean, GPS-time, float-quaternion"
            "or double-quaternion state fields.");

        Serializer<T> field_sr;
        WritableStateField<T> field_f(name, field_sr);
        std::shared_ptr<WritableStateField<T>> field_ptr(
            std::shared_ptr<WritableStateField<T>>{}, &field_f);
        add_writable(field_ptr);
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
    std::shared_ptr<ReadableStateField<T>> create_readable_field(const std::string name, 
        T min, T max, size_t bitsize)
    {
        /**
         * Due to the definitions of the serializer constructors, this function can
         * only be used to create state fields of specific types.
         */
        static_assert(std::is_same<T, signed int> ||
            std::is_same<T, unsigned int> ||
            std::is_same<T, float> ||
            std::is_same<T, double>,
        "This function can only be used to create signed int, unsigned int, float or double"
        "state fields.");

        Serializer<T> field_sr(min, max, bitsize);
        ReadableStateField<T> field_f(name, field_sr);
        std::shared_ptr<ReadableStateField<T>> field_ptr(
            std::shared_ptr<ReadableStateField<T>>{}, &field_f);
        add_readable(field_ptr);
        return field_ptr;
    }

    /**
     * @brief Create a writable field for signed/unsigned ints, and floats/doubles and add
     * it to the registry.
     * 
     * @tparam T Type of object.
     * @param name Name of field to create.
     * @param min Minimum value of field.
     * @param max Maximum value of field.
     * @param bitsize Number of bits with which to represent field internally.
     * @return Pointer to field that was created.
     */
    template<typename T>
    std::shared_ptr<WritableStateField<T>> create_writable_field(const std::string name, 
        T min, T max, size_t bitsize)
    {
        /**
         * Due to the definitions of the serializer constructors, this function can
         * only be used to create state fields of specific types.
         */
        static_assert(std::is_same<T, signed int> ||
            std::is_same<T, unsigned int> ||
            std::is_same<T, float> ||
            std::is_same<T, double>,
        "This function can only be used to create signed int, unsigned int, float or double"
        "state fields.");

        Serializer<T> field_sr(min, max, bitsize);
        WritableStateField<T> field_f(name, field_sr);
        std::shared_ptr<WritableStateField<T>> field_ptr(
            std::shared_ptr<WritableStateField<T>>{}, &field_f);
        add_writable(field_ptr);
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
    std::shared_ptr<ReadableStateField<T>> create_readable_vec_field(const std::string name, 
        std::array<T, 3>)
    {
        /**
         * Due to the definitions of the serializer constructors, this function can
         * only be used to create state fields of specific types.
         */
        static_assert(std::is_same<T, float> ||
                      std::is_same<T, double>,
        "This function can only be used to create float- or double-vector state fields.");

        Serializer<T> field_sr(min, max, bitsize);
        ReadableStateField<T> field_f(name, field_sr);
        std::shared_ptr<ReadableStateField<T>> field_ptr(
            std::shared_ptr<ReadableStateField<T>>{}, &field_f);
        add_readable(field_ptr);
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
    std::shared_ptr<WritableStateField<T>> create_writable_vec_field(const std::string name, 
        std::array<T, 3>)
    {
        /**
         * Due to the definitions of the serializer constructors, this function can
         * only be used to create state fields of specific types.
         */
        static_assert(std::is_same<T, float> ||
                      std::is_same<T, double>,
        "This function can only be used to create float- or double-vector state fields.");

        Serializer<T> field_sr(min, max, bitsize);
        WritableStateField<T> field_f(name, field_sr);
        std::shared_ptr<WritableStateField<T>> field_ptr(
            std::shared_ptr<WritableStateField<T>>{}, &field_f);
        add_writable(field_ptr);
        return field_ptr;
    }
};

#endif
