load(":fswbase.bzl", "gswbase")
load(":fswbase.bzl", "gswbase_ut")
load(":fswbase.bzl", "fswbase_desktop")
load(":fswbase.bzl", "fswbase_desktop_ut")
load(":fswbase.bzl", "fswbase_teensy")
load(":fswbase.bzl", "fswbase_teensy_ut")

load("@bazel_skylib//lib:selects.bzl", "selects")

cc_library(
    name = "fswbase",
    srcs = glob(["**/*.cpp", "**/*.inl"]),
    hdrs = glob(["**/*.hpp", "**/*.h"]),
    linkstatic = True,
    visibility = ["//visibility:public"],
    copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"],
    deps = [
            "//lib/common:psim",
            "//lib/common:i2c_t3",
            "//src/adcs:fsw_exports",
            "//src:flow_data",
            "//src/common:common",
        ] +
        selects.with_or({
            ("//:gsw", "//:hootl") : ["//lib/fsw:memuse", "//lib/fsw:json"],
            "//conditions:default" : [],
        }),
    defines = 
        select({
            "//:gsw" : ["GSW", "DESKTOP", "FLIGHT"],
            "//:hootl" : ["DESKTOP"],
            "//:hitl" : [""],
            "//:flight" : ["FLIGHT"],
        }) + 
        select({
            "//:leader" : ["PAN_LEADER"],
            "//:follower" : ["PAN_FOLLOWER"],
        }) +
        select({
            "//:realtime" : [],
            "//:speedup" : ["SPEEDUP"],
        }) +
        select({
            "//:unit_test" : ["UNIT_TEST"],
            "//conditions:default" : [],
        })
)

cc_library(
    name = "gswbase_",
    srcs = glob(["**/*.cpp", "**/*.inl"]),
    hdrs = glob(["**/*.hpp", "**/*.h"]),
    linkstatic = True,
    visibility = ["//visibility:public"],
    copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"],
    deps = [
            "//lib/common:psim",
            "//lib/common:i2c_t3",
            "//src/adcs:fsw_exports",
            "//src:flow_data",
            "//src/common:common",
            "//lib/fsw:memuse",
            "//lib/fsw:json"
        ],
    defines = ["GSW", "DESKTOP", "FLIGHT", "PAN_LEADER"] +
        select({
            "//:unit_test" : ["UNIT_TEST"],
            "//conditions:default" : [],
        })
)

gswbase(name = "gswbase", defines = ["GSW", "DESKTOP", "FLIGHT", "PAN_LEADER"])
gswbase_ut(name = "gswbase_ut", defines = ["GSW", "DESKTOP", "FLIGHT", "PAN_LEADER", "UNIT_TEST"])

fswbase_desktop(name = "fswbase_hootl_leader", defines = ["DESKTOP", "SPEEDUP", "PAN_LEADER"])
fswbase_desktop(name = "fswbase_hootl_follower", defines = ["DESKTOP", "SPEEDUP", "PAN_FOLLOWER"])
fswbase_desktop(name = "fswbase_realtime_hootl_leader", defines = ["DESKTOP", "PAN_LEADER"])
fswbase_desktop(name = "fswbase_realtime_hootl_follower", defines = ["DESKTOP", "PAN_FOLLOWER"])
fswbase_desktop_ut(name = "fswbase_desktop_ut", defines = ["DESKTOP", "PAN_LEADER", "UNIT_TEST", "SPEEDUP"])

fswbase_teensy(name = "fswbase_hitl_leader", defines = ["SPEEDUP", "PAN_LEADER"])
fswbase_teensy(name = "fswbase_hitl_follower", defines = ["SPEEDUP", "PAN_FOLLOWER"])
fswbase_teensy(name = "fswbase_realtime_hitl_leader", defines = ["PAN_LEADER"])
fswbase_teensy(name = "fswbase_realtime_hitl_follower", defines = ["PAN_FOLLOWER"])
fswbase_teensy_ut(name = "fswbase_teensy_ut", defines = ["PAN_LEADER", "UNIT_TEST"])

fswbase_teensy(name = "fswbase_flight_leader", defines = ["FLIGHT", "PAN_LEADER"])
fswbase_teensy(name = "fswbase_flight_follower", defines = ["FLIGHT", "PAN_FOLLOWER"])
fswbase_teensy_ut(name = "fswbase_flight_ut", defines = ["FLIGHT", "PAN_LEADER", "UNIT_TEST"])
