def pan_cc_library(name, srcs, hdrs,
                  deps = [],
                  copts = [],
                  linkstatic = True,
                  linkopts = [],
                  visibility = ["//visibility:public"],
                  defines = []):
    native.cc_library(
        name = name,
        srcs = srcs,
        hdrs = hdrs,
        linkstatic = linkstatic,
        visibility = visibility,
        copts = copts,
        deps = deps +
            select({
                "//bazel/configs:gsw" : ["//lib/fsw:memuse", "//lib/fsw:json"],
                "//bazel/configs:hootl" : ["//lib/fsw:memuse", "//lib/fsw:json"],
                "//conditions:default" : [],
            }),
        linkopts = linkopts + 
            select({
                "//bazel/configs:hootl" : ["-pthread"],
                "//bazel/configs:gsw" : ["-pthread"],
                "//conditions:default" : []
            }),
        defines = defines + ["LIN_RANDOM_SEED=6828031"] + 
            select({
                "//bazel/configs:gsw" : ["GSW", "DESKTOP", "FLIGHT"],
                "//bazel/configs:hootl" : ["DESKTOP"],
                "//bazel/configs:hitl32" : [],
                "//bazel/configs:hitl35" : [],
                "//bazel/configs:hitl36" : [],
                "//bazel/configs:flight" : ["FLIGHT"]
            }) + 
            select({
                "//bazel/configs:leader" : ["PAN_LEADER"],
                "//bazel/configs:follower" : ["PAN_FOLLOWER"],
            }) +
            select({
                "//bazel/configs:realtime" : [],
                "//bazel/configs:accelerated" : ["SPEEDUP"],
            }) +
            select({
                "//bazel/configs:unit_test" : ["UNIT_TEST"],
                "//conditions:default" : [],
            })
    )

def pan_cc_test(name, srcs, data = [], copts = [], deps = [], tags = [], defines = []):
    native.cc_test(
        name = name,
        srcs = srcs + native.glob(["shared/**/*.*"]),
        data = data,
        copts = ["-Iexternal/unity/src/ -Isrc/ -Ilib/common/psim/include -Ilib/common/libsbp/include -Ilib/common/concurrentqueue"] + copts + select({
            "//bazel/configs:gsw" : ['-Ilib/fsw/json'],
            "//bazel/configs:hootl" : ['-Ilib/fsw/json'],
            "//conditions:default" : []
        }),
        deps = ["//test:utils"] + deps,
        tags = tags,
        defines = defines +
            select({
                "//bazel/configs:unit_test" : ["UNIT_TEST"],
            })
    )
