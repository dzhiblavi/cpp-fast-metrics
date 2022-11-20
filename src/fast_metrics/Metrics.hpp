#ifndef LIB_FAST_METRICS_METRICS_HPP
#define LIB_FAST_METRICS_METRICS_HPP

#include <iostream>
#include <vector>

#include "src/fast_metrics/Metric.hpp"

namespace fmetrics {

class Metrics {
 public:
  void Dump(std::ostream& os) const;

  template <typename... Labels>
  void AddLabels(const Labels&... labels) {
    std::for_each(
        metrics_.begin(), metrics_.end(),
        [&labels...] (const auto& m) { m->AddLabels(labels...); });
  }

 protected:
  std::vector<Metric*> metrics_;
};

}  // namespace fmetrics

#endif  // LIB_FAST_METRICS_METRICS_HPP
