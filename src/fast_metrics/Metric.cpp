#include "src/fast_metrics/Metric.hpp"

namespace fmetrics {

Metric::Metric(const std::string& name, const LabelsType& labels)
  : Metric(name, std::make_shared<LabelsType>(labels)) {}

Metric::Metric(const std::string& name, const std::shared_ptr<LabelsType>& labels) noexcept
  : name_(name), labels_(labels) {}

void Metric::Dump(std::ostream& os) const { Dump(os, {}); }

}  // namespace fmetrics
