#pragma once

#include "Reporter.h"
#include <string>
#include <unordered_map>

namespace mull {

class SourceInfoProvider;
class MutationPoint;
class Diagnostics;

class MutationTestingElementsReporter : public Reporter {
public:
  MutationTestingElementsReporter(
      Diagnostics &diagnostics, const std::string &reportDir, const std::string &reportName,
      std::unordered_map<std::string, std::string> mullInformation = {});
  void reportResults(const Result &result) override;

  const std::string &getJSONPath();

private:
  void generateHTMLFile();

  Diagnostics &diagnostics;

  std::string filename;
  std::string htmlPath;
  std::string jsonPath;
  std::unordered_map<std::string, std::string> mullInformation;
};

} // namespace mull
