#pragma once

#include "Reporter.h"

#include <string>

namespace mull {

class SourceInfoProvider;
class MutationPoint;

class MutationTestingElementsReporter : public Reporter {
public:
  MutationTestingElementsReporter(std::string outputPath,
                                  SourceInfoProvider &sourceInfoProvider);
  void reportResults(const Result &result, const RawConfig &config,
                     const Metrics &metrics) override;

private:
  std::string outputPath;
  SourceInfoProvider &sourceInfoProvider;
};

} // namespace mull
