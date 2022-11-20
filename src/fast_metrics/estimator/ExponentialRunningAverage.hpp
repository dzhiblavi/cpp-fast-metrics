#ifndef LIB_FAST_METRICS_EXPONENTIAL_RUNNING_AVERAGE_HPP
#define LIB_FAST_METRICS_EXPONENTIAL_RUNNING_AVERAGE_HPP

#include <atomic>
#include <concepts>

#include "src/fast_metrics/estimator/Estimator.hpp"

namespace fmetrics::estimator {

template <typename T, int ForgetPercent>
class ExponentialRunningAverage : public Estimator<T, ExponentialRunningAverage<T, ForgetPercent>> {
 public:
  template <std::convertible_to<T> U>
  void Update(U value) noexcept;

  template <typename U = T> requires std::convertible_to<T, U>
  U Get() const noexcept;

 private:
  static constexpr float UPDATE_COEFF = static_cast<float>(ForgetPercent) / 100.f;
  static constexpr float FORGET_COEFF = 1.f - UPDATE_COEFF;

  std::atomic<T> value_{};
};

template <typename T, int ForgetPercent>
template <std::convertible_to<T> U>
void ExponentialRunningAverage<T, ForgetPercent>::Update(U curr_value) noexcept {
  T new_value, last_value = value_.load(std::memory_order_relaxed);
  do {
    new_value = FORGET_COEFF * last_value + UPDATE_COEFF * curr_value;
  } while (!value_.compare_exchange_strong(last_value, new_value));
}

template <typename T, int ForgetPercent>
template <typename U> requires std::convertible_to<T, U>
U ExponentialRunningAverage<T, ForgetPercent>::Get() const noexcept {
  return value_.load(std::memory_order_relaxed);
}

}  // namespace fmetrics::estimator

#endif  // LIB_FAST_METRICS_EXPONENTIAL_RUNNING_AVERAGE_HPP
