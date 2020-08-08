load("//bazel:pan.bzl", "pan_cc_library")
load("@bazel_skylib//lib:selects.bzl", "selects")

pan_cc_library(
    name = "fswbase",
    srcs = glob(["**/*.cpp", "**/*.inl"]),
    hdrs = glob(["**/*.hpp", "**/*.h"]),
    copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/i2c_t3 -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"] + 
    selects.with_or({
        ("//bazel/configs:hitl32",
         "//bazel/configs:hitl35",
         "//bazel/configs:hitl36",
         "//bazel/configs:flight") : ["-Ilib/teensyduino/ -Ilib/teensyduino/avr -std=gnu++14 -mthumb -mcpu=cortex-m4 -felide-constructors -fno-exceptions -fno-rtti"],
         "//conditions:default" : []
    }),
    deps = [
        "//lib/common:psim",
        "//lib/common:i2c_t3",
        "//src/adcs:fsw_exports",
        "//src:flow_data",
        "//src/common:common",
    ]
)

pan_cc_library(
    name = "gswbase",
    srcs = glob(["**/*.cpp", "**/*.inl"]),
    hdrs = glob(["**/*.hpp", "**/*.h"]),
    copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"],
    deps = [
        "//lib/common:psim",
        "//lib/common:i2c_t3",
        "//src/adcs:fsw_exports",
        "//src:flow_data",
        "//src/common:common",
    ],
    defines = ["GSW"]
)
