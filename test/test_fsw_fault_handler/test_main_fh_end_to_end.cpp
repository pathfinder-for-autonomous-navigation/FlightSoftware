#include "test_fault_handlers.hpp"
#include "test_fixture_main_fh.hpp"

////////////// Helper utilities ////////////////

void signal_until_persistence(std::shared_ptr<Fault>& fault_ptr) {
    for(int i = 0; i < fault_ptr->persistence_f.get(); i++)
        fault_ptr->signal();
}

//////////// End helper utilities //////////////

static void test_no_faults() {
    TestFixtureMainFHEndToEnd tf;
    tf.set(mission_state_t::standby);
    TEST_ASSERT_EQUAL(fault_response_t::none, tf.step());
}

void test_single_simple_faults(std::vector<std::shared_ptr<Fault>>& faults,
    fault_response_t expected_response, TestFixtureMainFHEndToEnd& tf)
{
    for(auto& fault_ptr : faults) {
        signal_until_persistence(fault_ptr);
        TEST_ASSERT_EQUAL(fault_response_t::none, tf.step());
        fault_ptr->signal();
        TEST_ASSERT_EQUAL(expected_response, tf.step());
        fault_ptr->unsignal();
        TEST_ASSERT_EQUAL(fault_response_t::none, tf.step());
    }
}

void test_single_simple_safehold_faults() {
    TestFixtureMainFHEndToEnd tf;
    std::vector<std::shared_ptr<Fault>> safehold_faults = {
        tf.adcs_wheel1_adc_fault_fp,
        tf.adcs_wheel2_adc_fault_fp,
        tf.adcs_wheel3_adc_fault_fp,
        tf.adcs_wheel_pot_fault_fp,
        tf.low_batt_fault_fp,
        tf.prop_overpressure_fault_fp
    };

    /** 
     * When the mission state is one where these faults are active,
     * the main fault handler should recommend safehold.
     **/
    tf.set(mission_state_t::standby);
    test_single_simple_faults(safehold_faults, fault_response_t::safehold, tf);

    /** 
     * When the mission state is one where these faults are inactive,
     * the main fault handler should recommend nothing.
     **/
    tf.set(mission_state_t::startup);
    test_single_simple_faults(safehold_faults, fault_response_t::none, tf);
}

void test_single_simple_standby_faults() {
    TestFixtureMainFHEndToEnd tf;
    std::vector<std::shared_ptr<Fault>> standby_faults = {
        tf.prop_failed_pressurize_fault_fp
    };

    /**
     * When the mission state is one where these faults are active,
     * the main fault handler should recommend safehold.
     **/
    tf.set(mission_state_t::standby);
    test_single_simple_faults(standby_faults, fault_response_t::standby, tf);

    /**
     * When the mission state is one where these faults are inactive,
     * the main fault handler should recommend nothing.
     **/
    tf.set(mission_state_t::startup);
    test_single_simple_faults(standby_faults, fault_response_t::none, tf);
}

void test_single_quake_fault() {
    // Set initial conditions: haven't had comms for more than 24 hours
    TestFixtureMainFHEndToEnd tf;
    tf.cc = PAN::one_day_ccno + 1;
    tf.set_last_comms_ccno(0);
    tf.set(radio_state_t::wait);

    // Initial response should be standby
    TEST_ASSERT_EQUAL(fault_response_t::standby, tf.step());

    // Continue stepping through the Quake fault state machine until we reach safehold.
    tf.cc = 2 * PAN::one_day_ccno + 1;
    TEST_ASSERT_EQUAL(fault_response_t::standby, tf.step()); // Now in powercycle_1
    tf.cc = 7 * PAN::one_day_ccno / 3 + 1;
    TEST_ASSERT_EQUAL(fault_response_t::standby, tf.step()); // Now in powercycle_2
    tf.cc = 8 * PAN::one_day_ccno / 3 + 1;
    TEST_ASSERT_EQUAL(fault_response_t::standby, tf.step()); // Now in powercycle_3
    tf.cc = 3 * PAN::one_day_ccno / 3 + 1;
    TEST_ASSERT_EQUAL(fault_response_t::safehold, tf.step()); // Now in safehold
}

void test_single_piksi_fault() {
    // TODO
}

