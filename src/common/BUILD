load("//bazel:pan.bzl", "pan_cc_library")

pan_cc_library(
    name = "common",
    srcs = glob(["*.cpp", "*.inl"]),
    hdrs = glob(["*.hpp", "*.h"]),
    copts = [
        "-Isrc/common -Isrc/ -Ilib/teensyduino/ -Ilib/common/libsbp/include/ -Ilib/common/concurrentqueue/  -Ilib/common/ArduinoJson/src/",
    ],
    deps = [
        "//lib/common:libsbp",
        "//lib/common:concurrentqueue",
        "//lib/common:ArduinoJson",
        "@lin//:lin",
    ]
)
