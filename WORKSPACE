load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
    ],
    sha256 = "97e70364e9249702246c0e9444bccdc4b847bed1eb03c5a3ece4f83dfe6abc44",
)
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
bazel_skylib_workspace()

lin_build = """
cc_library(
    name = "lin",
    srcs = glob(["src/**/*.cpp", "include/**/*.inl"]),
    hdrs = glob(["include/**/*.hpp"]),
    includes = ["include"],
    linkstatic = True,
    visibility = ["//visibility:public"]
)
"""

new_git_repository(
    name = "lin",
    init_submodules = True,
    remote = "http://github.com/pathfinder-for-autonomous-navigation/lin",
    commit = "f8fafaa5a29190663bc313a0b835201749b876b8",
    build_file_content = lin_build
)

unity_build = """
cc_library(
    name = "core",
    srcs = ["src/unity.c", "src/unity_internals.h"],
    hdrs = ["src/unity.h"],
    linkstatic = True,
    visibility = ["//visibility:public"],
    defines = ["UNITY_INCLUDE_DOUBLE"]
)
"""

new_git_repository(
    name = "unity",
    init_submodules = True,
    remote = "http://github.com/ThrowTheSwitch/Unity.git",
    commit = "0126e4804cf5edf87412044f841d7d7023a7c0c3",
    build_file_content = unity_build
)

git_repository(
    name = "platforms",
    commit = "681f1ee032566aa2d443cf0335d012925d9c58d4",
    remote = "http://github.com/bazelbuild/platforms.git",
    shallow_since = "1591199593 -0700"
)
