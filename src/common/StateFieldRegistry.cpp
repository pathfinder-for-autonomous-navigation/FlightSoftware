#include "StateFieldRegistry.hpp"

StateFieldRegistry::StateFieldRegistry() {}

InternalStateFieldBase*
StateFieldRegistry::find_internal_field(const std::string &name) const {
    for (InternalStateFieldBase* field : internal_fields) {
        if (name == field->name()) {
            return field;
        }
    }

    return nullptr;
}

ReadableStateFieldBase*
StateFieldRegistry::find_readable_field(const std::string &name) const {
    for (ReadableStateFieldBase* field : readable_fields) {
        if (name == field->name()) return field;
    }

    return nullptr;
}

WritableStateFieldBase*
StateFieldRegistry::find_writable_field(const std::string &name) const {
    for (WritableStateFieldBase* field : writable_fields) {
        if (name == field->name()) return field;
    }

    return nullptr;
}

bool StateFieldRegistry::add_internal_field(InternalStateFieldBase* field) {
    if (find_internal_field(field->name())) return false;
    internal_fields.push_back(field);
    return true;
}

bool StateFieldRegistry::add_readable_field(ReadableStateFieldBase* field) {
    if (find_readable_field(field->name())) return false;
    readable_fields.push_back(field);
    return true;
}

bool StateFieldRegistry::add_writable_field(WritableStateFieldBase* field) {
    if (!add_readable_field(field)) return false;
    if (find_writable_field(field->name())) return false;
    writable_fields.push_back(field);
    return true;
}
