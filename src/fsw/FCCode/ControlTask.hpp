#ifndef CONTROL_TASK_HPP_
#define CONTROL_TASK_HPP_

#include <memory>
#include <string>
#include <common/debug_console.hpp>
#include <common/assertion.hpp>
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
     * @brief Construct a new Control ControlTaskBase object.
     * 
     * The constructor may throw pan_asserts if state field initializations
     * fail.
     *
     * @param name     Name of control ControlTaskBase
     * @param registry Pointer to state field registry
     */
    ControlTask(StateFieldRegistry& registry) noexcept(false) : _registry(registry) {}

    /**
     * @brief Run main method of control ControlTaskBase.
     * 
     * The function is marked noexcept so that the control task's unit test
     * can ensure the function cannot throw any pan_asserts.
     */
    virtual T execute() noexcept = 0;

    /**
     * @brief Destroy the Control Task object
     * 
     * We need to have this destructor to avoid compilation errors.
     */
    virtual ~ControlTask() = 0;

  protected:
    StateFieldRegistry& _registry;

  private:
    void check_field_added(const bool added, const std::string& field_name) {
        char field_already_exists_msg[100];
        sprintf(field_already_exists_msg,
            "Field \"%s\" was already in the registry.",
            field_name.c_str());

        pan_assert<std::invalid_argument>(added, field_already_exists_msg);
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
        char error_msg[50];
        sprintf(error_msg, "%s field \"%s\" is not present in the registry.",
            field_type.c_str(), field_name);
        pan_assert<std::invalid_argument>(ptr != nullptr, error_msg);
    }


  #ifdef UNIT_TEST
  public:
  #else
  protected:
  #endif

    template<typename U>
    InternalStateField<U>* find_internal_field(const char* field, const char* file, const unsigned int line) {
        InternalStateFieldBase* field_ptr = _registry.find_internal_field(field);
        check_field_exists(field_ptr, "internal", field);
        return static_cast<InternalStateField<U>*>(field_ptr);
    }

    template<typename U>
    ReadableStateField<U>* find_readable_field(const char* field, const char* file, const unsigned int line) {
        ReadableStateFieldBase* field_ptr = _registry.find_readable_field(field);
        check_field_exists(field_ptr, "readable", field);
        return static_cast<ReadableStateField<U>*>(field_ptr);
    }

    template<typename U>
    WritableStateField<U>* find_writable_field(const char* field, const char* file, const unsigned int line) {
        WritableStateFieldBase* field_ptr = _registry.find_writable_field(field);
        check_field_exists(field_ptr, "writable", field);
        return static_cast<WritableStateField<U>*>(field_ptr);
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

template<typename T>
ControlTask<T>::~ControlTask() {}

#endif
