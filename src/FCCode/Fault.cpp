#include "Fault.hpp"

Fault::Fault(const std::string& name,
      const size_t _persistence, unsigned int& control_cycle_count,
      const bool default_setting) :
    WritableStateField<bool>(name, Serializer<bool>()),
    cc(control_cycle_count),
    persistence(_persistence),
    suppress_f(name + ".suppress", Serializer<bool>()),
    override_f(name + ".override", Serializer<bool>())
{
  set(default_setting);
  override_f.set(false);
  suppress_f.set(false);
}

bool Fault::add_to_registry(StateFieldRegistry& r) {
    if (!r.add_writable_field(static_cast<WritableStateFieldBase*>(this))) return false;
    if (!r.add_writable_field(static_cast<WritableStateFieldBase*>(&suppress_f))) return false;
    if (!r.add_writable_field(static_cast<WritableStateFieldBase*>(&override_f))) return false;
    return true;
}

void Fault::signal() {
    if (cc > static_cast<unsigned int>(last_fault_time) || cc == 0) {
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
    if (override_f.get()) {
        num_consecutive_faults = 0;
        return true;
    }
    else if (suppress_f.get()) {
        num_consecutive_faults = 0;
        return false;
    }
    else return get();
}
