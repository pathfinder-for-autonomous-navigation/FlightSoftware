def common_library(name, defines, linkopts):
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
        linkopts = linkopts,
        defines = ["LIN_RANDOM_SEED=6828031"] + defines
    )

def common_library_desktop(name, defines):
    common_library(name, defines, ["-pthread"])
    
def common_library_teensy(name, defines):
    common_library(name, defines, [])
