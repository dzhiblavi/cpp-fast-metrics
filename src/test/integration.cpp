#include <gtest/gtest.h>
#include <thread>
#include <random>

#include <fmetrics/build.hpp>
#include <fmetrics/CodeBlock.hpp>
#include <fmetrics/Counter.hpp>
#include <fmetrics/Gauge.hpp>
#include <fmetrics/Histogram.hpp>
#include <fmetrics/estimator/Value.hpp>
#include <fmetrics/estimator/ExponentialRunningAverage.hpp>
#include <fmetrics/estimator/WindowRunningAverage.hpp>
#include <fmetrics/http/PullService.hpp>

using HistType = fmetrics::Histogram<1, 10, 100, 1000, 2000, 5000>;
using CodeBlockType = fmetrics::CodeBlock<int64_t, HistType>;

BUILD_METRICS(
  Metrics,
  code_block, CodeBlockType,
  gauge, fmetrics::Gauge<fmetrics::estimator::Value<int>>,
  histogram, HistType,
  counter, fmetrics::Counter<int>,
);


int get_random(int from, int to) {
  static std::mt19937 gen;
  return std::uniform_int_distribution<int>(from, to)(gen);
}

void sleep_random_time(int from_ms, int to_ms) {
  static std::mt19937 gen;
  std::this_thread::sleep_for(std::chrono::milliseconds(get_random(from_ms, to_ms)));
}

void worker_thread() {
  auto& m = Metrics::Instance();
  m.AddLabels(
    fmetrics::Label("server", "dzhiblavi"),
    fmetrics::Label("test", "integrational"));

  for (int i = 0; ; ++i) {
    m.gauge.Set(i);
    m.counter.Set(i * 2);
    m.histogram.Add(get_random(0, 6000));
    m.code_block.Execute<std::chrono::milliseconds>(sleep_random_time, 0, 2000);
  }
}

int main() {
  fmetrics::http::PullService service(Metrics::Instance());
  service.Start(8084);

  std::thread worker(worker_thread);

  worker.join();
  service.Shutdown();
  service.Wait();
  return 0;
}
