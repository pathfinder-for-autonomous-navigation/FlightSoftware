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

git_repository(
    name = "lin",
    init_submodules = True,
    remote = "http://github.com/pathfinder-for-autonomous-navigation/lin",
    commit = "1adce4f99d326c6fc57f516caec4ae7f9589dc59",
    shallow_since = "1595120906 -0700"
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

http_archive(
    name = "arm_none_eabi",
    url = "https://github.com/d-asnaghi/bazel-arm-none-eabi/archive/v1.1.tar.gz",
    sha256 = "3d3728cbe88b08c12cd2cb89afcff9294bd77be958c78188db77fdc8ab7e7a5d",
    strip_prefix = "bazel-arm-none-eabi-1.1"
)

load("@arm_none_eabi//:deps.bzl", "arm_none_eabi_deps")
arm_none_eabi_deps()
