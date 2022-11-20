#ifndef LIB_FAST_METRICS_HISTOGRAM_HPP
#define LIB_FAST_METRICS_HISTOGRAM_HPP

#include <numeric>

#include "src/fast_metrics/Metric.hpp"
#include "src/fast_metrics/estimator/Estimator.hpp"

namespace fmetrics {

namespace detail_ {

template <int64_t... Bounds>
struct HistRate {
 public:
  static constexpr int64_t NumBuckets = sizeof...(Bounds);

  HistRate() noexcept;

  void Add(int64_t value) noexcept;

  int64_t GetInf() const noexcept;

  std::array<int64_t, NumBuckets> GetBuckets() const noexcept;

  const std::array<int64_t, NumBuckets>& GetBounds() const noexcept;

 private:
  std::array<int64_t, NumBuckets> bounds_;
  std::array<std::atomic<int64_t>, 1 + NumBuckets> buckets_;
};

}  // namespace detail_

template <int64_t... Bounds>
class Histogram : public detail_::HistRate<Bounds...>, public Metric {
 public:
  using Metric::Metric;

  void Dump(std::ostream& os, const LabelsType& labels) const override {
    const auto& bounds = detail_::HistRate<Bounds...>::GetBounds();
    const auto buckets = detail_::HistRate<Bounds...>::GetBuckets();
    const auto count = std::accumulate(buckets.begin(), buckets.end(), 0);

    os << "# TYPE " << name_ << " histogram\n";
    DumpValue(os, name_ + "_count", count, labels);

    for (size_t index = 0; index < buckets.size(); ++index) {
      DumpValue(
          os, name_ + "_bucket", buckets[index], labels,
          Label("le", std::to_string(bounds[index])));
    }
    DumpValue(os, name_ + "_bucket", count, labels, Label("le", "+Inf"));
  }
};

namespace detail_ {

template <int64_t... Bounds>
HistRate<Bounds...>::HistRate() noexcept : bounds_{Bounds...} {}

template <int64_t... Bounds>
void HistRate<Bounds...>::Add(int64_t value) noexcept {
  int bucket = 0;
  while (bucket < NumBuckets && value > bounds_[bucket]) {
    ++bucket;
  }
  buckets_[bucket].fetch_add(1, std::memory_order_relaxed);
}

template <int64_t... Bounds>
int64_t HistRate<Bounds...>::GetInf() const noexcept {
  return buckets_.back().load(std::memory_order_relaxed);
}

template <int64_t... Bounds>
std::array<int64_t, HistRate<Bounds...>::NumBuckets>
HistRate<Bounds...>::GetBuckets() const noexcept {
  std::array<int64_t, NumBuckets> buckets;
  std::transform(
    buckets_.begin(), std::prev(buckets_.end()), buckets.begin(),
    [] (auto& v) { return v.load(std::memory_order_relaxed); });
  return buckets;
}

template <int64_t... Bounds>
const std::array<int64_t, HistRate<Bounds...>::NumBuckets>&
HistRate<Bounds...>::GetBounds() const noexcept { return bounds_; }

}  // namespace detail_


}  // namepsace fmetrics

#endif  // LIB_FAST_METRICS_GAUGE_HPP
