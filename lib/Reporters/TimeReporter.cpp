#include "Reporters/TimeReporter.h"
#include "Metrics/Metrics.h"

using namespace mull;

void TimeReporter::reportResults(const Result &result,
                                 const Config &config,
                                 const Metrics &metrics) {
  metrics.dump();
}
