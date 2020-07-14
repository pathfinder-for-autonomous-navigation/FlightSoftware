cc_library(
    name = "FCCode",
    srcs = glob(["**/*.cpp", "**/*.inl"]),
    hdrs = glob(["**/*.hpp", "**/*.h"]),
    linkstatic = True,
    visibility = ["//visibility:public"],
    copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"],
    deps = [
        "//src/common:common",
        "//lib/common:psim",
        "//src/adcs:fsw_exports",
        "//lib/fsw:json",
        "//lib/fsw:memuse",
        "//src:flow_data"
    ],
    local_defines = ["DESKTOP", "PAN_LEADER"]
)
