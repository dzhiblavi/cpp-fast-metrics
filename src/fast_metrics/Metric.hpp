#ifndef LIB_FAST_METRICS_METRIC_HPP
#define LIB_FAST_METRICS_METRIC_HPP

#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <memory>

namespace fmetrics {

class Metric {
 public:
  using LabelType = std::pair<std::string, std::string>;
  using LabelsType = std::vector<LabelType>;

  explicit Metric(const std::string& name, const LabelsType& labels);

  explicit Metric(const std::string& name, const std::shared_ptr<LabelsType>& labels) noexcept;

  template <typename... Labels> 
    requires (... && std::same_as<LabelType, std::remove_cv_t<std::remove_reference_t<Labels>>>)
  explicit Metric(const std::string& name, Labels&&... labels)
    : Metric(
        name,
        std::make_shared<LabelsType>(
          std::initializer_list<LabelType>{ std::forward<LabelType>(labels)... })) {}

  template <typename... Labels> 
    requires (... && std::same_as<LabelType, std::remove_cv_t<std::remove_reference_t<Labels>>>)
  void AddLabels(Labels&&... labels) {
    std::array<LabelType, sizeof...(Labels)> passed_labels{ std::forward<Labels>(labels)... };
    for (auto& label : passed_labels) {
      labels_->push_back(std::move(label));
    }
  }

  virtual ~Metric() noexcept = default;

  virtual void Dump(std::ostream& os, const LabelsType& labels) const = 0;

  void Dump(std::ostream& os) const;

 protected:
  template <typename T, typename... Labels>
  void DumpValue(
      std::ostream& os, const std::string& name, const T& value,
      const LabelsType& labels, Labels&&... add_labels) const {
    os << name;
    DumpLabels(os, labels, std::forward<Labels>(add_labels)...);
    os << ' ' << std::to_string(value) << '\n';
  }

  template <typename... Labels>
  void DumpLabels(std::ostream& os, const LabelsType& labels, Labels&&... add_labels) const {
    std::array<LabelType, sizeof...(Labels)> passed_labels{ std::forward<Labels>(add_labels)... };

    static auto dumper = [] (auto& os, const auto& labels, bool last) {
      for (size_t i = 0; i < labels.size(); ++i) {
        const auto& [name, value] = labels[i];
        os << name << "=\"" << value << "\"";
        if (!last || i + 1 < labels.size()) {
          os << ',';
        }
      }
    };

    os << '{';
    dumper(os, *labels_, labels.empty() && passed_labels.empty());
    dumper(os, labels, passed_labels.empty());
    dumper(os, passed_labels, true);
    os << '}';
  }

  std::string name_;
  std::shared_ptr<LabelsType> labels_;
};

template <typename X, typename Y>
static std::pair<std::string, std::string> Label(X&& x, Y&& y) {
  return std::make_pair(std::forward<X>(x), std::forward<Y>(y));
}

}  // namespace fmetrics

#endif  // LIB_FAST_METRICS_METRIC_HPP
