#pragma once
#define DESKTOP // temporary because VScode is bad
#include "TimedControlTask.hpp"
#include "propulsion_mode_t.enum"
#include "propulsion_state_t.enum"
#include "propulsion_fault_t.enum"

/**
 * Propulsion Controller
 * 
 * Two inner valves: x, y
 * Four outer valves: 0, 1, 2, 3
 * 
 * External Fields:
 *  fire_time_f - time at which we wish to fire outer valves
 *  delta_thrust_times_f - duration to open each outer valve at fire_time
 * 
 * SIG_FIRE enabled only in the firing state
 * Check for faults/problems every control cycle
 * 
 */
class PropController : public TimedControlTask<void> {

    using prop_time_t = long long; // units of time
    int SIG_FIRE = SIGINT; // placeholder interrupt

    public:
    PropController(StateFieldRegistry& registry, unsigned int offset);

    void execute() override;

    InternalStateField<prop_state_t> prop_state_f;

    InternalStateField<prop_mode_t> prop_mode_f;

    InternalStateField<prop_fault_t> prop_fault_f;

    /**
     * time at which we wish to fire
     * @requires: fire_time_f must be at least 10 minutes in the future
     */
    InternalStateField<prop_time_t> fire_time_f; 

    /**
     * duration that each valve is to be kept open at fire_time
     * @requires: fire times must be less than 1s
     */
    InternalStateField<prop_time_t[4]> delta_thrust_times_f;

#ifndef DEBUG
    protected:
#endif
    void dispatch_idle();
    void dispatch_pressurizing();
    void dispatch_firing();
    void dispatch_handling_fault();

    /**
     * dispatch functions for handling faults
     */
    void handle_outer_temp();       // outer tank temperature too high
    void handle_inner_temp();       // inner tank temperature too high
    void handle_high_pressure();    // outer tank pressure too high
    void handle_refill_fail();      // failed to refill outer tank

#ifndef DEBUG
    private:
#endif
    
    /**
     * Detects faults
     * @return false if unrecoverable fault encountered; true otherwis
     * Side Effect: 
     *    May change the state to handling_fault and may change prop_fault_f
     */
    bool detect_and_set_faults();

    /**
     * Opens an inner valve and returns the time at which the valve was opened
     */
    prop_time_t open_inner(size_t valve_num);
    prop_time_t open_outer(size_t valve_num);
    void close_inner(size_t valve_num);
    void close_outer(size_t valve_num);

    /**
     * Resets the state variables when we are done with a firing
     */
    void reset_state_vars();

// ---------------------------------------------------------------------------
// Helper functions for timing
// ---------------------------------------------------------------------------
    /**
     * Convert to prop_time_t units from minutes, seconds, milliseconds
     */
    prop_time_t convert_min(double time);
    prop_time_t convert_sec(double time);
    prop_time_t convert_ms(double time);

    /**
     * Returns the current time in units of prop_time_t
     */
    prop_time_t now();

// ---------------------------------------------------------------------------
// Pressurizing Functions
// ---------------------------------------------------------------------------
    /**
     * Return true if we are done with one fill: 
     *      inner valves have been opened for at least 1s
     */
    bool done_filling();

    /**
     * Close the inner valves
     */ 
    void stop_filling();

    /**
     * Return true if tank pressure >= firing thresh_hold
     */
    bool done_pressurizing();

    /**
     * Return true if we have not exceeded the maximum number of fills
     */
    bool have_fills_left();

    /** 
     * Open the inner valves 
     */
    void start_filling();

// ---------------------------------------------------------------------------
// Firing Functions
// ---------------------------------------------------------------------------

    /**
     * Function that is called when device encounters SIG_FIRE
     */
    void handle_sigfire();

    /**
     * Returns true if we are ready to fire the outer valves: we are within
     * 5ms of fire_time
     */
    bool ready_to_fire();

    /**
     * Opens the outer valves
     * @requires: outer valves have not yet started to fire
     */
    void start_firing();

    /**
     * Returns true if ALL outer valves have finished firing
     */
    bool done_firing();

    /**
     * Check each valve to see if they are finished firing
     * @requires: outer valves have started firing
     */
    void check_progress();

// ---------------------------------------------------------------------------
// Data fields
// ---------------------------------------------------------------------------
    
    /**
     * Holds the duration that each inner valve should open when it is time to fire
     */
    prop_time_t thrust_vector[4];

    /**
     * Variables used only for the state machine
     */
    struct _state_vars{
        bool firing = 0;
        bool done_firing = 0;
        bool outers_done[4] = {0, 0, 0, 0};
        prop_time_t outer_start_times[4] = {0, 0, 0, 0};
        prop_time_t inner_start_time = 0;
        bool inner_open = 0;
        size_t num_fills = 0;

    } state_vars;

    /**
     * Constants
     */
    static const struct _prop_constants {
        // start pressurizing when we are within 5 min of fire_time
        prop_time_t minutes_until_pressurize = 5;
        // do not open inner valves more than 20 times during pressurizing state
        prop_time_t max_num_fills = 20;
        // wait 10 seconds between opening inner valves
        prop_time_t wait_time_between_fills = 10;
        // open inner valves for 1s
        prop_time_t inner_open_duration = 1; 
        // open inner valves for 2s if one of the inner valves stuck close
        prop_time_t inner_open_duration_bak = 2;
        // SIG_FIRE interrupt every 5 milliseconds
        prop_time_t sigfire_interval = 5;
    } prop_constants;

};