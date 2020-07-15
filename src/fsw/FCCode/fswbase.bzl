def fswbase(name, defines, deps):
    native.cc_library(
        name = name,
        srcs = native.glob(["**/*.cpp", "**/*.inl"]),
        hdrs = native.glob(["**/*.hpp", "**/*.h"]),
        linkstatic = True,
        visibility = ["//visibility:public"],
        copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"],
        deps = deps + [
            "//lib/common:psim",
            "//lib/common:i2c_t3",
            "//src/adcs:fsw_exports",
            "//src:flow_data"
        ],
        defines = defines,
    )

def gswbase(name, defines):
    fswbase(name, defines, ["//src/common:common_gsw", "//lib/fsw:json", "//lib/fsw:memuse"])

def gswbase_ut(name, defines):
    fswbase(name, defines, ["//src/common:common_gsw_ut", "//lib/fsw:json", "//lib/fsw:memuse"])

def fswbase_desktop(name, defines):
    fswbase(name, defines, ["//src/common:common_desktop_fsw", "//lib/fsw:json", "//lib/fsw:memuse"])

def fswbase_desktop_ut(name, defines):
    fswbase(name, defines, ["//src/common:common_desktop_fsw_ut", "//lib/fsw:json", "//lib/fsw:memuse"])

def fswbase_teensy(name, defines):
    fswbase(name, defines, ["//src/common:common_teensy"])

def fswbase_teensy_ut(name, defines):
    fswbase(name, defines, ["//src/common:common_teensy"])
