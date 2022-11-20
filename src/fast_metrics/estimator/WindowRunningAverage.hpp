#ifndef LIB_FAST_METRICS_WINDOW_RUNNING_AVERAGE_HPP
#define LIB_FAST_METRICS_WINDOW_RUNNING_AVERAGE_HPP

#include <array>
#include <mutex>
#include <concepts>

#include "src/fast_metrics/estimator/Estimator.hpp"

namespace fmetrics::estimator {

template <typename T, typename Period, int WindowTicks>
class WindowAverage : public Estimator<T, WindowAverage<T, Period, WindowTicks>> {
 public:
  template <std::convertible_to<T> U>
  void Update(U value) noexcept;

  template <typename U = T> requires std::convertible_to<T, U>
  U Get() const noexcept;

 private:
  int CleanUpOldEntries(uint64_t ticks) const noexcept;

  int Normalize(int pos) const noexcept;

  static uint64_t GetTicks() noexcept;

  mutable T sum_ = 0;
  mutable int min_point_ = 0;
  mutable std::mutex lock_;
  mutable std::array<std::pair<uint64_t, T>, WindowTicks> rps_{};
};

template <typename T, typename Period, int WindowTicks>
template <std::convertible_to<T> U>
void WindowAverage<T, Period, WindowTicks>::Update(U value) noexcept {
  std::lock_guard<std::mutex> lg(lock_);
  int pos = CleanUpOldEntries(GetTicks());
  if (pos != -1) {
    rps_[pos].second += value;
    sum_ += value;
  }
}

template <typename T, typename Period, int WindowTicks>
template <typename U> requires std::convertible_to<T, U>
U WindowAverage<T, Period, WindowTicks>::Get() const noexcept {
  std::lock_guard<std::mutex> lg(lock_);
  CleanUpOldEntries(GetTicks());
  return static_cast<U>(sum_) / WindowTicks;
}

template <typename T, typename Period, int WindowTicks>
int WindowAverage<T, Period, WindowTicks>::CleanUpOldEntries(uint64_t ticks) const noexcept {
  int max_point = Normalize(min_point_ - 1);
  uint64_t max_ticks = rps_[max_point].first;
  uint64_t min_ticks = rps_[min_point_].first;
  if (ticks < min_ticks) {
    return -1;
  }
  if (ticks <= max_ticks) {
    int offset = max_ticks - ticks;
    return Normalize(max_point - offset);
  }
  uint64_t delta = std::min(static_cast<uint64_t>(WindowTicks), ticks - max_ticks);
  for (uint64_t i = 0; i < delta; ++i) {
    int pos = Normalize(min_point_ + i);
    sum_ -= rps_[pos].second;
    rps_[pos] = {ticks - (delta - i - 1), 0};
  }
  min_point_ = Normalize(min_point_ + delta);
  return Normalize(min_point_ - 1);
}

template <typename T, typename Period, int WindowTicks>
int WindowAverage<T, Period, WindowTicks>::Normalize(int pos) const noexcept {
  return (WindowTicks + pos) % WindowTicks;
}

template <typename T, typename Period, int WindowTicks>
uint64_t WindowAverage<T, Period, WindowTicks>::GetTicks() noexcept {
  return std::chrono::duration_cast<std::chrono::duration<uint64_t, Period>>(
      std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

}  // namespace fmetrics::estimator

#endif  // LIB_FAST_METRICS_WINDOW_RUNNING_AVERAGE_HPP
