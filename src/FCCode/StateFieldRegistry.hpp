#ifndef STATE_FIELD_REGISTRY_HPP_
#define STATE_FIELD_REGISTRY_HPP_

#include <map>
#include <memory>
#include <vector>
#include "ControlTaskBase.hpp"
#include "StateFieldBase.hpp"

/**
 * @brief Registry of state fields and which tasks have read/write access to
 * the fields. StateField objects use this registry to verify valid access to
 * their values. Essentially, this class is a lightweight wrapper around
 * multimap.
 */
class StateFieldRegistry {
   private:
    std::set<std::shared_ptr<StateFieldBase>> fields;
    std::set<std::shared_ptr<StateFieldBase>> readable_fields;
    std::set<std::shared_ptr<StateFieldBase>> writable_fields;

   public:
    // TODO make a singleton
    StateFieldRegistry() {}

    /**
     * @brief Copy constructor.
     */
    void operator=(const StateFieldRegistry &r) {
        this->fields = r.fields;
        this->readable_fields = r.readable_fields;
        this->writable_fields = r.writable_fields;
    }

    /**
     * @brief Find a field of a given name within the state registry and return a pointer to it.
     *
     * @param name Name of state field.
     *
     * @return Pointer to field, or null pointer if field doesn't exist.
     */
    std::shared_ptr<StateFieldBase> find_field(const std::string &name) {
        for (auto const &field : fields) {
            if (name == field->name()) return field;
        }

        return std::shared_ptr<StateFieldBase>(nullptr);
    }

    /**
     * @brief Marks a field as being downloadable by ground.
     *
     * @param field State field
     */
    bool add_readable(std::shared_ptr<StateFieldBase> &field) {
        if (find_field(field->name())) return false;
        fields.insert(field);
        readable_fields.insert(field);
        return true;
    }

    /**
     * @brief Marks a field as being uploadable by ground.
     *
     * @param r ControlTaskBase
     * @param field Data field
     */
    bool add_writable(std::shared_ptr<StateFieldBase> &field) {
        if (find_field(field->name())) return false;
        fields.insert(field);
        writable_fields.insert(field);
        return true;
    }
};

#endif