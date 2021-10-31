#include "Reporter.h"

#include "mull/Reporters/SourceCodeReader.h"
#include <memory>
#include <string>
#include <vector>

namespace mull {

class Result;
class Diagnostics;

class PatchesReporter : public Reporter {
public:
  explicit PatchesReporter(Diagnostics &diagnostics, const std::string &reportDir = "",
                          const std::string &reportName = "");

  void reportResults(const Result &result) override;

  std::string getPatchesPath();

private:
  Diagnostics &diagnostics;
  std::string patchesPath;
  SourceCodeReader sourceCodeReader;
};

} // namespace mull
