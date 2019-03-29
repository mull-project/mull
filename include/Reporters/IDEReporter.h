#pragma once

#include "Reporters/Reporter.h"

namespace mull {

class IDEReporter : public Reporter {
public:
  void reportResults(const Result &result, const RawConfig &config,
                     const Metrics &metrics) override;
};

} // namespace mull