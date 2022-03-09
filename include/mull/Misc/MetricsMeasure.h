#pragma once

#include <chrono>

namespace mull {

struct MetricsMeasure {
  using Precision = std::chrono::milliseconds;
  using Duration = Precision::rep;

  Precision begin;
  Precision end;

  MetricsMeasure();
  void start();
  void finish();
  Duration duration() const;
  static const char *precision();
};

} // namespace mull
