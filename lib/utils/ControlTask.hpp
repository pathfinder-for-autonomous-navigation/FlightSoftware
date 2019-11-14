#ifndef CONTROL_TASK_HPP_
#define CONTROL_TASK_HPP_

#include <memory>
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

    /**
     * @brief Destroy the Control Task object
     * 
     * We need to have this destructor to avoid compilation errors.
     */
    virtual ~ControlTask() = 0;

  protected:
    StateFieldRegistry& _registry;

    template<typename U>
    bool add_internal_field(InternalStateField<U>& field) {
        return _registry.add_internal_field(field.ptr());
    }

    template<typename U>
    bool add_readable_field(ReadableStateField<U>& field) {
        return _registry.add_readable_field(field.ptr());
    }

    template<typename U>
    bool add_writable_field(WritableStateField<U>& field) {
        return _registry.add_writable_field(field.ptr());
    }

    template<typename U>
    std::shared_ptr<ReadableStateField<U>> find_readable_field(const char* field, const char* file, const unsigned int line) {
        auto field_ptr = std::static_pointer_cast<ReadableStateField<U>>(_registry.find_readable_field(field));
        if (!field_ptr) { 
            printf(debug_severity::error, "%s:%d: Readable field required is not present in state registry: %s\n", file, line, field);
        }
        return field_ptr;
    }

    template<typename U>
    std::shared_ptr<WritableStateField<U>> find_writable_field(const char* field, const char* file, const unsigned int line) {
        auto field_ptr = std::static_pointer_cast<WritableStateField<U>>(_registry.find_writable_field(field));
        if (!field_ptr) { 
            printf(debug_severity::error, "%s:%d: Writable field required is not present in state registry: %s\n", file, line, field);
        }
        return field_ptr;
    }
};

template<typename T>
ControlTask<T>::~ControlTask() {}

#endif
