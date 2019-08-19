#pragma once

#include "Reporter.h"

#include <string>

namespace mull {

class SourceInfoProvider;
class MutationPoint;

class MutationTestingElementsReporter : public Reporter {
public:
  MutationTestingElementsReporter(const std::string &reportDir,
                                  const std::string &reportName,
                                  SourceInfoProvider &sourceInfoProvider);
  void reportResults(const Result &result, const RawConfig &config,
                     const Metrics &metrics) override;

  const std::string &getJSONPath();

private:
  std::string filename;
  std::string htmlPath;
  std::string jsonPath;
  SourceInfoProvider &sourceInfoProvider;
};

} // namespace mull
