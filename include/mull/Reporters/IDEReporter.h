#pragma once

#include "Reporter.h"

namespace mull {

class IDEReporter : public Reporter {
public:
  void reportResults(const Result &result, const Metrics &metrics) override;
};

} // namespace mull