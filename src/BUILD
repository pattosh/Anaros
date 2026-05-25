load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "anaros",
    hdrs = ["anaros.h"],
    visibility = ["//visibility:public"],
    strip_include_prefix = "/src",
    deps = ["@yaml-cpp//:yaml-cpp"],
)