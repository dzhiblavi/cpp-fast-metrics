#include <gtest/gtest.h>
#include <thread>

#include <fmetrics/build.hpp>
#include <fmetrics/CodeBlock.hpp>
#include <fmetrics/Counter.hpp>
#include <fmetrics/Gauge.hpp>
#include <fmetrics/Histogram.hpp>
#include <fmetrics/estimator/Value.hpp>
#include <fmetrics/estimator/ExponentialRunningAverage.hpp>
#include <fmetrics/estimator/WindowRunningAverage.hpp>

TEST(estimator, value) {
  fmetrics::estimator::Value<int> value;

  EXPECT_EQ(0, value.Get());
  value.Set(5);
  EXPECT_EQ(5, value.Get());
  value.Add(-1);
  EXPECT_EQ(4, value.Get());
}

TEST(estimator, exponential) {
  fmetrics::estimator::ExponentialRunningAverage<int, 15> ema;
  EXPECT_EQ(0, ema.Get());
  ema.Update(100);
  ASSERT_GT(ema.Get(), 0);
}

TEST(estimator, window) {
  fmetrics::estimator::WindowAverage<int, std::milli, 15> wa;
  EXPECT_EQ(0, wa.Get());
  wa.Update(100);
  ASSERT_GT(wa.Get(), 0);
}

TEST(metric, dump) {
  using ValueEstimator = fmetrics::estimator::Value<int>;
  fmetrics::Gauge<ValueEstimator> gauge(
      "name",
      std::make_pair(std::string("label1"), std::string("value1")));
  gauge.Set(239);
}

using HistType = fmetrics::Histogram<1, 10, 100, 1000, 2000, 5000>;
using CodeBlockType = fmetrics::CodeBlock<int64_t, HistType>;

BUILD_METRICS(
  Metrics,
  code_block, CodeBlockType,
  counter, fmetrics::Counter<int>,
  gauge, fmetrics::Gauge<fmetrics::estimator::Value<int>>,
  histogram, HistType,
);

TEST(metrics, instance) {
  auto& metrics = Metrics::Instance();
  metrics.code_block.Execute<std::chrono::milliseconds>(
    [] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
  );
  metrics.Dump(std::cout);
}
