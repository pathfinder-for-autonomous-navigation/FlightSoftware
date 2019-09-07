#include "StateFieldRegistry.hpp"

StateFieldRegistry::StateFieldRegistry() {}

void StateFieldRegistry::operator=(const StateFieldRegistry &r) {
    this->fields = r.fields;
    this->readable_fields = r.readable_fields;
    this->writable_fields = r.writable_fields;
}

std::shared_ptr<StateFieldBase> StateFieldRegistry::find_field(const std::string &name) const {
    for (auto const &field : fields) {
        if (name == field->name()) return field;
    }

    return std::shared_ptr<StateFieldBase>(nullptr);
}

std::shared_ptr<ReadableStateFieldBase> StateFieldRegistry::find_readable_field(
    const std::string &name) const {
    for (auto const &field : readable_fields) {
        if (name == field->name()) return field;
    }

    return std::shared_ptr<ReadableStateFieldBase>(nullptr);
}

std::shared_ptr<WritableStateFieldBase> StateFieldRegistry::find_writable_field(
    const std::string &name) const {
    for (auto const &field : writable_fields) {
        if (name == field->name()) return field;
    }

    return std::shared_ptr<WritableStateFieldBase>(nullptr);
}

bool StateFieldRegistry::add_readable(std::shared_ptr<ReadableStateFieldBase> field) {
    if (find_field(field->name())) return false;
    fields.insert(field);
    readable_fields.insert(field);
    return true;
}

bool StateFieldRegistry::add_writable(std::shared_ptr<WritableStateFieldBase> field) {
    if (find_field(field->name())) return false;
    fields.insert(field);
    writable_fields.insert(field);
    return true;
}
