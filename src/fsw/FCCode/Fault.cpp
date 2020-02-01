#include "Fault.hpp"

Fault::Fault(const std::string& name,
      const size_t _persistence, unsigned int& control_cycle_count,
      const bool default_setting) :
    WritableStateField<bool>(name, Serializer<bool>()),
    cc(control_cycle_count),
    fault_bool_sr(),
    suppress_f(name + ".suppress", fault_bool_sr),
    override_f(name + ".override", fault_bool_sr),
    unsignal_f(name + ".unsignal", fault_bool_sr),
    // 65536 = 2^16 -1
    persist_sr(0, 65535, 16),
    persistence_f(name + ".persistence", persist_sr)
{
  set(default_setting);
  override_f.set(false);
  suppress_f.set(false);
  unsignal_f.set(false);
  persistence_f.set(_persistence);
}

bool Fault::add_to_registry(StateFieldRegistry& r) {
    if (!r.add_writable_field(static_cast<WritableStateFieldBase*>(this))) return false;
    if (!r.add_writable_field(static_cast<WritableStateFieldBase*>(&suppress_f))) return false;
    if (!r.add_writable_field(static_cast<WritableStateFieldBase*>(&override_f))) return false;
    if (!r.add_writable_field(static_cast<WritableStateFieldBase*>(&unsignal_f))) return false;
    return true;
}

void Fault::signal() {
    if (cc > static_cast<unsigned int>(last_fault_time) || cc == 0) {
        num_consecutive_signals++;
        last_fault_time = cc;
    }

    if (num_consecutive_signals > persistence_f.get()) {
        set(true);
    }
    else
        set(false);

    process_commands();
}

void Fault::unsignal() {
    set(false);
    num_consecutive_signals = 0;
    process_commands();
}

bool Fault::is_faulted() const {
    if (override_f.get()) {
        return true;
    }
    else if (suppress_f.get()) {
        return false;
    }
    else return get();
}

void Fault::process_commands(){
    if(prev_override == false && override_f.get()){
        num_consecutive_signals = 0;
    }
    if(prev_suppress == false && suppress_f.get()){
        num_consecutive_signals = 0;
    }
    prev_override = override_f.get();
    prev_suppress = suppress_f.get();

    if(unsignal_f.get()){
        unsignal_f.set(false);
        unsignal();
    }
}

#ifdef UNIT_TEST
unsigned int Fault::get_num_consecutive_signals(){
    return num_consecutive_signals;
}
#endif