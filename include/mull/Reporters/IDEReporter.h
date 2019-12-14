#pragma once

#include "Reporter.h"

namespace mull {

class IDEReporter : public Reporter {
public:
  explicit IDEReporter(bool showKilled = false);
  void reportResults(const Result &result, const Metrics &metrics) override;

private:
  bool showKilled;

  void reportSurvivedMutants(const Result &result, const Metrics &metrics);
  void reportAllMutants(const Result &result, const Metrics &metrics);
};

} // namespace mull