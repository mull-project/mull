#include "Reporter.h"

#include "mull/Reporters/SourceCodeReader.h"
#include <memory>
#include <regex>
#include <string>
#include <vector>

struct MullDiagnostics;

namespace mull {

class Result;

class GithubAnnotationsReporter : public Reporter {
public:
  explicit GithubAnnotationsReporter(const MullDiagnostics &diagnostics);

  void reportResults(const Result &result) override;

private:
  const MullDiagnostics &diagnostics;
};

} // namespace mull
