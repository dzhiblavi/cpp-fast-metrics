#ifndef LIB_FAST_METRICS_CONST_HPP
#define LIB_FAST_METRICS_CONST_HPP

#include <atomic>
#include <concepts>

#include "src/fast_metrics/estimator/Estimator.hpp"

namespace fmetrics::estimator {

template <typename T>
class Value : public Estimator<T, Value<T>> {
 public:
  template <std::convertible_to<T> U>
  void Set(U value) noexcept { value_.store(static_cast<T>(value), std::memory_order_relaxed); }

  template <std::convertible_to<T> U>
  void Add(U value) noexcept { value_.fetch_add(static_cast<T>(value), std::memory_order_relaxed); }

  template <typename U = T> requires std::convertible_to<T, U>
  U Get() const noexcept { return static_cast<U>(value_.load(std::memory_order_relaxed)); }

 private:
  std::atomic<T> value_{};
};

}  // namespace fmetrics::estimator

#endif  // LIB_FAST_METRICS_CONST_HPP
