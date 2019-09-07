#ifndef STATE_FIELD_REGISTRY_HPP_
#define STATE_FIELD_REGISTRY_HPP_

#include <memory>
#include <set>
#include "StateField.hpp"

/**
 * @brief Registry of state fields and which tasks have read/write access to
 * the fields. StateField objects use this registry to verify valid access to
 * their values. Essentially, this class is a lightweight wrapper around
 * multimap.
 */
class StateFieldRegistry {
   private:
    std::set<std::shared_ptr<StateFieldBase>> fields;
    std::set<std::shared_ptr<ReadableStateFieldBase>> readable_fields;
    std::set<std::shared_ptr<WritableStateFieldBase>> writable_fields;

   public:
    // TODO make a singleton
    StateFieldRegistry();

    /**
     * @brief Copy constructor.
     */
    void operator=(const StateFieldRegistry &r);

    /**
     * @brief Find a field of a given name within the state registry and return a pointer to it.
     *
     * @param name Name of state field.
     *
     * @return Pointer to field, or null pointer if field doesn't exist.
     */
    std::shared_ptr<StateFieldBase> find_field(const std::string &name) const;

    /**
     * @brief Find a readable field of a given name within the state registry and return a pointer
     * to it.
     *
     * @param name Name of state field.
     *
     * @return Pointer to field, or null pointer if field doesn't exist.
     */
    std::shared_ptr<ReadableStateFieldBase> find_readable_field(const std::string &name) const;

    /**
     * @brief Find a writable field of a given name within the state registry and return a pointer
     * to it.
     *
     * @param name Name of state field.
     *
     * @return Pointer to field, or null pointer if field doesn't exist.
     */
    std::shared_ptr<WritableStateFieldBase> find_writable_field(const std::string &name) const;

    /**
     * @brief Marks a field as being downloadable by ground.
     *
     * @param field State field
     */
    bool add_readable(std::shared_ptr<ReadableStateFieldBase> field);

    /**
     * @brief Marks a field as being uploadable by ground.
     *
     * @param r ControlTaskBase
     * @param field Data field
     */
    bool add_writable(std::shared_ptr<WritableStateFieldBase> field);
};

#endif