#ifndef CONTROL_TASK_HPP_
#define CONTROL_TASK_HPP_

#include <memory>
#include <string>
#include <common/casts.hpp>
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
    virtual ~ControlTask() = default;

  protected:
    StateFieldRegistry& _registry;

  private:
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

  #ifdef UNIT_TEST
  public:
  #else
  protected:
  #endif

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

    void add_event(Event& event) {
        const bool added = _registry.add_event(&event);
        check_field_added(added, event.name());
    }

    void add_fault(Fault& fault) {
        const bool added = _registry.add_fault(&fault);
        check_field_added(added, fault.name());
    }

  private:

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

    template<class C, class D>
    void check_templated_field_exists(const C* ptr, const std::string& field_type,
            const char* field_name) {
        check_field_exists(ptr, field_type, field_name);
        if (!DYNAMIC_CAST(const D*, ptr)) {
            #ifdef UNIT_TEST
                #ifdef DESKTOP
                    std::cout << field_type << " field \"" << field_name
                            << "\" was casted to the wrong type." << std::endl;
                #else
                    Serial.printf("%s field \"%s\" was casted to the wrong type.",
                            field_type.c_str(), field_name);
                #endif
            #else
                #ifndef FLIGHT
                printf(debug_severity::error, "%s field \"%s\" was casted to the wrong type.",
                        field_type.c_str(), field_name);
                #endif
            #endif
            assert(false);
        }
    }

  #ifdef UNIT_TEST
  public:
  #else
  protected:
  #endif

    template<typename U>
    InternalStateField<U>* find_internal_field(const char* field, const char* file, const unsigned int line) {
        InternalStateFieldBase* field_ptr = _registry.find_internal_field(field);
        check_templated_field_exists<InternalStateFieldBase, InternalStateField<U>>(field_ptr, "internal", field);
        return DYNAMIC_CAST(InternalStateField<U>*, field_ptr);
    }

    template<typename U>
    ReadableStateField<U>* find_readable_field(const char* field, const char* file, const unsigned int line) {
        ReadableStateFieldBase* field_ptr = _registry.find_readable_field(field);
        check_templated_field_exists<ReadableStateFieldBase, ReadableStateField<U>>(field_ptr, "readable", field);
        return DYNAMIC_CAST(ReadableStateField<U>*, field_ptr);
    }

    template<typename U>
    WritableStateField<U>* find_writable_field(const char* field, const char* file, const unsigned int line) {
        WritableStateFieldBase* field_ptr = _registry.find_writable_field(field);
        check_templated_field_exists<WritableStateFieldBase, WritableStateField<U>>(field_ptr, "writable", field);
        return DYNAMIC_CAST(WritableStateField<U>*, field_ptr);
    }

    Event* find_event(const char* event, const char* file, const unsigned int line) {
        Event* event_ptr = _registry.find_event(event);
        check_field_exists(event_ptr, "event", event);
        return event_ptr;
    }

    Fault* find_fault(const char* fault, const char* file, const unsigned int line) {
        Fault* fault_ptr = _registry.find_fault(fault);
        check_field_exists(fault_ptr, "fault", fault);
        return fault_ptr;
    }
};

/* Convenient macros to find fields, events, and faults. The field/event/fault
 * argument will be automatically stringified so no need to include quotation
 * marks. */
#define FIND_INTERNAL_FIELD(type, field) find_internal_field<type>(#field, __FILE__, __LINE__)
#define FIND_READABLE_FIELD(type, field) find_readable_field<type>(#field, __FILE__, __LINE__)
#define FIND_WRITABLE_FIELD(type, field) find_writable_field<type>(#field, __FILE__, __LINE__)
#define FIND_EVENT(event)                find_event(#event, __FILE__, __LINE__)
#define FIND_FAULT(fault)                find_fault(#fault, __FILE__, __LINE__)

#endif
