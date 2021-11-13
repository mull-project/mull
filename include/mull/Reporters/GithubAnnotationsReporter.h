#include "Reporter.h"

#include "mull/Reporters/SourceCodeReader.h"
#include <memory>
#include <string>
#include <vector>
#include <regex>

namespace mull {

class Result;
class Diagnostics;

class GithubAnnotationsReporter : public Reporter {
public:
  explicit GithubAnnotationsReporter(Diagnostics &diagnostics);

  void reportResults(const Result &result) override;

private:
  Diagnostics &diagnostics;
};

} // namespace mull
