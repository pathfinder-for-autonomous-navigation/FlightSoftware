#include "Fault.hpp"

const unsigned int* Fault::cc = nullptr;
 
Fault::Fault(const std::string& name, const size_t _persistence) : 
    WritableStateField<bool>(name, Serializer<bool>()),
    _name(name + ".base"),
    fault_bool_sr(),
    suppress_f(name + ".suppress", fault_bool_sr),
    override_f(name + ".override", fault_bool_sr),
    unsignal_f(name + ".unsignal", fault_bool_sr),
    // 65536 = 2^16 -1
    persist_sr(65535),
    persistence_f(name + ".persistence", persist_sr)
{
  set(false);
  override_f.set(false);
  suppress_f.set(false);
  unsignal_f.set(false);
  persistence_f.set(_persistence);
}

void Fault::evaluate(bool flag) {
    if (flag) signal();
    else unsignal();
}

void Fault::signal() {
    if (*cc > static_cast<unsigned int>(last_fault_time) || *cc == 0) {
        num_consecutive_signals++;
        last_fault_time = *cc;
    }
}

void Fault::unsignal() {
    num_consecutive_signals = 0;
}

#ifdef UNIT_TEST
void Fault::override() {
    override_f.set(true);
}
void Fault::un_override() {
    override_f.set(false);
}
void Fault::suppress() {
    suppress_f.set(true);
}
void Fault::unsuppress() {
    suppress_f.set(false);
}
#endif

bool Fault::is_faulted() {
    process_commands();

    if (num_consecutive_signals > persistence_f.get()) {
        set(true);
    }
    else
        set(false);

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
        num_consecutive_signals = 0;
    }
}

#ifdef UNIT_TEST
unsigned int Fault::get_num_consecutive_signals(){
    return num_consecutive_signals;
}
#endif