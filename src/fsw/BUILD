load(":fswtarget.bzl", "fswtarget_desktop")
load(":fswtarget.bzl", "fswtarget_teensy")

load("@bazel_skylib//lib:selects.bzl", "selects")

cc_binary(
    name = "fsw",
    srcs = selects.with_or({
        "//:hootl" : ["targets/native.cpp"],
        ("//:hitl", "//:flight") : ["targets/teensy.cpp"],
    }),
    copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"],
    deps = ["//src/fsw/FCCode:fswbase"],
    visibility = ["//visibility:public"]
)

fswtarget_desktop(name = "hootl_leader")
fswtarget_desktop(name = "hootl_follower")
fswtarget_desktop(name = "realtime_hootl_leader")
fswtarget_desktop(name = "realtime_hootl_follower")

fswtarget_teensy(name = "hitl_leader")
fswtarget_teensy(name = "hitl_follower")
fswtarget_teensy(name = "realtime_hitl_leader")
fswtarget_teensy(name = "realtime_hitl_follower")

fswtarget_teensy(name = "flight_leader")
fswtarget_teensy(name = "flight_follower")
