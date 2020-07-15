def testitem_general(suffix, name, folder, deps, tags):
    native.cc_test(
        name = name + "_" + suffix,
        srcs = native.glob([folder + "/**/*.cpp", folder + "/**/*.h", folder + "/**/*.hpp", folder + "/**/*.inl", "shared/**/*.*"]),
        copts = ["-Iexternal/unity/src/ -Isrc/ -Ilib/common/psim/include -Ilib/common/libsbp/include -Ilib/common/concurrentqueue -Ilib/fsw/json"],
        deps = [":utils"] + deps,
        tags = tags + [suffix],
    )

def testitem_desktop(name, folder, deps, tags):
    testitem_general("desktop", name, folder, deps, tags)

def testitem_teensy(name, folder, deps, tags):
    testitem_general("teensy", name, folder, deps, tags)

def testitem(name, folder, desktop_deps, teensy_deps, tags):
    testitem_desktop(name, folder, desktop_deps, tags)
    testitem_teensy(name, folder, teensy_deps, tags)
