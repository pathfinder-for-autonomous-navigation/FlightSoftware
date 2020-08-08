cc_library(
    name = "fsw_exports",
    hdrs = ["constants.hpp", "havt_devices.hpp", "state_registers.hpp"],
    linkstatic = True,
    visibility = ["//visibility:public"]
)

cc_binary(
    name = "flight",
    srcs = glob(["utl/**/*.*", "*.cpp", "*.hpp"]),
    copts = ["-Ilib/common/i2c_t3", "-Ilib/teensyduino -Isrc/"],
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        "//src/common:common",
        "//src/adcs/dev:devices",
    ]
)
