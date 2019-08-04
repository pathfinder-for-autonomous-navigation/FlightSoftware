#ifndef STATE_FIELD_REGISTRY_HPP_
#define STATE_FIELD_REGISTRY_HPP_

#include <map>
#include <memory>
#include <vector>
#include "ControlTaskBase.hpp"
#include "StateFieldBase.hpp"
#include "debug_console.hpp"

/**
 * @brief Registry of state fields and which tasks have read/write access to
 * the fields. StateField objects use this registry to verify valid access to
 * their values. Essentially, this class is a lightweight wrapper around
 * multimap.
 */
class StateFieldRegistry : public Debuggable {
   private:
    std::map<std::string, std::shared_ptr<StateFieldBase>> _fields;
    std::map<std::shared_ptr<ControlTaskBase>, std::vector<std::shared_ptr<StateFieldBase>>>
        _fields_allowed_to_read;
    std::map<std::shared_ptr<ControlTaskBase>, std::vector<std::shared_ptr<StateFieldBase>>>
        _fields_allowed_to_write;

   public:
    // TODO make a singleton
    StateFieldRegistry();

    /**
     * @brief Copy constructor.
     */
    void operator=(const StateFieldRegistry &r);

    /**
     * @brief Find a field of a given name within the state registry
     * and return a pointer to it.
     *
     * @param name Name of state field.
     *
     * @return Pointer to field, or null pointer if field doesn't exist.
     */
    std::shared_ptr<StateFieldBase> find_field(const std::string& name) {
        auto it = _fields.find(name);

        if (it != _fields.end())
            return it->second;
        else
            return std::shared_ptr<StateFieldBase>(nullptr);
    }

    /**
     * @brief Allows the specified Control ControlTaskBase to read the specified state field.
     * If the field is not present in the registry yet, it is added.
     *
     * @param r ControlTaskBase
     * @param field State field
     */
    void add_reader(const std::shared_ptr<ControlTaskBase> &r,
                    const std::shared_ptr<StateFieldBase> &field);

    /**
     * @brief Allows the specified Control ControlTaskBase to write to the specified state
     * field. If the field is not present in the registry yet, it is added.
     *
     * @param r ControlTaskBase
     * @param field Data field
     */
    void add_writer(const std::shared_ptr<ControlTaskBase> &w,
                    const std::shared_ptr<StateFieldBase> &field);

    /**
     * @brief Checks registry for read access.
     *
     * @param r
     * @param field
     * @return true If Control ControlTaskBase has read access to state field.
     * @return false If Control ControlTaskBase does not have read access to state field.
     */
    bool can_read(const std::shared_ptr<ControlTaskBase> &r,
                  const std::shared_ptr<StateFieldBase> &field);

    /**
     * @brief Checks registry for write access.
     *
     * @param w
     * @param field
     * @return true If Control ControlTaskBase has write access to state field.
     * @return false If Control ControlTaskBase does not have write access to state field.
     */
    bool can_write(const std::shared_ptr<ControlTaskBase> &r,
                   const std::shared_ptr<StateFieldBase> &field);
};

#endif