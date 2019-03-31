#pragma once

#include "Reporter.h"

namespace mull {

class IDEReporter : public Reporter {
public:
  void reportResults(const Result &result, const RawConfig &config,
                     const Metrics &metrics) override;
};

} // namespace mull