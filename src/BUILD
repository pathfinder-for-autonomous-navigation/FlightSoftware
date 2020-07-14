filegroup(
    name = "flow_data_generator",
    srcs = ["flow_data_generator.py"]
)

genrule(
    name = "flow_data_genrule",
    srcs = ["flow_data.csv"],
    tools = [":flow_data_generator"],
    outs = ["flow_data.cpp"],
    cmd = "python3 $(location :flow_data_generator) $< $@"
)

cc_library(
    name = "flow_data",
    srcs = [":flow_data_genrule"],
    hdrs = ["flow_data.hpp"],
    copts = ["-Isrc/"],
    linkstatic = True,
    visibility = ["//visibility:public"]
)
