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

SerializableStateFieldBase*
StateFieldRegistry::find_eeprom_saved_field(const std::string &name) const {
    for (ReadableStateFieldBase* field : eeprom_saved_fields) {
        if (name == field->name()) return field;
    }

    return nullptr;
}

Event*
StateFieldRegistry::find_event(const std::string &name) const {
    for (Event* event : events) {
        if (name == event->name()) return event;
    }

    return nullptr;
}

Fault*
StateFieldRegistry::find_fault(const std::string &name) const {
    for (Fault* fault : faults) {
        if (name == fault->name()) return fault;
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
    if (field->eeprom_save_period() > 0) {
        if (find_eeprom_saved_field(field->name())) return false;
        else eeprom_saved_fields.push_back(field);
    }
    readable_fields.push_back(field);
    return true;
}

bool StateFieldRegistry::add_writable_field(WritableStateFieldBase* field) {
    if (!add_readable_field(field)) return false;
    if (find_writable_field(field->name())) return false;
    writable_fields.push_back(field);
    return true;
}

bool StateFieldRegistry::add_event(Event* event) {
    if (find_event(event->name())) return false;
    events.push_back(event);
    return true;
}

bool StateFieldRegistry::add_fault(Fault* fault) {
    if (find_fault(fault->name())) return false;
    if (!add_writable_field(static_cast<WritableStateFieldBase*>(fault))) return false;
    if (!add_writable_field(&fault->suppress_f)) return false;
    if (!add_writable_field(&fault->override_f)) return false;
    if (!add_writable_field(&fault->unsignal_f)) return false;
    if (!add_writable_field(&fault->persistence_f)) return false;

    faults.push_back(fault);
    return true;
}
