#pragma once

#include "mull/Reporters/Reporter.h"
#include "mull/Reporters/SourceCodeReader.h"

namespace mull {

class Diagnostics;

class IDEReporter : public Reporter {
public:
  explicit IDEReporter(Diagnostics &diagnostics, bool showKilled = false);
  void reportResults(const Result &result) override;

private:
  Diagnostics &diagnostics;
  bool showKilled;
  SourceCodeReader sourceCodeReader;
};

} // namespace mull