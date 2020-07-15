cc_library(
    name = "utils",
    hdrs = ["custom_assertions.hpp", "StateFieldRegistryMock.hpp", "util.cpp"],
    deps = [
        "@unity//:core"
    ],
    linkstatic = True,
    local_defines = ["BAZEL"]
)

test_suite(
    name = "all",
    tests = [
        ":common"
    ]
)

test_suite(
    name = "common",
    tests = [
        ":common_bit_array"
    ]
)

cc_test(
    name = "common_bit_array",
    srcs = glob(["test_common_bit_array/**/*.*"]),
    copts = ["-Iexternal/unity/src/ -Isrc/"],
    deps = [
        ":utils",
        "//src/common:common"
    ],
    local_defines = ["DESKTOP"]
)