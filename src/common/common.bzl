def common_library(name, defines):
    native.cc_library(
        name = name,
        srcs = native.glob(["*.cpp", "*.inl"]),
        hdrs = native.glob(["*.hpp", "*.h"]),
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
        defines = ["LIN_RANDOM_SEED=6828031"] + defines
    )
