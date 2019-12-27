#include "PropController.h"

PropController::PropController(StateFieldRegistry &registry, unsigned int offset) : 
    TimedControlTask<void>(registry, "prop", offset),
    prop_state_f("prop.state"),
    prop_mode_f("prop.mode"),
    prop_fault_f("prop.fault"),
    fire_time_f("prop.fire_time"),
    delta_thrust_times_f("prop.thrust_vector")
{ 
    add_internal_field(prop_state_f);
    add_internal_field(prop_mode_f);
    add_internal_field(prop_fault_f);
    add_internal_field(fire_time_f);
    add_internal_field(delta_thrust_times_f);

    prop_state_f.set(prop_state_t::idle);
    prop_mode_f.set(prop_mode_t::disabled);
    prop_fault_f.set(prop_fault_t::nofaults);
    fire_time_f.set(0);
    delta_thrust_times_f.set(thrust_vector);
}

void PropController::execute()
{
    bool is_recoverable = true;
    switch (prop_mode_f.get()) 
    {
        case prop_mode_t::disabled:
            return;
        case prop_mode_t::active:
            is_recoverable = detect_and_set_faults();
            break;
        default:
            printf(debug_severity::error, "Prop mode not defind: %d", prop_mode_f.get());
    }

    if ( !is_recoverable )
    {
        prop_mode_f.set(prop_mode_t::disabled);
        return;
    }

    switch (prop_state_f.get()) 
    {
        case prop_state_t::idle:
            return dispatch_idle();
        case prop_state_t::pressurizing:
            return dispatch_pressurizing();
        case prop_state_t::firing:
            return dispatch_firing();
        case prop_state_t::handling_fault:
            return dispatch_handling_fault();
        default:
            printf(debug_severity::error, "Prop state not defined: %d", prop_state_f.get());
    }
}


void PropController::dispatch_idle()
{
    prop_time_t delta = abs(now() - fire_time_f.get());
    // Return if there are more than 5 minutes between now and when we have to fire
    if ( delta > convert_min(prop_constants.minutes_until_pressurize) )
        return;
    // Start pressurizing
    reset_state_vars();
    prop_state_f.set(prop_state_t::pressurizing);
}

// ---------------------------------------------------------------------------
// Pressurizing
// ---------------------------------------------------------------------------

void PropController::dispatch_pressurizing()
{
    // If inner valves are open
    if ( state_vars.inner_open )
    {
        if ( done_filling() )
            stop_filling();
    }
    else // If inner valves are closed
    {
        if ( done_pressurizing() )
        {
            prop_state_f.set(prop_state_t::firing);
            // TODO: enable interrupts
        }
        else if ( have_fills_left() ) 
        {
            start_filling();
        }
        else 
        {
            prop_fault_f.set(prop_fault_t::cannot_refill);
        }
    }
}

bool PropController::done_filling()
{
    prop_time_t delta = now() - state_vars.inner_start_time;
    return delta >= convert_sec(prop_constants.inner_open_duration);
}

void PropController::stop_filling()
{
    close_inner(0);
    close_inner(1);
}

bool PropController::have_fills_left()
{
    return state_vars.num_fills <= prop_constants.max_num_fills;
}

void PropController::start_filling()
{
    prop_time_t delta = now() - state_vars.inner_start_time;
    // Return if we have fired outer valves within the last 10 seconds
    if (delta < convert_sec(prop_constants.wait_time_between_fills))
        return;

    ++state_vars.num_fills;
    state_vars.inner_start_time = open_inner(0);
    open_inner (1);
    state_vars.inner_open = true;
}

// ---------------------------------------------------------------------------
// Firing
// ---------------------------------------------------------------------------

void PropController::dispatch_firing()
{
    if ( state_vars.done_firing )
        prop_state_f.set(prop_state_t::idle);
}

void PropController::handle_sigfire()
{
    // TODO: disable interrupts
    if ( state_vars.firing )
    {
        if ( done_firing() )
            state_vars.done_firing = true;
        else 
            check_progress();
    }
    else
    {
        if ( ready_to_fire() )
            start_firing();
    }   
    // TODO: enable interrupts
}

bool PropController::done_firing()
{
    bool done = true;
    for (size_t i = 0; i < 4; ++i)
        done &= state_vars.outers_done[i];
    return done;
}

void PropController::check_progress()
{
    for (size_t i = 0; i < 4; ++i)
    {
        if ( !state_vars.outers_done[i] )
        {
            if ( abs(now() - state_vars.outer_start_times[i]) >= thrust_vector[i] )
            {
                close_outer(i);
                state_vars.outers_done[i] = true;
            }
        }
    }
}

bool PropController::ready_to_fire()
{
    return abs( fire_time_f.get() - now() ) < convert_ms(5);
}

void PropController::start_firing()
{
    for (size_t i = 0; i < 4; ++i)
    {
        state_vars.outer_start_times[i] = open_outer(i);
    }
}

// ---------------------------------------------------------------------------
// Handle Faults
// ---------------------------------------------------------------------------

void PropController::dispatch_handling_fault()
{
    switch (prop_fault_f.get())
    {
        case prop_fault_t::nofaults:
            prop_state_f.set(prop_state_t::idle);
            break;
        case prop_fault_t::outer_temp:
            break;
        case prop_fault_t::inner_temp:
            break;
        case prop_fault_t::outer_pressure:
            break;
        case prop_fault_t::cannot_refill:
            break;
        default:
            printf(debug_severity::error, "Prop fault not found %d", prop_fault_f.get());
    }
}

bool PropController::detect_and_set_faults()
{
    
}