void test_two_simple_faults_safehold_and_standby() {
    /**
     * These next two tests check that if two faults are occurring simultaneously,
     * one with a safehold response and one with a standby response, the stronger 
     * response (safehold) is the one that dominates. Then, these two tests check 
     * that if one of the faults goes away, the one remaining is the one whose fault
     * response is applied.
     **/
    {
        TestFixtureMainFHEndToEnd tf;

        // If both faults occur simultaneously, the stronger
        // response is recommended.
        signal_until_persistence(tf.prop_failed_pressurize_fault_fp);
        signal_until_persistence(tf.adcs_wheel1_adc_fault_fp);
        TEST_ASSERT_EQUAL(fault_response_t::none, tf.step());
        tf.prop_failed_pressurize_fault_fp->signal(); 
        tf.adcs_wheel1_adc_fault_fp->signal();
        TEST_ASSERT_EQUAL(fault_response_t::safehold, tf.step());

        // If the wheel fault goes away, the response is standby.
        tf.adcs_wheel1_adc_fault_fp->unsignal();
        TEST_ASSERT_EQUAL(fault_response_t::standby, tf.step());
    }

    {
        TestFixtureMainFHEndToEnd tf;

        // Set up both faults at the same time.
        signal_until_persistence(tf.prop_failed_pressurize_fault_fp);
        signal_until_persistence(tf.adcs_wheel1_adc_fault_fp);
        TEST_ASSERT_EQUAL(fault_response_t::none, tf.step());
        tf.prop_failed_pressurize_fault_fp->signal(); 
        tf.adcs_wheel1_adc_fault_fp->signal();
        TEST_ASSERT_EQUAL(fault_response_t::safehold, tf.step());

        // If the pressurization fault goes away, the response is still safehold.
        tf.prop_failed_pressurize_fault_fp->unsignal();
        TEST_ASSERT_EQUAL(fault_response_t::safehold, tf.step());
    }

    /**
     * These next two tests check the behavior when there is an existing singular
     * fault and then a second fault occurs. The fault response after the second
     * fault arrives should be the more severe of the two faults.
     **/
    {
        // Suppose a wheel fault is currently signaled (so that the safehold
        // response is caused.) If the pressurization fault happens some control
        // cycles later, it should not affect the fault response recommendation.
        // If the pressurization fault then disappears, the fault response recommendation
        // is still safehold.
        TestFixtureMainFHEndToEnd tf;
        signal_until_persistence(tf.adcs_wheel1_adc_fault_fp);
        TEST_ASSERT_EQUAL(fault_response_t::none, tf.step());
        tf.adcs_wheel1_adc_fault_fp->signal();
        TEST_ASSERT_EQUAL(fault_response_t::safehold, tf.step());

        tf.cc += 5;
        signal_until_persistence(tf.prop_failed_pressurize_fault_fp);
        TEST_ASSERT_EQUAL(fault_response_t::safehold, tf.step());
        tf.prop_failed_pressurize_fault_fp->signal();
        TEST_ASSERT_EQUAL(fault_response_t::safehold, tf.step());
        tf.prop_failed_pressurize_fault_fp->unsignal();
        TEST_ASSERT_EQUAL(fault_response_t::safehold, tf.step());
    }

    /**
     * Suppose the pressurization fault is currently signaled (so that the standby 
     * response is caused.) If a wheel fault happens some control cycles later, 
     * it should force the fault response recommendation to safehold. If the wheel
     * fault goes away, the fault response recommendation should become standby
     * again.
     **/
    {
        TestFixtureMainFHEndToEnd tf;
        signal_until_persistence(tf.prop_failed_pressurize_fault_fp);
        TEST_ASSERT_EQUAL(fault_response_t::none, tf.step());
        tf.prop_failed_pressurize_fault_fp->signal();
        TEST_ASSERT_EQUAL(fault_response_t::standby, tf.step());

        tf.cc += 5;
        signal_until_persistence(tf.adcs_wheel1_adc_fault_fp);
        TEST_ASSERT_EQUAL(fault_response_t::standby, tf.step());
        tf.adcs_wheel1_adc_fault_fp->signal();
        TEST_ASSERT_EQUAL(fault_response_t::safehold, tf.step());
        tf.adcs_wheel1_adc_fault_fp->unsignal();
        TEST_ASSERT_EQUAL(fault_response_t::standby, tf.step());
    }
}

// TODO after prop fault handling is implemented
void test_overpressure() {
    /** 
     * If a low-power fault is signaled, the overpressure handling 
     * should not be able to occur.
     **/
    {

    }

    /**
     * Otherewise, the overpressure handling response proceeds as
     * expected.
     */
    {

    }
}

void test_fault_handling_end_to_end() {
    RUN_TEST(test_no_faults);
    RUN_TEST(test_single_simple_safehold_faults);
    RUN_TEST(test_single_simple_standby_faults);
    RUN_TEST(test_single_quake_fault);
    RUN_TEST(test_single_piksi_fault);

    RUN_TEST(test_two_simple_faults_safehold_and_standby);

    /**
     * There's not much point in testing three or more faults at once 
     * in this end-to-end test since the fault response, at this
     * point, has been demonstrated to work exactly as designed
     * when multiple faults are overlapping.
     */

    /**
     * We also won't test Quake and Piksi Fault Handler's interaction
     * with the other faults since these state machines are very independent
     * from the rest of the fault system. Their fault responses
     * should interact in exactly the same way as the simple faults do.
     * 
     * However, we will test the interaction of these fault handlers
     * with real hardware and othersubsystems in ptest.
     */

    /**
     * It's still worth testing the overpressure response since there are
     * a lot of autonomous response components that get executed in a precise
     * sequence. However, the location of this test might change once the
     * overpressure tests are actually implemented.
     */
    RUN_TEST(test_overpressure);
}
