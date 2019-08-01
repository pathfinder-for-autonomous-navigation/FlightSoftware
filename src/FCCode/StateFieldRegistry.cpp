#include "StateField.hpp"

DataField::DataField(const std::string &name) : Nameable(name) {}

StateFieldRegistry::StateFieldRegistry()
    : Debuggable(), _fields_allowed_to_read(), _fields_allowed_to_write() {}

void StateFieldRegistry::operator=(const StateFieldRegistry &r) {
    _fields_allowed_to_read = r._fields_allowed_to_read;
    _fields_allowed_to_write = r._fields_allowed_to_write;
}

void StateFieldRegistry::add_reader(const std::shared_ptr<Task> &reader,
                                    const std::shared_ptr<DataField> &field) {
    // Add field to registry if it doesn't exist
    if (_fields.find(field->name()) == _fields.end()) _fields.insert({field->name(), field});

    if (_fields_allowed_to_read.count(reader) == 0) {
        // TODO add debug console
        std::vector<std::shared_ptr<DataField>> allowed_fields;
        _fields_allowed_to_read.emplace(reader, allowed_fields);
    }
    // TODO
    _fields_allowed_to_read.at(reader).push_back(field);
}

void StateFieldRegistry::add_writer(const std::shared_ptr<Task> &writer,
                                    const std::shared_ptr<DataField> &field) {
    // Add field to registry if it doesn't exist
    if (_fields.find(field->name()) == _fields.end()) _fields.insert({field->name(), field});

    if (_fields_allowed_to_write.count(writer) == 0) {
        // TODO add debug console
        std::vector<std::shared_ptr<DataField>> fields;
        _fields_allowed_to_write.emplace(writer, fields);
    }
    // TODO
    _fields_allowed_to_write.at(writer).push_back(field);
}

bool StateFieldRegistry::can_read(const std::shared_ptr<Task> &reader,
                                  const std::shared_ptr<DataField> &field) {
    // TODO
    return false;
}

bool StateFieldRegistry::can_write(const std::shared_ptr<Task> &reader,
                                   const std::shared_ptr<DataField> &field) {
    // TODO
    return false;
}