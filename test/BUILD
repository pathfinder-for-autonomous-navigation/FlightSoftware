load(":test.bzl", "testitem")
load(":test.bzl", "testitem_desktop")

cc_library(
    name = "utils",
    hdrs = ["custom_assertions.hpp", "StateFieldRegistryMock.hpp"],
    deps = [
        "@unity//:core"
    ],
    linkstatic = True,
    local_defines = ["BAZEL"]
)

test_suite(
    name = "desktop",
    tests = [
        ":common_desktop",
        ":fsw_desktop",
        ":gsw",
    ]
)

test_suite(
    name = "teensy",
    tests = [
        ":common_teensy",
        ":fsw_teensy",
    ]
)

common_desktop_deps = ["//src/common:common_desktop_ut"]
common_teensy_deps = ["//src/common:common_teensy_ut"]
test_suite(name = "common_desktop", tags = ["common", "desktop"])
test_suite(name = "common_teensy", tags = ["common", "teensy"])
testitem(name = "bit_array", folder = "test_common_bit_array", desktop_deps = common_desktop_deps, teensy_deps = common_teensy_deps, tags = ["common"])
testitem(name = "gpstime", folder = "test_common_gpstime", desktop_deps = common_desktop_deps, teensy_deps = common_teensy_deps, tags = ["common"])
testitem(name = "serializers", folder = "test_common_serializers", desktop_deps = common_desktop_deps, teensy_deps = common_teensy_deps, tags = ["common"])
testitem(name = "state_field", folder = "test_common_state_field", desktop_deps = common_desktop_deps, teensy_deps = common_teensy_deps, tags = ["common"])
testitem(name = "state_field_registry", folder = "test_common_state_field_registry", desktop_deps = common_desktop_deps, teensy_deps = common_teensy_deps, tags = ["common"])

fsw_desktop_deps = ["//src/fsw/FCCode:fswbase_desktop_ut"]
fsw_teensy_deps = ["//src/fsw/FCCode:fswbase_teensy_ut"]
test_suite(name = "fsw_desktop", tags = ["fsw", "desktop"])
test_suite(name = "fsw_teensy", tags = ["fsw", "teensy"])
testitem(name = "adcs_box_monitor", folder = "test_fsw_adcs_box_monitor", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "adcs_commander", folder = "test_fsw_adcs_commander", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "attitude_computer", folder = "test_fsw_attitude_computer", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "attitude_estimator", folder = "test_fsw_attitude_estimator", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "bitstream", folder = "test_fsw_bitstream", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "control_task", folder = "test_fsw_control_task", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "dcdc", folder = "test_fsw_dcdc", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "docking", folder = "test_fsw_docking", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "downlink_producer", folder = "test_fsw_downlink_producer", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "EEPROM", folder = "test_fsw_EEPROM", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "event", folder = "test_fsw_event", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "fault", folder = "test_fsw_fault", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "fault_handler", folder = "test_fsw_fault_handler", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "gomspace", folder = "test_fsw_gomspace", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "mission_manager", folder = "test_fsw_mission_manager", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "orbit_controller", folder = "test_fsw_orbit_controller", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "piksi_control_task", folder = "test_fsw_piksi_control_task", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "prop_controller", folder = "test_fsw_prop_controller", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "prop_faults", folder = "test_fsw_prop_faults", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "quake_manager", folder = "test_fsw_quake_manager", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "state_field_registry_mock", folder = "test_fsw_state_field_registry_mock", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "timed_control_task", folder = "test_fsw_timed_control_task", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])
testitem(name = "uplink_consumer", folder = "test_fsw_uplink_consumer", desktop_deps = fsw_desktop_deps, teensy_deps = fsw_teensy_deps, tags = ["fsw"])

gsw_deps = ["//src/fsw/FCCode:fswbase_gsw_ut", "//src/gsw:ut_lib", "//src:flow_data"]
test_suite(name = "gsw", tags = ["gsw", "desktop"])
testitem_desktop(name = "downlink_parser", folder = "test_gsw_downlink_parser", deps = gsw_deps, tags = ["gsw"])
testitem_desktop(name = "uplink_producer", folder = "test_gsw_uplink_producer", deps = gsw_deps, tags = ["gsw"])
