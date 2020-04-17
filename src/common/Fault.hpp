#ifndef FAULT_HPP_
#define FAULT_HPP_

#include "common/StateField.hpp"

class Fault : public WritableStateField<bool> {
  protected:
   const std::string _name;
  
  public:
    /**
     * @brief Construct a new non-latching fault.
     * 
     * @param name Name of base fault state field.
     * @param _persistence Persistence threshold before signaling fault.
     * @param control_cycle_count Reference to the control cycle count.
     */
    Fault(const std::string& name, const size_t _persistence);

    const std::string &name() const override { return _name; } 

    /**
     * @brief Calls signal if flag is true and unsignal if flag is false.
     * 
     * @param flag 
     */
    void evaluate(bool flag);

    /**
     * @brief Client-facing function to signal an occurrence of the
     * fault-related condition. Attempts to increment num_consecutive_signals by 1
     */
    void signal();

    /**
     * @brief Client-facing function to signal a non-occurrence of the
     * fault-related condition. Resets the num_consecutive_signals to 0
     */
    void unsignal();

    /**
     * @brief Client-facing functions to override or suppress the fault behavior.
     * These exist for use in test fixtures to make it easy to signal or
     * unsignal faults from a test perspective.
     */
  #ifdef UNIT_TEST
    void override();
    void un_override();
    void suppress();
    void unsuppress();
  #endif

    /**
     * @brief Calls process_commands, sets the fault true if num_consecutive_faults > persistence_f
     * returns true or false if override or suppress are true respectivly,
     * Otherwise, it returns the state of the fault itself.
     *
     * @return true if: override is true, or num_consecutive_faults > persistence_f
     * @return false if: suppress is true, or num_consecutive_faults < persistence_f 
     */
    bool is_faulted();

    #ifdef UNIT_TEST
    /**
     * @brief a debug return that tells the current consecutive signals
     * 
     * @return unsigned int 
     */
    unsigned int get_num_consecutive_signals();
    #endif

    /**
     * @brief State fields that can be set by the ground to
     * suppress, override, signal and unsignal the fault
     */
    Serializer<bool> fault_bool_sr;
    WritableStateField<bool> suppress_f;
    WritableStateField<bool> override_f;
    WritableStateField<bool> unsignal_f;

    Serializer<unsigned int> persist_sr;
    WritableStateField<unsigned int> persistence_f;
    
    static const unsigned int* cc; // Control cycle count

  private:
    // Make the get() and set() methods of the state field private,
    // so that the user is forced to use the signal() and unsignal()
    // methods instead.
    using WritableStateField<bool>::set;
    using WritableStateField<bool>::get;
    unsigned int last_fault_time = 0; // Last control cycle # that the fault condition
                                      // occurred

    mutable unsigned int num_consecutive_signals = 0; // Number of consecutive signal condition
                                                     // occurrences at the current moment.
    
    // Keeps track of the previous suppress or override state
    bool prev_suppress = false;
    bool prev_override = false;

    /**
     * @brief Resets consecutive faults to 0, whenever pulls transition from false to true,
     * or if unsignal_f is true.
     */
    void process_commands();
};

#endif