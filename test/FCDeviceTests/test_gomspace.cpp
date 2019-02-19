#include <i2c_t3.h>
#include <Gomspace/Gomspace.hpp>
#include <unity.h>
#include <unity_test/unity_fixture.h>

using namespace Devices;
Gomspace::eps_hk_t gomspace_data;
Gomspace::eps_config_t gomspace_config;
Gomspace::eps_config2_t gomspace_config2;
Gomspace gs(&gomspace_data, &gomspace_config, &gomspace_config2, Wire, Gomspace::ADDRESS);
bool gs_setup;

TEST_GROUP(GomspaceTests);

TEST_SETUP(GomspaceTests) {
    if (!gs_setup) {
        Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
        gs.setup();
        gs_setup = true;
    }
    if(!gs.is_functional()) TEST_IGNORE_MESSAGE("Gomspace is not connected/not functional");
}

TEST_TEAR_DOWN(GomspaceTests) {}

TEST(GomspaceTests, test_ping) {
    TEST_ASSERT(gs.ping(0x01));
}

TEST(GomspaceTests, test_get_hk) {
    TEST_ASSERT(gs.get_hk());
    // TODO add value comparisons
}

TEST(GomspaceTests, test_set_single_output) {
    for(unsigned char channel = 0; channel < 6; channel++) {
        TEST_ASSERT(gs.set_single_output(channel, 1));
        gs.get_hk();
        // TODO add value comparisons
        TEST_ASSERT(gs.set_single_output(channel, 0));
        delay(10000); // Wait for channel output to die down to zero
        gs.get_hk();
        // TODO add value comparisons
    }
}

TEST(GomspaceTests, test_set_pv_auto) {
    for(unsigned char mode = 0; mode < 2; mode++) {
        TEST_ASSERT(gs.set_pv_auto(mode));
        gs.get_hk();
        // TODO add value comparisons
    }  
}

TEST(GomspaceTests, test_heater) {
    TEST_ASSERT(gs.turn_on_heater());
    TEST_ASSERT_EQUAL(1, gs.get_heater());
    TEST_ASSERT(gs.turn_off_heater());
    TEST_ASSERT_EQUAL(0, gs.get_heater());
}

TEST(GomspaceTests, test_reset_counters) {
    // TODO
}

TEST(GomspaceTests, test_reset_wdt) {
    // TODO
}

TEST(GomspaceTests, test_config_get) {
    // TODO
}

TEST(GomspaceTests, test_config_set) {
    // TODO
}

TEST(GomspaceTests, test_hard_reset) {
    // TODO
}

TEST(GomspaceTests, test_config2_get) {
    // TODO
}

TEST(GomspaceTests, test_config2_set) {
    // TODO
}

TEST(GomspaceTests, test_reboot) {
    // TODO
}

TEST_GROUP_RUNNER(GomspaceTests) {
    RUN_TEST_CASE(GomspaceTests, test_ping);
    RUN_TEST_CASE(GomspaceTests, test_get_hk);
    RUN_TEST_CASE(GomspaceTests, test_set_single_output);
    RUN_TEST_CASE(GomspaceTests, test_set_pv_auto);
    RUN_TEST_CASE(GomspaceTests, test_heater);
    RUN_TEST_CASE(GomspaceTests, test_reset_counters);
    RUN_TEST_CASE(GomspaceTests, test_reset_wdt);
    RUN_TEST_CASE(GomspaceTests, test_config_get);
    RUN_TEST_CASE(GomspaceTests, test_config_set);
    RUN_TEST_CASE(GomspaceTests, test_hard_reset);
    RUN_TEST_CASE(GomspaceTests, test_config2_get);
    RUN_TEST_CASE(GomspaceTests, test_config2_set);
    RUN_TEST_CASE(GomspaceTests, test_reboot);
}