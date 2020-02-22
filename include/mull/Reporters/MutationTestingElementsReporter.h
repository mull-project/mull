#pragma once

#include "Reporter.h"
#include <string>

namespace mull {

class SourceInfoProvider;
class MutationPoint;
class Diagnostics;

class MutationTestingElementsReporter : public Reporter {
public:
  MutationTestingElementsReporter(Diagnostics &diagnostics, const std::string &reportDir,
                                  const std::string &reportName,
                                  SourceInfoProvider &sourceInfoProvider);
  void reportResults(const Result &result, const Metrics &metrics) override;

  const std::string &getJSONPath();

private:
  void generateHTMLFile();

  Diagnostics &diagnostics;

  std::string filename;
  std::string htmlPath;
  std::string jsonPath;
  SourceInfoProvider &sourceInfoProvider;
};

} // namespace mull
