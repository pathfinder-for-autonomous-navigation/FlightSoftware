def teensy_base(name, processor):
    native.cc_library(
        name = name,
        hdrs = ["Arduino.h","core_pins.h", "usb_serial.h"],
        srcs = native.glob(["**/*.c", "**/*.cpp", "**/*.h", "**/*.S"],
            exclude=["Arduino.h","core_pins.h", "usb_serial.h"] +
                    ["libraries/**/examples/**/*.*", "libraries/**/tests/**/*.*"]
                ),
        data = [processor.lower() + ".ld"],
        copts = ["-Ilib/teensyduino/ -std=gnu++14 -mthumb -mcpu=cortex-m4 -felide-constructors -fno-exceptions -fno-rtti"],
        linkstatic = True,
        linkopts = ["-Os -Wl,--gc-sections,--defsym=__rtc_localtime=0 --specs=nano.specs -mcpu=cortex-m4 -mthumb -T@{}.ld -lm".format(processor.lower())],
        defines = ["ARDUINO=10805", "TEENSYDUINO=144", "F_CPU=120000000","USB_SERIAL", "__@{}__".capitalize(), "LAYOUT_US_ENGLISH"],
        visibility = ["//visibility:public"],
    )
