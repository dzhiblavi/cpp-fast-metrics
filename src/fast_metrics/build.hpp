#ifndef LIB_FAST_METRICS_BUILD_HPP
#define LIB_FAST_METRICS_BUILD_HPP

#include "src/fast_metrics/Metrics.hpp"

#define PARENS ()
#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...) \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))

#define FOR_EACH_HELPER(macro, a1, a2, ...)                \
    macro(a1, a2)                                          \
    __VA_OPT__(FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))

#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define DEFINE_METRIC_MEMBER(Name, Type) Type Name{std::string(#Name)};
#define REGISTER_METRIC_MEMBER(Name, _) this->metrics_.push_back(&Name);

/**
 * Creates class with name ClassName
 * With metrics specified by rest of the arguments
 */
#define BUILD_METRICS(ClassName, ...)             \
class ClassName : public fmetrics::Metrics {      \
 public:                                          \
  FOR_EACH(DEFINE_METRIC_MEMBER, __VA_ARGS__)     \
                                                  \
  inline static ClassName& Instance() {           \
    static ClassName instance;                    \
    return instance;                              \
  }                                               \
                                                  \
 private:                                         \
  ClassName() {                                   \
    FOR_EACH(REGISTER_METRIC_MEMBER, __VA_ARGS__) \
  }                                               \
}


#endif  // LIB_FAST_METRICS_BUILD_HPP
