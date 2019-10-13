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

  protected:
    StateFieldRegistry& _registry;

    template<typename U>
    void add_readable_field(ReadableStateField<U>& field) {
        auto field_ptr_base = field.ptr();
        _registry.add_readable_field(field_ptr_base);
    }

    template<typename U>
    void add_writable_field(WritableStateField<U>& field) {
        auto field_ptr_base = field.ptr();
        _registry.add_writable_field(field_ptr_base);
    }

    template<typename U>
    void find_readable_field(const char* field, std::shared_ptr<ReadableStateField<U>>& field_ptr, const char* file, const unsigned int line) {
        auto field_ptr_base = field_ptr->ptr();
        bool found = _registry.find_readable_field(field, field_ptr_base);
        if (!found) { 
            printf(debug_severity::error, "%s:%d: Readable field required is not present in state registry: %s\n", file, line, field);
        }
    }

    template<typename U>
    void find_writable_field(const char* field, std::shared_ptr<WritableStateField<U>>& field_ptr, const char* file, const unsigned int line) {
        auto field_ptr_base = field_ptr->ptr();
        bool found = _registry.find_writable_field(field, field_ptr_base);
        if (!found) { 
            printf(debug_severity::error, "%s:%d: Writable field required is not present in state registry: %s\n", file, line, field);
        }
    }
};

#endif
