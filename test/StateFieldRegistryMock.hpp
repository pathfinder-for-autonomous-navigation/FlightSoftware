#ifndef STATE_FIELD_REGISTRY_MOCK_HPP_
#define STATE_FIELD_REGISTRY_MOCK_HPP_

#include <common/StateFieldRegistry.hpp>

#ifdef DESKTOP
    #include <iostream>
#else
    #include <Arduino.h>
#endif

/**
 * @brief Provides a state field registry with additional field creation
 * utilities for use in unit testing.
 */
class StateFieldRegistryMock : public StateFieldRegistry {
  protected:
    template<typename T, unsigned int eeprom_save_period>
    class __field_creator {
      public:
        static std::shared_ptr<ReadableStateField<T>>
        create_readable_field(const std::string& name, const Serializer<T>& sr)
        {
            if (!SerializableStateField<T>::is_eeprom_saveable()) assert(false);
            else return std::make_shared<ReadableStateField<T>>(name, sr, eeprom_save_period);
        }

        static std::shared_ptr<WritableStateField<T>>
        create_writable_field(const std::string& name, const Serializer<T>& sr)
        {
            if (!SerializableStateField<T>::is_eeprom_saveable()) assert(false);
            else return std::make_shared<WritableStateField<T>>(name, sr, eeprom_save_period);
        }
    };

    template<typename T>
    class __field_creator<T, 0> {
      public:
        static std::shared_ptr<ReadableStateField<T>>
        create_readable_field(const std::string& name, const Serializer<T>& sr)
        {
            return std::make_shared<ReadableStateField<T>>(name, sr);
        }

        static std::shared_ptr<WritableStateField<T>>
        create_writable_field(const std::string& name, const Serializer<T>& sr)
        {
            return std::make_shared<WritableStateField<T>>(name, sr);
        }
    };

    void check_field_exists(StateFieldBase* ptr, const std::string& name) {
        if (!ptr) {
            #ifdef DESKTOP
            std::cout << "Could not find field named \"" << name << "\" in registry." << std::endl;
            #else
            Serial.printf("Could not find field named \"%s\" in registry.", name.c_str());
            #endif
            assert(false);
        }
    }

  public:
    /**
     * @brief Finds a internal state field of the given name.
     */
    template<typename T>
    InternalStateField<T>* find_internal_field_t(const std::string& name) {
        auto ptr = static_cast<InternalStateField<T>*>(find_internal_field(name));
        check_field_exists(ptr, name);
        return ptr;
    }

    /**
     * @brief Finds a readable state field of the given name.
     */
    template<typename T>
    ReadableStateField<T>* find_readable_field_t(const std::string& name) {
        auto ptr = static_cast<ReadableStateField<T>*>(find_readable_field(name));
        check_field_exists(ptr, name);
        return ptr;
    }

    /**
     * @brief Finds a writable state field of the given name.
     */
    template<typename T>
    WritableStateField<T>* find_writable_field_t(const std::string& name) {
        auto ptr = static_cast<WritableStateField<T>*>(find_writable_field(name));
        check_field_exists(ptr, name);
        return ptr;
    }

    /**
     * @brief Finds an event of the given name.
     */
    Event* find_event_t(const std::string& name) {
        auto ptr = static_cast<Event*>(find_event(name));
        check_field_exists(ptr, name);
        return ptr;
    }
    
    /**
     * @brief Finds an fault of the given name.
     */
    Fault* find_fault_t(const std::string& name) {
        auto ptr = static_cast<Fault*>(find_fault(name));
        check_field_exists(ptr, name);
        return ptr;
    }

    /**
     * @brief Create an internal state field.
     * 
     * @param name Name of field.
     * @return Pointer to field that was created.
     */
    template<typename T>
    std::shared_ptr<InternalStateField<T>> create_internal_field(const std::string& name) {
        auto field_ptr = std::make_shared<InternalStateField<T>>(name);
        add_internal_field(field_ptr.get());
        created_internal_fields.push_back(field_ptr);
        return field_ptr;
    }

