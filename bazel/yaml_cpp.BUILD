load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "yaml-cpp",
    hdrs = glob(["include/yaml-cpp/**/*.h"]),
    srcs = glob(["lib/libyaml-cpp.so*"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
