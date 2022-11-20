#ifndef LIB_FAST_METRICS_COUNTER_HPP
#define LIB_FAST_METRICS_COUNTER_HPP

#include "src/fast_metrics/Metric.hpp"
#include "src/fast_metrics/estimator/Value.hpp"

namespace fmetrics {

template <std::integral T>
class Counter : public estimator::Value<T>, public Metric {
 public:
  using Metric::Metric;

  void Dump(std::ostream& os, const LabelsType& labels) const override {
    os << "# TYPE " << name_ << " counter" << '\n';
    DumpValue(os, name_, this->Get(), labels);
  }
};

}  // namepsace fmetrics

#endif  // LIB_FAST_METRICS_GAUGE_HPP
