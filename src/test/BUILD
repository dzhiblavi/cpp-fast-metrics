cc_test(
    name = "fmetrics_test",
    srcs = [
        "estimator_test.cpp",
    ],
    deps = [
        "//src/fast_metrics",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "integration_test",
    srcs = [
        "integration.cpp",
    ],
    deps = [
        "//src/fast_metrics:fast_metrics",
        "//src/fast_metrics/http:http",
    ],
)
