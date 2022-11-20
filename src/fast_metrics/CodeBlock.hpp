#ifndef LIB_FAST_METRICS_CODE_BLOCK_HPP
#define LIB_FAST_METRICS_CODE_BLOCK_HPP

#include <chrono>

#include "src/fast_metrics/Metric.hpp"
#include "src/fast_metrics/Counter.hpp"
#include "src/fast_metrics/Gauge.hpp"

namespace fmetrics {

template <
  /* integral type to represent number of events */ std::integral T,
  /* histograms for measuring execution time */ typename HistogramType
>
class CodeBlock : public Metric {
 public:
  template <typename... Args>
  explicit CodeBlock(const std::string& name, Args&&... args)
    : Metric(name, std::forward<Args>(args)...)
    , running_right_now_count_("running_right_now_count", labels_)
    , started_count_("started_count", labels_)
    , success_count_("success_count", labels_)
    , failure_count_("failure_count", labels_)
    , total_time_spent_("total_time_spent", labels_)
    , success_time_hist_("success_time_hist", labels_)
    , failure_time_hist_("failure_time_hist", labels_) {}

  void Dump(std::ostream& os, const LabelsType& labels) const override {
    LabelsType cb_labels = labels;
    cb_labels.push_back(Label("code_block_name", name_));

    running_right_now_count_.Dump(os, cb_labels);
    started_count_.Dump(os, cb_labels);
    success_count_.Dump(os, cb_labels);
    failure_count_.Dump(os, cb_labels);
    total_time_spent_.Dump(os, cb_labels);
    success_time_hist_.Dump(os, cb_labels);
    failure_time_hist_.Dump(os, cb_labels);
  }

  template <typename DurType, typename F, typename... Args>
  std::invoke_result_t<F, Args...> Execute(F&& func, Args&&... args) {
    static auto end = [this] (const auto& start, bool failed) {
      auto end = std::chrono::high_resolution_clock::now();
      auto time_spent = std::chrono::duration_cast<DurType>(end - start).count();

      running_right_now_count_.Add(-1);
      total_time_spent_.Add(time_spent);
      if (!failed) {
        success_count_.Add(1);
        success_time_hist_.Add(time_spent);
      } else {
        failure_count_.Add(1);
        failure_time_hist_.Add(time_spent);
      }
    };

    auto start = std::chrono::high_resolution_clock::now();
    try {
      running_right_now_count_.Add(1);
      started_count_.Add(1);
      std::forward<F>(func)(std::forward<Args>(args)...);
      end(start, /* failed = */ false);
    } catch (...) {
      end(start, /* failed = */ true);
      throw;
    }
  }

 private:
  Counter<T> running_right_now_count_{};
  Counter<T> started_count_{};
  Counter<T> success_count_{};
  Counter<T> failure_count_{};
  Counter<T> total_time_spent_{};
  HistogramType success_time_hist_{};
  HistogramType failure_time_hist_{};
};

}  // namepsace fmetrics

#endif  // LIB_FAST_METRICS_GAUGE_HPP
