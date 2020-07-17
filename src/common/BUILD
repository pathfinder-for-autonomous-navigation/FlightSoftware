load(":common.bzl", "common_library_desktop")
load(":common.bzl", "common_library_teensy")

common_library_desktop(name = "common_desktop_fsw_ut", defines = ["DESKTOP", "UNIT_TEST"])

common_library_desktop(name = "common_desktop_fsw", defines = ["DESKTOP"])
common_library_desktop(name = "common_gsw_ut", defines = ["GSW", "DESKTOP", "UNIT_TEST"])

common_library_desktop(name = "common_gsw", defines = ["GSW", "DESKTOP"])
common_library_teensy(name = "common_teensy", defines = [])
common_library_teensy(name = "common_teensy_ut", defines = ["UNIT_TEST"])


cc_library(
    name = "common",
    srcs = glob(["*.cpp", "*.inl"]),
    hdrs = glob(["*.hpp", "*.h"]),
    copts = [
        "-Isrc/common -Isrc/ -Ilib/common/libsbp/include/ -Ilib/common/concurrentqueue/  -Ilib/common/ArduinoJson/src/",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        "//lib/common:libsbp",
        "//lib/common:concurrentqueue",
        "//lib/common:ArduinoJson",
        "@lin//:lin",
    ],
    defines = ["LIN_RANDOM_SEED=6828031"] +
        select({
            "//:gsw" : ["GSW, DESKTOP"],
            "//:hootl" : ["DESKTOP"],
            "//:hitl" : [],
            "//:flight" : [],
        }) +
        select({
            "//:unit_test" : ["UNIT_TEST"],
            "//conditions:default" : []
        })
)
