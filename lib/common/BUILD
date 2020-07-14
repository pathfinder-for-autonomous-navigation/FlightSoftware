cc_library(
    name = "libsbp",
    srcs = glob(["libsbp/src/*.c"]),
    hdrs = glob(["libsbp/include/libsbp/*.h"]),
    copts = ["-Ilib/common/libsbp/include/libsbp/ -Ilib/common/libsbp/include/"],
    linkstatic = True,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "concurrentqueue",
    hdrs = ["concurrentqueue/concurrentqueue.h"],
    linkstatic = True,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "ArduinoJson",
    hdrs = ["ArduinoJson/src/ArduinoJson.hpp"],
    srcs = glob(["ArduinoJson/src/**/*.*"], exclude=["ArduinoJson/src/ArduinoJson.hpp"]),
    linkstatic = True,
    visibility = ["//visibility:public"],
)