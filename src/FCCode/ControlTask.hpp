#ifndef CONTROL_TASK_HPP_
#define CONTROL_TASK_HPP_

#include <memory>
#include <string>
#include "ChRt.h"
#include "ControlTaskBase.hpp"
#include "Nameable.hpp"
#include "StateFieldRegistry.hpp"
#include "debug_console.hpp"

/**
 * @brief A control ControlTaskBase is a wrapper around any high-level ControlTaskBase that
 * interacts with satellite data.
 *
 * This class is subclassed heavily in order to provide multiple
 * implementations of execute() for different kinds of ControlTasks.
 */
template <typename T>
class ControlTask : public ControlTaskBase {
   protected:
    std::shared_ptr<StateFieldRegistry> _registry;

   public:
    /**
     * @brief Construct a new Control ControlTaskBase object
     *
     * @param name     Name of control ControlTaskBase
     * @param registry Pointer to state field registry
     */
    ControlTask(const std::string& name, const std::shared_ptr<StateFieldRegistry>& registry)
        : ControlTaskBase(name), _registry(registry) {}

    /**
     * @brief Run main method of control ControlTaskBase.
     */
    virtual T execute() = 0;

    /**
     * @brief Find a field of a given name within the state registry
     * and return a pointer to it.
     *
     * @param name Name of state field.
     *
     * @return Pointer to field, or null pointer if field doesn't exist.
     */
    std::shared_ptr<StateFieldBase> find_field(const std::string& name) {
        return _registry->find_field(name);
    }

    /**
     * @brief Allows this task to read the specified state field.
     * If the field is not present in the registry yet, it is added.
     *
     * @param field State field
     */
    void add_reader(const std::shared_ptr<StateFieldBase> &field) {
        _registry->add_reader(std::shared_ptr<ControlTaskBase>(this), field);
    }

    /**
     * @brief Allows this task to write to the specified state
     * field. If the field is not present in the registry yet, it is added.
     *
     * @param field Data field
     */
    void add_writer(const std::shared_ptr<StateFieldBase> &field) {
        _registry->add_writer(std::shared_ptr<ControlTaskBase>(this), field);
    }

    /**
     * @brief Checks registry for write access.
     *
     * @param field
     * @return true
     * @return false
     */
    bool can_read(const std::shared_ptr<StateFieldBase>& field) {
        return _registry->can_read(std::shared_ptr<ControlTaskBase>(this), field);
    }

    /**
     * @brief Checks registry for read access.
     *
     * @param field
     * @return true
     * @return false
     */
    bool can_write(const std::shared_ptr<StateFieldBase>& field) {
        return _registry->can_read(std::shared_ptr<ControlTaskBase>(this), field);
    }
};

#endif