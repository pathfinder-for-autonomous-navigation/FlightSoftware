#include "../custom_assertions.hpp"
#include <fsw/FCCode/PropController.hpp>
#include <fsw/FCCode/PropFaultHandler.h>
#include <fsw/FCCode/prop_state_t.enum>
#include "../StateFieldRegistryMock.hpp"
#include "fsw/FCCode/OrbitController.hpp"

#define assert_fault_state(state, x) TEST_ASSERT_EQUAL(state, tf.pc->x.is_faulted())

#define check_state(expected) TEST_ASSERT_EQUAL(expected, tf.pc->prop_state_f.get())

#define suppress_fault(fault) tf.pc->fault.suppress_f.set(true)

#define unsignal_fault(fault) tf.pc->fault.unsignal_f.set(true)

#define override_fault(fault) tf.pc->fault.override_f.set(true)

#define get_persistence(fault) tf.pc->fault.persistence_f.get()

inline void
do_nothing()
{
}
// See pressure_data.txt and temp_data.txt in test/test_fsw_prop_controller for
//  the analogRead --> sensor conversions

inline void simulate_ambient()
{
    // This is the ambient readings
    Tank2.fake_tank2_pressure_high_read = 305; // 14.6 psi
    Tank2.fake_tank2_pressure_low_read = 80;   // doesn't really matter
    Tank1.fake_tank1_temp_sensor_read = 163;   // 20 C
    Tank2.fake_tank2_temp_sensor_read = 163;   // 20 C
}

inline void simulate_at_threshold()
{
    Tank2.fake_tank2_pressure_high_read = 520;
}

inline void simulate_pressure_rising()
{
    // When tank2_pressure_high 520 --> 25.043261 psi
    // Starting at ambient 305, expect to reach pressurized state at 520-305 = 215 cycles
    Tank2.fake_tank2_pressure_high_read++;
}
// Expected response: Open all 4 thruster valves to vent Tank 2, 10 1-second bursts separated by 1 second
inline void simulate_overpressured()
{
    // Spoof an overpressured event
    Tank2.fake_tank2_pressure_high_read = 1000; // values above 1000 are ignored
    Tank2.fake_tank2_pressure_low_read = 853;   // 84.527626 psi
}
// Expected response: Open 2 tank-to-tank valves for 10 1-second bursts separated by 1 second
inline void simulate_tank1_high()
{
    std::printf("Spoofing Tank1 high\n");
    // Spoof a tank1 high event
    Tank1.fake_tank1_temp_sensor_read = 35; // 69 C
}

// Expected response: Open all 4 thruster valves to vent Tank 2, 10 1-second bursts separated by 1 second
inline void simulate_tank2_high()
{
    std::printf("Spoofing Tank2 high\n");
    Tank2.fake_tank2_temp_sensor_read = 50; // 47 C
}

class TestFixture
{
public:
    unsigned int &cc = TimedControlTaskBase::control_cycle_count;
    using FnVoid_t = void (*)(); // pointer to a void function

    StateFieldRegistryMock registry;

    std::unique_ptr<PropController> pc;
    std::unique_ptr<PropFaultHandler> pfh;

    TestFixture()
    {
        cc = 0;
        Fault::cc = &cc;
        auto orbit_controller = std::make_unique<OrbitController>(registry);
        pc = std::make_unique<PropController>(registry);
        pfh = std::make_unique<PropFaultHandler>(registry);
        simulate_ambient();
    }

    inline int ctrl_cycles_per_pressurizing_cycle()
    {
        return pc->ctrl_cycles_per_filling_period.get() + pc->ctrl_cycles_per_cooling_period.get();
    }

    inline void set_state(prop_state_t state)
    {
        pc->prop_state_f.set(static_cast<unsigned int>(state));
    }

    inline void set_schedule(size_t v1, size_t v2, size_t v3, size_t v4, size_t cycles_until_firing)
    {
        pc->sched_valve1_fp->set(v1);
        pc->sched_valve2_fp->set(v2);
        pc->sched_valve3_fp->set(v3);
        pc->sched_valve4_fp->set(v4);
        pc->cycles_until_firing.set(cycles_until_firing);
    }

