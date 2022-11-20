#ifndef LIB_FAST_METRICS_GAUGE_HPP
#define LIB_FAST_METRICS_GAUGE_HPP

#include "src/fast_metrics/Metric.hpp"
#include "src/fast_metrics/estimator/Estimator.hpp"

namespace fmetrics {

template <typename Estimator>
class Gauge : public Estimator, public Metric {
 public:
  using Metric::Metric; 

  void Dump(std::ostream& os, const LabelsType& labels) const override {
    os << "# TYPE " << name_ << " gauge" << '\n';
    DumpValue(os, name_, Estimator::Get(), labels);
  }
};

}  // namepsace fmetrics

#endif  // LIB_FAST_METRICS_GAUGE_HPP
