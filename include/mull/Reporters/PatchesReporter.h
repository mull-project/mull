#include "Reporter.h"

#include "mull/Reporters/SourceCodeReader.h"
#include <memory>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

namespace mull {

class Result;
class Diagnostics;

class PatchesReporter : public Reporter {
public:
  PatchesReporter(Diagnostics &diagnostics, const std::string &reportDir = "",
                  const std::string &reportName = "", const std::string &basePath = "",
                  std::unordered_map<std::string, std::string> mullInformation = {});

  void reportResults(const Result &result) override;

  std::string getPatchesPath();

private:
  Diagnostics &diagnostics;
  std::string patchesPath;
  std::regex basePathRegex;
  SourceCodeReader sourceCodeReader;
  std::unordered_map<std::string, std::string> mullInformation;
};

} // namespace mull
