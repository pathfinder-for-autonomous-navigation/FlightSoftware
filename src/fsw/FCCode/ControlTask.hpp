#ifndef CONTROL_TASK_HPP_
#define CONTROL_TASK_HPP_

#include <memory>
#include <string>
#include <common/debug_console.hpp>
#include <common/Nameable.hpp>
#include <common/StateFieldBase.hpp>
#include <common/StateFieldRegistry.hpp>

#ifdef DESKTOP
#include <iostream>
#else
#include <Arduino.h>
#endif

/**
 * @brief A control ControlTaskBase is a wrapper around any high-level ControlTaskBase that
 * interacts with satellite data.
 *
 * This class is subclassed heavily in order to provide multiple
 * implementations of execute() for different kinds of ControlTasks.
 */
template <typename T>
class ControlTask : protected debug_console {
  public:
    /**
     * @brief Construct a new Control ControlTaskBase object
     *
     * @param name     Name of control ControlTaskBase
     * @param registry Pointer to state field registry
     */
    ControlTask(StateFieldRegistry& registry) : _registry(registry) {}

    /**
     * @brief Run main method of control ControlTaskBase.
     */
    virtual T execute() = 0;

    /**
     * @brief Destroy the Control Task object
     * 
     * We need to have this destructor to avoid compilation errors.
     */
    virtual ~ControlTask() = 0;

  protected:
    StateFieldRegistry& _registry;

    void check_field_added(const bool added, const std::string& field_name) {
        if(!added) {
            #ifdef UNIT_TEST
                #ifdef DESKTOP
                    std::cout << "Field \"" << field_name
                        << "\" is already in the registry." << std::endl;
                #else
                    Serial.printf("Field \"%s\" is already in the registry.",
                        field_name.c_str());
                #endif
            #else
                #ifdef FUNCTIONAL_TEST
                printf(debug_severity::error, "Field \"%s\" is already in the registry.",
                    field_name.c_str());
                #endif
            #endif
            assert(false);
        }
    }

    template<typename U>
    void add_internal_field(InternalStateField<U>& field) {
        const bool added = _registry.add_internal_field(
            static_cast<InternalStateFieldBase*>(&field));
        check_field_added(added, field.name());
    }

    template<typename U>
    void add_readable_field(ReadableStateField<U>& field) {
        const bool added = _registry.add_readable_field(
            static_cast<ReadableStateFieldBase*>(&field));
        check_field_added(added, field.name());
    }

    template<typename U>
    void add_writable_field(WritableStateField<U>& field) {
        const bool added = _registry.add_writable_field(
            static_cast<WritableStateFieldBase*>(&field));
        check_field_added(added, field.name());
    }

    void check_field_exists(const StateFieldBase* ptr, const std::string& field_type,
            const char* field_name) {
        if(!ptr) {
            #ifdef UNIT_TEST
                #ifdef DESKTOP
                    std::cout << field_type << " field \"" << field_name
                        << "\" is not present in the registry." << std::endl;
                #else
                    Serial.printf("%s field \"%s\" is not present in the registry.",
                        field_type.c_str(), field_name);
                #endif
            #else
                #ifndef FLIGHT
                printf(debug_severity::error, "%s field \"%s\" is not present in the registry.",
                    field_type.c_str(), field_name);
                #endif
            #endif
            assert(false);
        }
    }

    template<typename U>
    InternalStateField<U>* find_internal_field(const char* field, const char* file, const unsigned int line) {
        auto field_ptr = _registry.find_internal_field(field);
        check_field_exists(field_ptr, "internal", field);
        return static_cast<InternalStateField<U>*>(field_ptr);
    }

    template<typename U>
    ReadableStateField<U>* find_readable_field(const char* field, const char* file, const unsigned int line) {
        auto field_ptr = _registry.find_readable_field(field);
        check_field_exists(field_ptr, "readable", field);
        return static_cast<ReadableStateField<U>*>(field_ptr);
    }

    template<typename U>
    WritableStateField<U>* find_writable_field(const char* field, const char* file, const unsigned int line) {
        auto field_ptr = _registry.find_writable_field(field);
        check_field_exists(field_ptr, "writable", field);
        return static_cast<WritableStateField<U>*>(field_ptr);
    }
};

template<typename T>
ControlTask<T>::~ControlTask() {}

#endif
