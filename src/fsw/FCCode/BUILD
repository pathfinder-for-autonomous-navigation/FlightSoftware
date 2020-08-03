load("//bazel:pan.bzl", "pan_cc_library")

pan_cc_library(
    name = "fswbase",
    srcs = glob(["**/*.cpp", "**/*.inl"]),
    hdrs = glob(["**/*.hpp", "**/*.h"]),
    copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"],
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
