#include "StateFieldRegistry.hpp"

#include <algorithm>

template <typename T>
static auto lower_bound(std::vector<T *> const &ts, std::string const &name) {
    return std::lower_bound(ts.cbegin(), ts.cend(), name,
            [](auto const *e, auto const &name) -> bool {
                return e->name() < name;
            });
}

template <typename T>
static bool add(std::vector<T *> &ts, T *t) {
    auto const it = lower_bound(ts, t->name());

    // If the lower bound isn't end and is "equal" the name has already been
    // taken.
    if (it != ts.cend() && (*it)->name() == t->name()) return false;

    // Otherwise, we insert the new entry at it's lower bound inserting at the
    // end if the new element is the largest.
    ts.insert(it, t);
    assert(std::is_sorted(ts.cbegin(), ts.cend(),
            [](auto const *t, auto const *u) -> bool {
                return t->name() == u->name();
            }));
    return true;
}

template <typename T>
static T *find(std::vector<T *> const &ts, std::string const &name) {
    auto const it = lower_bound(ts, name);
    return (it != ts.cend() && (*it)->name() == name) ? *it : nullptr;
}

StateFieldRegistry::StateFieldRegistry() {}

InternalStateFieldBase*
StateFieldRegistry::find_internal_field(const std::string &name) const {
    return find(internal_fields, name);
}

ReadableStateFieldBase*
StateFieldRegistry::find_readable_field(const std::string &name) const {
    return find(readable_fields, name);
}

WritableStateFieldBase*
StateFieldRegistry::find_writable_field(const std::string &name) const {
    return find(writable_fields, name);
}

SerializableStateFieldBase*
StateFieldRegistry::find_eeprom_saved_field(const std::string &name) const {
    return find(eeprom_saved_fields, name);
}

Event*
StateFieldRegistry::find_event(const std::string &name) const {
    return find(events, name);
}

Fault*
StateFieldRegistry::find_fault(const std::string &name) const {
    return find(faults, name);
}

bool StateFieldRegistry::add_internal_field(InternalStateFieldBase* field) {
    return add(internal_fields, field);
}

bool StateFieldRegistry::add_readable_field(ReadableStateFieldBase* field) {
    if (find_readable_field(field->name())) return false;
    if (field->eeprom_save_period() > 0) {
        if (find_eeprom_saved_field(field->name())) return false;
        else eeprom_saved_fields.push_back(field);
    }
    return add(readable_fields, field);
}

bool StateFieldRegistry::add_writable_field(WritableStateFieldBase* field) {
    if (!add_readable_field(field)) return false;
    return add(writable_fields, field);
}

bool StateFieldRegistry::add_event(Event* event) {
    return add(events, event);
}

bool StateFieldRegistry::add_fault(Fault* fault) {
    if (find_fault(fault->name())) return false;
    if (!add_writable_field(static_cast<WritableStateFieldBase*>(fault))) return false;
    if (!add_writable_field(&fault->suppress_f)) return false;
    if (!add_writable_field(&fault->override_f)) return false;
    if (!add_writable_field(&fault->unsignal_f)) return false;
    if (!add_writable_field(&fault->persistence_f)) return false;

    return add(faults, fault);
}
