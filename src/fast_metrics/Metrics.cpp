#include "src/fast_metrics/Metrics.hpp"

namespace fmetrics {

void Metrics::Dump(std::ostream& os) const {
  std::for_each(metrics_.begin(), metrics_.end(), [&os] (const auto& m) { m->Dump(os); });
}

}  // namespace fmetrics

