gsw_copts = ["-Isrc -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json"]
gsw_deps = ["//src/fsw/FCCode:fswbase_gsw"]
gsw_defines = ["DESKTOP", "PAN_LEADER", "GSW"]

cc_binary(
    name = "downlink_parser",
    srcs = glob(["parsers/src/DownlinkParser.*"]) + ["parsers/targets/downlink_parser.cpp"],
    copts = gsw_copts,
    deps = gsw_deps,
    local_defines = gsw_defines
)

cc_binary(
    name = "telem_info_generator",
    srcs = glob(["parsers/src/TelemetryInfoGenerator.*"]) + ["parsers/targets/telem_info_generator.cpp"],
    copts = gsw_copts,
    deps = gsw_deps,
    local_defines = gsw_defines
)

cc_binary(
    name = "uplink_producer",
    srcs = glob(["parsers/src/UplinkProducer.*"]) + ["parsers/targets/uplink_producer.cpp"],
    copts = gsw_copts,
    deps = gsw_deps,
    local_defines = gsw_defines
)

cc_library(
    name = "ut_lib",
    srcs = glob(["parsers/src/*.cpp"]),
    hdrs = glob(["parsers/src/*.hpp", "parsers/src/*.h"]),
    copts = gsw_copts,
    deps = gsw_deps,
    local_defines = gsw_defines,
    visibility = ["//visibility:public"]
)
