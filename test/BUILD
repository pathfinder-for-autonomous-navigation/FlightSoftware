load("//bazel:pan.bzl", "pan_cc_test")

cc_library(
    name = "utils",
    hdrs = ["custom_assertions.hpp", "StateFieldRegistryMock.hpp"],
    deps = [
        "@unity//:core",
    ],
    linkstatic = True,
    local_defines = ["BAZEL"],
)

test_suite(
    name = "desktop",
    tests = [":common", ":fsw", ":gsw"],
)

test_suite(
    name = "teensy",
    tests = [":common", ":fsw"],
)

test_suite(
    name = "fsw",
    tags = ["fsw"],
)

test_suite(
    name = "common",
    tags = ["common"]
)

common_tags = ["common"]
common_deps = ["//src/common:common"]
pan_cc_test(name = "bit_array", srcs = ["test_common_bit_array/test_bit_array.cpp"], tags = common_tags, deps = common_deps)
pan_cc_test(name = "gpstime", srcs = ["test_common_gpstime/test_gps_time.cpp"], tags = common_tags, deps = common_deps)
pan_cc_test(name = "serializers", srcs = ["test_common_serializers/test_serializers.cpp"], tags = common_tags, deps = common_deps)
pan_cc_test(name = "state_field", srcs = ["test_common_state_field/test_state_field.cpp"], tags = common_tags, deps = common_deps)
pan_cc_test(name = "state_field_registry", srcs = ["test_common_state_field_registry/test_state_field_registry.cpp"], tags = common_tags, deps = common_deps)

fsw_tags = ["fsw"]
fsw_deps = ["//src/fsw/FCCode:fswbase"]
pan_cc_test(name = "adcs_box_monitor", srcs = ["test_fsw_adcs_box_monitor/test_adcs_box_monitor.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "adcs_commander", srcs = ["test_fsw_adcs_commander/test_adcs_commander.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "attitude_computer", srcs = ["test_fsw_attitude_computer/test_fsw_attitude_computer.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "attitude_estimator", srcs = ["test_fsw_attitude_estimator/test_attitude_estimator.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "bitstream", srcs = ["test_fsw_bitstream/test_bitstream.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "control_task", srcs = ["test_fsw_control_task/test_control_task.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "dcdc", srcs = ["test_fsw_dcdc/test_fsw_dcdc.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "docking", srcs = ["test_fsw_docking/test_fsw_docking.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "downlink_producer", srcs = ["test_fsw_downlink_producer/test_downlink_producer.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "EEPROM", srcs = ["test_fsw_EEPROM/test_fsw_EEPROM.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "event", srcs = ["test_fsw_event/test_fsw_event.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "fault", srcs = ["test_fsw_fault/test_fsw_fault.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "fault_handler", srcs = glob(["test_fsw_fault_handler/*.cpp", "test_fsw_fault_handler/*.hpp"]), tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "gomspace", srcs = ["test_fsw_gomspace/test_fsw_gomspace.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "mission_manager", srcs = glob(["test_fsw_mission_manager/*.cpp", "test_fsw_mission_manager/*.hpp"]), tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "orbit_controller", srcs = ["test_fsw_orbit_controller/test_orbit_controller.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "piksi_control_task", srcs = ["test_fsw_piksi_control_task/test_piksi_control_task.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "prop_controller", srcs = ["test_fsw_prop_controller/test_prop_controller.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "prop_faults", srcs = ["test_fsw_prop_faults/test_prop_faults.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "quake_manager", srcs = ["test_fsw_quake_manager/test_quake_manager.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "state_field_registry_mock", srcs = ["test_fsw_state_field_registry_mock/test_fsw_state_field_registry_mock.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "timed_control_task", srcs = ["test_fsw_timed_control_task/test_timed_control_task.cpp"], tags = fsw_tags, deps = fsw_deps)
pan_cc_test(name = "uplink_consumer", srcs = ["test_fsw_uplink_consumer/test_fsw_uplink_consumer.cpp"], tags = fsw_tags, deps = fsw_deps)

#################################################
# Ground Software Tests
#################################################

gsw_deps = ["//src/gsw:ut_lib"]
gsw_copts = ["-Ilib/fsw/json"]
gsw_tags = ["gsw"]
test_suite(name = "gsw", tags = ["gsw"])

pan_cc_test(
    name = "downlink_parser",
    srcs = ["test_gsw_downlink_parser/test_gsw_downlink_parser.cpp", "test_gsw_downlink_parser/DownlinkParserMock.hpp"],
    copts = gsw_copts,
    deps = gsw_deps,
    tags = gsw_tags,
)

pan_cc_test(
    name = "uplink_producer",
    srcs = ["test_gsw_uplink_producer/test_gsw_uplink_producer.cpp"],
    data = glob([
        "test_gsw_uplink_producer/*.sbd",
        "test_gsw_uplink_producer/*.json",
    ]),
    copts = gsw_copts,
    deps = gsw_deps,
    tags = gsw_tags,
)