    /**
     * @brief Create a readable state field for booleans, GPS times, and float/double quaternions.
     * 
     * @param name Name of field.
     * @return Pointer to field that was created.
     */
    template<typename T, unsigned int eeprom_save_period = 0>
    std::shared_ptr<ReadableStateField<T>> create_readable_field(const std::string& name)
    {
        static_assert(std::is_same<T, bool>::value ||
                      std::is_same<T, gps_time_t>::value ||
                      std::is_same<T, f_quat_t>::value ||
                      std::is_same<T, d_quat_t>::value ||
                      std::is_same<T, lin::Vector4f>::value ||
                      std::is_same<T, lin::Vector4d>::value ||
                      std::is_same<T, unsigned int>::value ||
                      std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr;
        auto field_ptr = __field_creator<T, eeprom_save_period>::create_readable_field(name, field_sr);
        add_readable_field(field_ptr.get());
        created_readable_fields.push_back(field_ptr);
        return field_ptr;
    }

    /**
     * @brief Create a writable state field for booleans, GPS times, and float/double quaternions.
     * 
     * @param name Name of field.
     * @return Pointer to field that was created.
     */
    template<typename T, unsigned int eeprom_save_period = 0>
    std::shared_ptr<WritableStateField<T>> create_writable_field(const std::string& name)
    {
        static_assert(std::is_same<T, bool>::value ||
                      std::is_same<T, gps_time_t>::value ||
                      std::is_same<T, f_quat_t>::value ||
                      std::is_same<T, d_quat_t>::value ||
                      std::is_same<T, lin::Vector4f>::value ||
                      std::is_same<T, lin::Vector4d>::value ||
                      std::is_same<T, unsigned int>::value ||
                      std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr;
        auto field_ptr = __field_creator<T, eeprom_save_period>::create_writable_field(name, field_sr);
        add_writable_field(field_ptr.get());
        created_writable_fields.push_back(field_ptr);
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
    template<typename T, unsigned int eeprom_save_period = 0>
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
        auto field_ptr = __field_creator<T, eeprom_save_period>::create_readable_field(name, field_sr);
        add_readable_field(field_ptr.get());
        created_readable_fields.push_back(field_ptr);
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
    template<typename T, unsigned int eeprom_save_period = 0>
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
        auto field_ptr = __field_creator<T, eeprom_save_period>::create_writable_field(name, field_sr);
        add_writable_field(field_ptr.get());
        created_writable_fields.push_back(field_ptr);
        return field_ptr;
    }

    template<typename T, unsigned int eeprom_save_period = 0>
    std::shared_ptr<ReadableStateField<T>> create_readable_field(const std::string& name, 
        T min, T max)
    {
        static_assert(std::is_same<T, signed int>::value ||
            std::is_same<T, unsigned int>::value ||
            std::is_same<T, signed char>::value ||
            std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr(min, max);
        auto field_ptr = __field_creator<T, eeprom_save_period>::create_readable_field(name, field_sr);
        add_readable_field(field_ptr.get());
        created_readable_fields.push_back(field_ptr);
        return field_ptr;
    }

    template<typename T, unsigned int eeprom_save_period = 0>
    std::shared_ptr<WritableStateField<T>> create_writable_field(const std::string& name, 
        T min, T max)
    {
        static_assert(std::is_same<T, signed int>::value ||
            std::is_same<T, unsigned int>::value ||
            std::is_same<T, signed char>::value ||
            std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr(min, max);
        auto field_ptr = __field_creator<T, eeprom_save_period>::create_writable_field(name, field_sr);
        add_writable_field(field_ptr.get());
        created_writable_fields.push_back(field_ptr);
        return field_ptr;
    }

    template<typename T, unsigned int eeprom_save_period = 0>
    std::shared_ptr<ReadableStateField<T>> create_readable_field(const std::string& name, T max)
    {
        static_assert(std::is_same<T, unsigned int>::value ||
                      std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr(max);
        auto field_ptr = __field_creator<T, eeprom_save_period>::create_readable_field(name, field_sr);
        add_readable_field(field_ptr.get());
        created_readable_fields.push_back(field_ptr);
        return field_ptr;
    }

    template<typename T, unsigned int eeprom_save_period = 0>
    std::shared_ptr<WritableStateField<T>> create_writable_field(const std::string& name, T max)
    {
        static_assert(std::is_same<T, unsigned int>::value ||
                      std::is_same<T, unsigned char>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<T> field_sr(max);
        auto field_ptr = __field_creator<T, eeprom_save_period>::create_writable_field(name, field_sr);
        add_writable_field(field_ptr.get());
        created_writable_fields.push_back(field_ptr);
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
        auto field_ptr = __field_creator<std::array<T, 3>, 0>::create_readable_field(name, field_sr);
        add_readable_field(field_ptr.get());
        created_readable_fields.push_back(field_ptr);
        return field_ptr;
    }

    template<typename T>
    std::shared_ptr<ReadableStateField<lin::Vector<T,3>>> create_readable_lin_vector_field(const std::string& name,
        T min, T max, size_t bitsize)
    {
        static_assert(std::is_same<T, float>::value ||
                      std::is_same<T, double>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<lin::Vector<T, 3>> field_sr(min, max, bitsize);
        auto field_ptr = std::make_shared<ReadableStateField<lin::Vector<T, 3>>>(name, field_sr);
        add_readable_field(field_ptr.get());
        created_readable_fields.push_back(field_ptr);
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
        auto field_ptr = __field_creator<std::array<T, 3>, 0>::create_writable_field(name, field_sr);
        add_writable_field(field_ptr.get());
        created_writable_fields.push_back(field_ptr);
        return field_ptr;
    }

    template<typename T>
    std::shared_ptr<WritableStateField<lin::Vector<T, 3>>> create_writable_lin_vector_field(const std::string& name,
        T min, T max, size_t bitsize)
    {
        static_assert(std::is_same<T, float>::value ||
                      std::is_same<T, double>::value,
            "Type argument for field creation with the given parameters was invalid.");

        Serializer<lin::Vector<T, 3>> field_sr(min, max, bitsize);
        auto field_ptr = std::make_shared<WritableStateField<lin::Vector<T, 3>>>(name, field_sr);
        add_writable_field(field_ptr.get());
        created_writable_fields.push_back(field_ptr);
        return field_ptr;
    }

    /**
     * @brief Create an event and add it to the registry.
     * 
     * @param name Name of event to create.
     * @return Pointer to event that was created.
     */
    std::shared_ptr<Event> create_event(const std::string& name,
          std::vector<ReadableStateFieldBase*>& data_fields,
          const char* (*print_fn)(const unsigned int, std::vector<ReadableStateFieldBase*>&))
    {
        auto event_ptr = std::make_shared<Event>(name, data_fields, print_fn);
        add_event(event_ptr.get());
        created_events.push_back(event_ptr);
        return event_ptr;
    }

    /**
     * @brief Create a fault and add it to the registry.
     * 
     * @param name Name of fault to create.
     * @return Pointer to fault that was created.
     */
    std::shared_ptr<Fault> create_fault(const std::string& name, const size_t _persistence, const unsigned int& control_cycle_count)
    {
        auto fault_ptr = std::make_shared<Fault>(name, _persistence, control_cycle_count);
        add_fault(fault_ptr.get());
        created_faults.push_back(fault_ptr);
        return fault_ptr;
    }

    /**
     * @brief Empty the registry.
     */
    void clear() {
        internal_fields.clear();
        readable_fields.clear();
        writable_fields.clear();
        faults.clear();
        events.clear();
        created_internal_fields.clear();
        created_readable_fields.clear();
        created_writable_fields.clear();
        created_events.clear();
        created_faults.clear();
        created_events.clear();
    }

  private:
    // Store pointers to all of the state fields, events, and faults that have been created, in order
    // to prevent segmentation faults due to shared pointers going out of scope before a
    // ControlTask has a chance to call find_readable_field().
    
    std::vector<std::shared_ptr<InternalStateFieldBase>> created_internal_fields;
    std::vector<std::shared_ptr<ReadableStateFieldBase>> created_readable_fields;
    std::vector<std::shared_ptr<WritableStateFieldBase>> created_writable_fields;
    std::vector<std::shared_ptr<Event>> created_events;
    std::vector<std::shared_ptr<Fault>> created_faults;
};

#endif