    inline void check_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4,
                               unsigned int ctrl_cycles_from_now)
    {
        TEST_ASSERT_EQUAL(v1, pc->sched_valve1_fp->get());
        TEST_ASSERT_EQUAL(v2, pc->sched_valve2_fp->get());
        TEST_ASSERT_EQUAL(v3, pc->sched_valve3_fp->get());
        TEST_ASSERT_EQUAL(v4, pc->sched_valve4_fp->get());
        TEST_ASSERT_EQUAL(pc->cycles_until_firing.get(), ctrl_cycles_from_now);
    }

    // Step forward the state machine by num control cycle.
    inline void step(size_t num = 1)
    {
        for (size_t i = 0; i < num; ++i)
        {
            pfh->execute();
            pc->execute();
        }

        if (PropulsionSystem.is_firing())
        {
            for (size_t i = 0; i < 4; ++i)
            {
                // decrement each schedule as if we were thrust valve loop
                if (Tank2.schedule[i] > PAN::control_cycle_time_ms)
                    Tank2.schedule[i] -= PAN::control_cycle_time_ms;
                else
                    Tank2.schedule[i] = 0;
            }
        }
    }

    // Keep stepping until the state changes (or until we have step max_cycles steps)
    // Return the number of steps taken or return 6969696969 if there has been no state change after max_cycles steps
    inline size_t execute_until_state_change(size_t max_cycles = 2 * 2048)
    {
        size_t num_steps = 0;
        unsigned int current_state = pc->prop_state_f.get();
        while (current_state == pc->prop_state_f.get() && num_steps < max_cycles)
        {
            ++num_steps;
            step();
        }
        return (num_steps < max_cycles) ? num_steps : 6969696969;
    }

    // Execute fn1 for num_fn1_cycles cycles then execute fn2 and return
    inline void execute_step(FnVoid_t fn1, size_t num_fn1_cycles, FnVoid_t fn2)
    {
        for (size_t i = 0; i < num_fn1_cycles - 1; ++i)
        {
            fn1();
            step();
        }
        fn2();
        // This is to account for the fact that sensors are read at the end of the control cycle
        step();
    }

    ~TestFixture()
    {
        // Reset the prop between tests
        PropulsionSystem.reset();
        // Reset all state variables
        PropController::state_venting.tank_choice = 1;
        PropController::state_venting.saved_tank2_valve_choice = 0;
    }

    inline void simulate_underpressured()
    {
        // Set prop to idle and then set a schedule
        set_state(prop_state_t::idle);
        set_schedule(200, 800, 900, 800, pc->min_cycles_needed());

        // Prop should fail to pressurize since pressure is still ambient
        step(pc->min_cycles_needed());
    }

    // The following helper functions will set the sensor readings
    inline void simulate_await_pressurizing()
    {
        set_state(prop_state_t::idle);
        set_schedule(200, 800, 900, 800, pc->min_cycles_needed() + 1);
        step(); // we should now be in await_pressurizing
        TEST_ASSERT_EQUAL(prop_state_t::await_pressurizing, pc->prop_state_f.get());
    }

    inline void simulate_pressurizing()
    {
        set_state(prop_state_t::idle);
        set_schedule(200, 800, 900, 800, pc->min_cycles_needed() + 1);
        step(); // we should now be in await_pressurizing
        step(); // we should now be in pressurizing
        TEST_ASSERT_EQUAL(prop_state_t::pressurizing, pc->prop_state_f.get());
    }

    inline void simulate_await_firing()
    {
        set_state(prop_state_t::idle);
        set_schedule(200, 800, 900, 800, pc->min_cycles_needed());
        step();
        simulate_at_threshold();
        execute_until_state_change();
        TEST_ASSERT_EQUAL(prop_state_t::await_firing, pc->prop_state_f.get());
    }

    inline void simulate_firing()
    {
        set_state(prop_state_t::idle);
        set_schedule(200, 800, 900, 800, pc->min_cycles_needed() + 1);
        step();
        step();
        simulate_at_threshold();
        execute_until_state_change();
        execute_until_state_change();
        TEST_ASSERT_EQUAL(prop_state_t::firing, pc->prop_state_f.get());
    }
};
