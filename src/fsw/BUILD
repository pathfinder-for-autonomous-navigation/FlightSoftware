load("@bazel_skylib//lib:selects.bzl", "selects")

cc_binary(
    name = "fsw",
    srcs = selects.with_or({
        "//bazel/configs:hootl" : ["targets/native.cpp"],
        ("//bazel/configs:hitl32", "//bazel/configs:hitl35", "//bazel/configs:hitl36", "//bazel/configs:flight") : ["targets/teensy.cpp"],
    }),
    copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"],
    deps = ["//src/fsw/FCCode:fswbase"],
    visibility = ["//visibility:public"]
)
