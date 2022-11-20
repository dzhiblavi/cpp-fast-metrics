cc_library(
    name = "http",
    srcs = [
        "PullService.cpp",
    ],
    hdrs = [
        "PullService.hpp",
    ],
    deps = [
        "//src/fast_metrics",
        "@boost//:beast",
        "@boost//:asio",
        "@boost//:bind",
    ],
    include_prefix = "fmetrics/http",
    visibility = ["//visibility:public"],
)
