#ifndef CONTROL_TASK_HPP_
#define CONTROL_TASK_HPP_

#include <memory>
#include <string>
#include "ControlTaskBase.hpp"
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
class ControlTask : protected ControlTaskBase {
  public:
    /**
     * @brief Construct a new Control ControlTaskBase object
     *
     * @param name     Name of control ControlTaskBase
     * @param registry Pointer to state field registry
     */
    ControlTask(const std::string& name, StateFieldRegistry& registry)
        : _name(name), _registry(registry) {}

    const std::string& name() const override { return _name; };

    /**
     * @brief Run main method of control ControlTaskBase.
     */
    virtual T execute() = 0;

  protected:
    const std::string _name;
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

    /**
     * @brief Marks a field as being downloadable by ground.
     *
     * @param field State field
     */
    bool add_readable(std::shared_ptr<ReadableStateFieldBase>& field) {
        return _registry.add_readable(field);
    }

    /**
     * @brief Marks a field as being uploadable by ground.
     *
     * @param r ControlTaskBase
     * @param field Data field
     */
    bool add_writable(std::shared_ptr<WritableStateFieldBase>& field) {
        return _registry.add_writable(field);
    }

    /**
     * @brief Convenience function for telling the console that the ControlTask tried to load a reference
     * to a state field that it expected to exist, but couldn't find it.
     * 
     * @param field_name Field that task tried to load.
     */
    void print_registry_404_error(const char* field_name) {
        printf(debug_severity::error, "Field required by %s is not present in state registry: %s\n", (this->name()).c_str(), field_name);
    }
};

#endif
