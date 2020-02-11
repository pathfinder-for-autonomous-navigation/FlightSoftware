#pragma once
#include <fsw/FCCode/TimedControlTask.hpp>
#include <fsw/FCCode/Drivers/PropulsionSystem.hpp>
#include <fsw/FCCode/prop_state_t.enum>

class PropController : public TimedControlTask<void> {
  public:
  PropController(StateFieldRegistry& registry, unsigned int offset);

  // ------------------------------------------------------------------------
  // Public Interface
  // ------------------------------------------------------------------------
  void execute() override;

  void set_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now);

  // Dangerous: prop_state_f can be changed at anytime by outside components
  WritableStateField<unsigned int>prop_state_f;

  WritableStateField<unsigned int>fire_cycle_f;
  WritableStateField<unsigned int>sched_valve1_f;
  WritableStateField<unsigned int>sched_valve2_f;
  WritableStateField<unsigned int>sched_valve3_f;
  WritableStateField<unsigned int>sched_valve4_f;

  private:
  // ------------------------------------------------------------------------
  // State Handling Functions
  // ------------------------------------------------------------------------

  // Prop is only willing to transition to idle
  void dispatch_disabled();
  // Prop is willing to start pressurizing
  void dispatch_idle();
  // Tank1 valves are open
  void dispatch_pressurizing();
  // Both Tank1 and Tank2 valves are open
  void dispatch_venting();
  // Tank2 valves are currently open
  void dispatch_firing();
  // Tank2 pressure must be at the threshold_pressure but has not yet fired
  void dispatch_await_firing();
  
  void dispatch_handling_fault();

  // ------------------------------------------------------------------------
  // State Functions
  // ------------------------------------------------------------------------
  inline void change_state(prop_state_t new_state)
  {
    prop_state_f.set(static_cast<unsigned int>(new_state));
  }

  // check our state
  inline bool assert_state(prop_state_t expected_state)
  {
    return static_cast<prop_state_t>(prop_state_f.get()) == expected_state;
  }

  // return true if tank2 is at threshold pressure
  inline static bool is_at_threshold_pressure()
  {
    static constexpr float threshold_pressure = 25;
    return Tank2.get_pressure() >= threshold_pressure;
  }

  // return true if we can feasibly fire at the current schedule
  bool is_schedule_good();

  // ------------------------------------------------------------------------
  // Fault Handling Functions
  // ------------------------------------------------------------------------
  void handle_unexpected_state();
  void handle_bad_schedule();

  // ------------------------------------------------------------------------
  // Propulsion States
  // ------------------------------------------------------------------------
  // Abstraction of a prop state - everything is private
  class PropState
  {
    protected:
    // called when entering this state
    virtual void entry_protocol() = 0;
    // called when we have been in this state and want to determine
    // whether we should transition
    virtual prop_state_t next_state() = 0;
  };

  #define PropIdle PropController::_PropIdle::Instance()
  class _PropIdle : public PropState
  {
    private:
    _PropIdle();
    inline static _PropIdle & Instance()
    {
      static _PropIdle Instance;
      return Instance;
    }
    void entry_protocol() override;
    prop_state_t next_state() override;
  };

  // A pressurizing cycle is a 1 second duration in which an intertank valve is opened
  // A single pressurizing cycle will span multiple control cycles
  // If we have executed 20 consecutive pressurizing cycles and have not yet reached
  // threshold pressure, then this is a fault
  #define Pressurizing PropController::_Pressurizing::Instance()
  class _Pressurizing : public PropState
  {
      private:
    _Pressurizing();
    inline static _Pressurizing& Instance()
    {
        static _Pressurizing Instance;
        return Instance;
    }
    void entry_protocol() override;
    prop_state_t next_state() override;
    // maximum number pressurizing cycles allowed
    static unsigned int const max_cycles = 20;
    private:
    // Called when prop is currently in a pressurizing cycle (i.e. valve is open)
    void handle_currently_pressurizing();
    // Called when prop has failed to reach threshod_pressure after maximum consecutive pressurizing cycles
    void handle_pressurize_failed();
    // Starts another pressurization cycle
    void start_pressurize_cycle();
    // Returns true if we should use the backup valve for pressurizing
    static bool should_use_backup();
    // The number of pressurizing cycles we have executed since entering this state
    unsigned int current_cycle = 0;
    // 1 if we are using the backup valve and 0 otherwise
    bool valve_num = 0;
    // micros() time at which we opened Tank1 valve
    unsigned int cycle_start_time = 0;
  };

  #define Firing PropController::_Firing::Instance()

  class _Firing : public PropState
  {
    private:
    _Firing();
    inline static _Firing& Instance()
    {
        static _Firing Instance;
        return Instance;
    }
    void entry_protocol() override;
    prop_state_t next_state() override;
    // Returns true if the entire schedule is all zeros
    bool is_schedule_empty();
  };

  #define Await_Firing PropController::_Await_Firing::Instance()

  class _Await_Firing : public PropState
  {
    private:
    _Await_Firing();
    inline static _Await_Firing& Instance()
    {
      static _Await_Firing Instance;
      return Instance;
    }
    void entry_protocol() override;
    prop_state_t next_state() override;
    // returnt true if schedule and start time is feasible
    static bool is_schedule_valid();
  };

  // pointer to the current prop state
  static PropState* prop_state;

};
