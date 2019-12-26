#ifndef FAULT_HPP_
#define FAULT_HPP_

#include <StateFieldRegistry.hpp>

class Fault : public ReadableStateField<bool> {
  public:
    Fault(const std::string& name,
          const size_t _persistence, unsigned int& control_cycle_count,
          const bool default_setting = false);

    /**
     * @brief Add fault-related flags to the registry.
     */
    bool add_to_registry(StateFieldRegistry& r);

    /**
     * @brief Client-facing function to signal an occurrence of the
     * fault-related condition.
     */
    void signal();

    /**
     * @brief Client-facing function to signal a non-occurrence of the
     * fault-related condition. Resets the persistence of the fault.
     */
    void unsignal();

    /**
     * @brief If the fault-related condition has been persistent or if
     * the fault is overridden by the ground, the fault is signaled.
     * Otherwise, if the fault-related condition has not been persistent,
     * or if the fault is suppressed by the ground, the fault is not signaled.
     *
     * @return true 
     * @return false 
     */
    bool is_faulted() const;

  private:
    using ReadableStateField<bool>::set;
    using ReadableStateField<bool>::get;

    unsigned int cc; // Control cycle count
    long int last_fault_time = -1;

    unsigned int persistence;
    unsigned int num_consecutive_faults = 0;

    /**
     * @brief State fields that can be set by the ground to suppress
     * or forcibly signal the fault.
     */
    WritableStateField<bool> suppress_f;
    WritableStateField<bool> override_f;
};

#endif
