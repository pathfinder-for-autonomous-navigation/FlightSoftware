#ifndef STATE_FIELD_REGISTRY_HPP_
#define STATE_FIELD_REGISTRY_HPP_

#include <memory>
#include <set>
#include "StateField.hpp"
#include "Fault.hpp"

/**
 * @brief Registry of state fields and which tasks have read/write access to
 * the fields. StateField objects use this registry to verify valid access to
 * their values. Essentially, this class is a lightweight wrapper around
 * multimap.
 */
class StateFieldRegistry {
public:
    std::vector<InternalStateFieldBase*> internal_fields;
    std::vector<ReadableStateFieldBase*> readable_fields;
    std::vector<WritableStateFieldBase*> writable_fields;
    std::vector<Fault*> faults;

    StateFieldRegistry();

    /**
     * @brief Find a field of a given name within the state registry and return a pointer to it.
     *
     * @param[in] name Name of state field.
     * @return Pointer to field, or null pointer if field doesn't exist.
     */
    InternalStateFieldBase* find_internal_field(const std::string &name) const;

    /**
     * @brief Find a readable field of a given name within the state registry and return a pointer
     * to it.
     *
     * @param[in] name Name of state field.
     * @return Pointer to field, or null pointer if field doesn't exist.
     */
    ReadableStateFieldBase* find_readable_field(const std::string &name) const;

    /**
     * @brief Find a writable field of a given name within the state registry and return a pointer
     * to it.
     *
     * @param[in] name Name of state field.
     * @return Pointer to field, or null pointer if field doesn't exist.
     */
    WritableStateFieldBase* find_writable_field(const std::string &name) const;

    /**
     * @brief Find a fault with the given name and return a pointer to it.
     * 
     * @param name Name of fault.
     * @return Fault* Pointer to fault, or null pointer if fault doesn't exist.
     */
    Fault* find_fault(const std::string& name) const;

    /**
     * @brief Adds a field to the registry.
     *
     * @param field State field
     */
    bool add_internal_field(InternalStateFieldBase* field);

    /**
     * @brief Marks a field as being downloadable by ground.
     *
     * @param field State field
     */
    bool add_readable_field(ReadableStateFieldBase* field);

    /**
     * @brief Marks a field as being uploadable by ground.
     *
     * @param field Data field
     */
    bool add_writable_field(WritableStateFieldBase* field);

    /**
     * @brief Adds a fault to the registry.
     * 
     * @param Pointer to fault.
     */
    bool add_fault(Fault* fault);
};

#endif
