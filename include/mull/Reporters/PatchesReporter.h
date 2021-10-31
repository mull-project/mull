#include "Reporter.h"

#include "mull/Reporters/SourceCodeReader.h"
#include <memory>
#include <string>
#include <vector>
#include <regex>

namespace mull {

class Result;
class Diagnostics;

class PatchesReporter : public Reporter {
public:
  explicit PatchesReporter(Diagnostics &diagnostics, const std::string &reportDir = "",
                          const std::string &reportName = "", const std::string basePath = "");

  void reportResults(const Result &result) override;

  std::string getPatchesPath();

private:
  Diagnostics &diagnostics;
  std::string patchesPath;
  std::regex basePathRegex;
  SourceCodeReader sourceCodeReader;
};

} // namespace mull
