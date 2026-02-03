#pragma once

#include "Reporter.h"
#include <string>
#include <unordered_map>

struct MullDiagnostics;

namespace mull {

class SourceInfoProvider;
class MutationPoint;

class MutationTestingElementsReporter : public Reporter {
public:
  MutationTestingElementsReporter(
      const MullDiagnostics &diagnostics, const std::string &reportDir,
      const std::string &reportName,
      std::unordered_map<std::string, std::string> mullInformation = {});
  void reportResults(const Result &result) override;

  const std::string &getJSONPath();

private:
  void generateHTMLFile();

  const MullDiagnostics &diagnostics;

  std::string filename;
  std::string htmlPath;
  std::string jsonPath;
  std::unordered_map<std::string, std::string> mullInformation;
};

} // namespace mull
