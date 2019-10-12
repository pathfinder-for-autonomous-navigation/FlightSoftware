#ifndef CONTROL_TASK_HPP_
#define CONTROL_TASK_HPP_

#include <memory>
#include <iostream>
#include <string>
#include <debug_console.hpp>
#include <Nameable.hpp>
#include <StateFieldBase.hpp>
#include <StateFieldRegistry.hpp>

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

  protected:
    StateFieldRegistry& _registry;

    /**
     * @brief Find a field of a given name within the state registry
     * and return a pointer to it.
     *
     * @param name Name of state field.
     *
     * @return Pointer to field, or null pointer if field doesn't exist.
     */
    std::shared_ptr<StateFieldBase> find_field(const std::string& name) {
        return _registry.find_field(name);
    }

    template<typename U>
    void add_readable(ReadableStateField<U>& field) {
        std::shared_ptr<ReadableStateField<U>> field_ptr(
            std::shared_ptr<ReadableStateField<U>>{}, &field);
        _registry.add_readable(field_ptr);
    }

    template<typename U>
    void add_writable(WritableStateField<U>& field) {
        std::shared_ptr<WritableStateField<U>> field_ptr(
            std::shared_ptr<WritableStateField<U>>{}, &field);
        _registry.add_writable(field_ptr);
    }

    template<typename U>
    void find_readable_field(const char* field, std::shared_ptr<ReadableStateField<U>>* field_ptr, const char* file, const unsigned int line) {
        *field_ptr = std::dynamic_pointer_cast<ReadableStateField<U>>(_registry.find_readable_field(field));
        if (!(*field_ptr)) { 
            printf(debug_severity::error, "%s:%d: Readable field required is not present in state registry: %s\n", file, line, field);
        }
    }

    template<typename U>
    void find_writable_field(const char* field, std::shared_ptr<WritableStateField<U>>* field_ptr, const char* file, const unsigned int line) {
        *field_ptr = std::dynamic_pointer_cast<WritableStateField<U>>(_registry.find_writable_field(field));
        if (!(*field_ptr)) { 
            printf(debug_severity::error, "%s:%d: Writable field required is not present in state registry: %s\n", file, line, field);
        }
    }
};

#endif
