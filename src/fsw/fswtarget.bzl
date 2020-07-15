def fswtarget(name,target):
    native.cc_binary(
        name = name,
        srcs = [target],
        copts = ["-Isrc/ -Isrc/fsw/ -Isrc/fsw/Devices -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/common/psim/include -Ilib/fsw/json  -Ilib/fsw/memuse"],
        deps = ["//src/fsw/FCCode:fswbase_" + name],
        visibility = ["//visibility:public"]
    )

def fswtarget_teensy(name):
    fswtarget(name, "targets/teensy.cpp")

def fswtarget_desktop(name):
    fswtarget(name, "targets/native.cpp")
