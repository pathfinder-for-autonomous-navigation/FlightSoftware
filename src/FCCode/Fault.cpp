#include "Fault.hpp"

Fault::Fault(const std::string& name,
      const size_t _persistence, unsigned int& control_cycle_count,
      const bool default_setting) :
    ReadableStateField<bool>(name, Serializer<bool>()),
    cc(control_cycle_count),
    persistence(_persistence),
    suppress_f(name + std::string(".suppress"), Serializer<bool>()),
    override_f(name + std::string(".override"), Serializer<bool>())
{
  set(default_setting);
  override_f.set(false);
  suppress_f.set(false);
}

bool Fault::add_to_registry(StateFieldRegistry& r) {
    bool added = false;
    added |= r.add_readable_field(static_cast<ReadableStateFieldBase*>(this));
    added |= r.add_writable_field(static_cast<WritableStateFieldBase*>(&suppress_f));
    added |= r.add_writable_field(static_cast<WritableStateFieldBase*>(&override_f));
    return added;
}

void Fault::signal() {
    if (cc > last_fault_time) {
        num_consecutive_faults++;
        last_fault_time = cc;
    }

    if (num_consecutive_faults > persistence) {
        set(true);
    }
}

void Fault::unsignal() {
    set(false);
    num_consecutive_faults = 0;
}

bool Fault::is_faulted() const {
    if (override_f.get()) return true;
    else if (suppress_f.get()) return false;
    else return get();
}
