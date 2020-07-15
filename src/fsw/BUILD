alias(
    name = "base",
    actual = "//src/fsw/FCCode:fswbase",
    visibility = ["//visibility:public"]
)

cc_binary(
    name = "native",
    srcs = ["targets/native.cpp"],
    copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"],
    deps = [
        ":base",
    ],
    local_defines = ["DESKTOP", "PAN_LEADER"],
    visibility = ["//visibility:public"]
)
