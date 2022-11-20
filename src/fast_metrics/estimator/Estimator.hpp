#ifndef LIB_FAST_METRICS_ESTIMATOR_HPP
#define LIB_FAST_METRICS_ESTIMATOR_HPP

#include <concepts>
#include <utility>
#include <tuple>

namespace fmetrics::estimator {

template <typename T, typename SelfType>
class Estimator {
 public:
  template <typename U = T> requires std::convertible_to<T, U>
  U Get() const noexcept(noexcept(Self().template Get<U>())) { return Self().template Get<U>(); }

 private:
  SelfType& Self() noexcept { return *static_cast<SelfType*>(this); }

  const SelfType& Self() const noexcept { return *static_cast<const SelfType*>(this); }
};

}  // namespace fmetrics::estimator

#endif  // LIB_FAST_METRICS_ESTIMATOR_HPP
