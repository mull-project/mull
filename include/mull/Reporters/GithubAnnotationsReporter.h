#include "Reporter.h"

#include "mull/SourceManager/SourceCodeReader.h"
#include <memory>
#include <regex>
#include <string>
#include <vector>

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
