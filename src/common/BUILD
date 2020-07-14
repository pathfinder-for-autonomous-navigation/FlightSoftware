cc_library(
    name = "common",
    srcs = glob(["*.cpp", "*.inl"], exclude=["targets/"]),
    hdrs = glob(["*.hpp", "*.h"]),
    copts = ["-Isrc/common -Isrc/ -Ilib/common/libsbp/include/ -Ilib/common/concurrentqueue/  -Ilib/common/ArduinoJson/src/"],
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        "//lib/common:libsbp",
        "//lib/common:concurrentqueue",
        "//lib/common:ArduinoJson",
        "@lin//:lin",
    ],
    local_defines = ["DESKTOP"]
)
