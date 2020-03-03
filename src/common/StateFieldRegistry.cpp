#include "StateFieldRegistry.hpp"
#include <algorithm>

StateFieldRegistry::StateFieldRegistry() {}

InternalStateFieldBase*
StateFieldRegistry::find_internal_field(const std::string &name) const {
    auto it =
        std::find_if(
            internal_fields.begin(), internal_fields.end(), 
            [&](InternalStateFieldBase* f) { return f->name() == name; });

    if (it != internal_fields.end()) return *it;
    else return nullptr;
}

ReadableStateFieldBase*
StateFieldRegistry::find_readable_field(const std::string &name) const {
    auto it =
        std::find_if(
            readable_fields.begin(), readable_fields.end(), 
            [&](ReadableStateFieldBase* f) { return f->name() == name; });

    if (it != readable_fields.end()) return *it;
    else return nullptr;
}

WritableStateFieldBase*
StateFieldRegistry::find_writable_field(const std::string &name) const {
    auto it =
        std::find_if(
            writable_fields.begin(), writable_fields.end(), 
            [&](WritableStateFieldBase* f) { return f->name() == name; });

    if (it != writable_fields.end()) return *it;
    else return nullptr;
}

Event*
StateFieldRegistry::find_event(const std::string &name) const {
    auto it =
        std::find_if(
            events.begin(), events.end(), 
            [&](Event* e) { return e->name() == name; });

    if (it != events.end()) return *it;
    else return nullptr;
}

Fault*
StateFieldRegistry::find_fault(const std::string &name) const {
    auto it =
        std::find_if(
            faults.begin(), faults.end(), 
            [&](Fault* f) { return f->name() == name; });

    if (it != faults.end()) return *it;
    else return nullptr;
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