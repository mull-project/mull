#include "mull/Metrics/MetricsMeasure.h"

using namespace mull;

MetricsMeasure::Precision currentTimestamp() {
  using namespace std::chrono;
  using clock = system_clock;
  return duration_cast<MetricsMeasure::Precision>(clock::now().time_since_epoch());
}

MetricsMeasure::MetricsMeasure() : begin(0), end(0) {}

void MetricsMeasure::start() {
  begin = currentTimestamp();
}

void MetricsMeasure::finish() {
  end = currentTimestamp();
}

MetricsMeasure::Duration MetricsMeasure::duration() const {
  return (end - begin).count();
}

const char *MetricsMeasure::precision() {
  return "ms";
}
