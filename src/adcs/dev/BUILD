cc_library(
    name = "devices",
    srcs = glob(["*.cpp", "*.inl"]),
    hdrs = glob(["*.hpp"]),
    copts = ["-Ilib/teensyduino -Ilib/teensyduino/libraries/Servo -Ilib/teensyduino/libraries/SPI -Ilib/common/i2c_t3"],
    deps = [
        "//lib/teensyduino:teensy35",
        "//lib/common:i2c_t3",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"]
)
