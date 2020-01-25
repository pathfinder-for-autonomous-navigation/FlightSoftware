#ifndef FAULT_HPP_
#define FAULT_HPP_

#include <common/StateFieldRegistry.hpp>

class Fault : public WritableStateField<bool> {
  public:
    /**
     * @brief Construct a new latching fault.
     * 
     * @param name Name of base fault state field.
     * @param _persistence Persistence threshold before signaling fault.
     * @param control_cycle_count Reference to the control cycle count.
     * @param default_setting Default setting of fault signal.
     */
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
    // Make the get() and set() methods of the state field private,
    // so that the user is forced to use the signal() and unsignal()
    // methods instead.
    using WritableStateField<bool>::set;
    using WritableStateField<bool>::get;

    unsigned int& cc; // Control cycle count
    unsigned int last_fault_time = 0; // Last control cycle # that the fault condition
                                      // occurred

    unsigned int persistence; // Persistence threshold for fault signal
    mutable unsigned int num_consecutive_faults = 0; // Number of consecutive fault condition
                                                     // occurrences at the current moment.

    /**
     * @brief State fields that can be set by the ground to suppress
     * or forcibly signal the fault.
     */
    WritableStateField<bool> suppress_f;
    WritableStateField<bool> override_f;
};

#endif
